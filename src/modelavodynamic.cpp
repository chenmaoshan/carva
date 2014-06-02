/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#include <iostream>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <time.h>
#include <assert.h>
#include <limits.h>
#define _USE_MATH_DEFINES
#include <cmath>

#include "src/definitions.h"
#include "src/modelgeneral.h"
#include "src/modelavostatic.h"
#include "src/modelavodynamic.h"
#include "src/xmlmodelfile.h"
#include "src/modelsettings.h"
#include "src/wavelet1D.h"
#include "src/wavelet3D.h"
#include "src/vario.h"
#include "src/simbox.h"
#include "src/fftgrid.h"
#include "src/gridmapping.h"
#include "src/timings.h"
#include "src/io.h"
#include "src/tasklist.h"
#include "src/seismicparametersholder.h"

#include "lib/utils.h"
#include "lib/random.h"
#include "lib/timekit.hpp"
#include "nrlib/iotools/fileio.hpp"
#include "nrlib/iotools/stringtools.hpp"
#include "nrlib/segy/segy.hpp"
#include "nrlib/surface/surfaceio.hpp"
#include "nrlib/surface/surface.hpp"
#include "nrlib/surface/regularsurface.hpp"
#include "nrlib/iotools/logkit.hpp"
#include "nrlib/stormgrid/stormcontgrid.hpp"
#include "nrlib/volume/volume.hpp"

ModelAVODynamic::ModelAVODynamic(ModelSettings          *& model_settings,
                                 ModelAVOStatic          * model_avo_static,
                                 ModelGeneral            * model_general,
                                 CommonData              * common_data,
                                 SeismicParametersHolder & seismic_parameters,
                                 const Simbox            * simbox,
                                 int                       t,
                                 int                       i_interval)
{ //Time lapse constructor
  reflection_matrix_        = NULL;
  failed_                   = false;
  this_timelapse_           = t;
  bool failed_loading_model = false;
  number_of_angles_         = model_settings->getNumberOfAngles(this_timelapse_);
  local_noise_scale_        = common_data->GetLocalNoiseScaleTimeLapse(this_timelapse_);
  use_local_noise_          = common_data->GetUseLocalNoise();

  //timeDepthMapping ? "Skal evt. eksistera for heile resultate og ikkje per sone."
  //timeCutMapping Er dekka av det ntye simboxformatet

  //estimateWavelet_        = modelSettings->getEstimateWavelet(thisTimeLapse_);
  //matchEnergies_          = modelSettings->getMatchEnergies(thisTimeLapse_);
  wavelets_.  resize(number_of_angles_);
  seis_cubes_.resize(number_of_angles_);
  sn_ratio_.  resize(number_of_angles_);

  theta_deg_       = new float[number_of_angles_];
  data_variance_   = new float[number_of_angles_];
  error_variance_  = new float[number_of_angles_];
  model_variance_  = new float[number_of_angles_];
  signal_variance_ = new float[number_of_angles_];
  theo_sn_ratio_   = new float[number_of_angles_];
  err_theta_cov_   = new double*[number_of_angles_];

  LogKit::WriteHeader("Setting up models for vintage " + NRLib::ToString(t));

  const std::vector<SeismicStorage> & seismic_data = common_data->GetSeismicDataTimeLapse(this_timelapse_);

  angle_.resize(number_of_angles_);
  for (int i = 0; i < number_of_angles_; i++)
    angle_[i] = seismic_data[i].GetAngle();

  for (int i=0; i < number_of_angles_; i++) {
    err_theta_cov_[i] = new double[number_of_angles_];
    theta_deg_[i]     = static_cast<float>(angle_[i]*180.0/NRLib::Pi);
  }

  //AngulurCorr between angle i and j
  angular_corr_ = common_data->GetAngularCorrelation(this_timelapse_);

  //Seismic data: resample seismic-data from correct vintage into the simbox for this interval.
  seis_cubes_.resize(number_of_angles_);

  int nx  = simbox->getnx();
  int ny  = simbox->getny();
  int nz  = simbox->getnz();
  int nxp = simbox->GetNXpad();
  int nyp = simbox->GetNYpad();
  int nzp = simbox->GetNZpad();

  SegY          * segy  = NULL;
  StormContGrid * storm = NULL;

  for (int i = 0; i < number_of_angles_; i++) {

    LogKit::LogFormatted(LogKit::Low,"\nResampling seismic data for angle %4.1f ", theta_deg_[i]);

    seis_cubes_[i] = ModelAVOStatic::CreateFFTGrid(nx, ny, nz, nxp, nyp, nzp, model_settings->getFileGrid());
    seis_cubes_[i]->createRealGrid();
    seis_cubes_[i]->setType(FFTGrid::DATA); //PARAMETER

    int seismic_type = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetSeismicType();
    bool is_segy           = false;
    bool is_storm          = false;
    FFTGrid * fft_grid_old = NULL;

    if (seismic_type == 0) { //SEGY
      segy    = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetSegY();
      is_segy = true;
    }
    else if (seismic_type == 3) { //FFTGrid
      fft_grid_old = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetFFTGrid();
    }
    else { //STORM / SGRI
      storm    = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetStorm();
      is_storm = true;
    }

    bool scale = false;
    if (seismic_type == 2) //SGRI
      scale = true;

    int missing_traces_simbox  = 0;
    int missing_traces_padding = 0;
    int dead_traces_simbox     = 0;

    NRLib::Grid<float> * grid_tmp = new NRLib::Grid<float>();

    seis_cubes_[i]->setAccessMode(FFTGrid::RANDOMACCESS);
    common_data->FillInData(grid_tmp,
                            seis_cubes_[i],
                            simbox,
                            storm,
                            segy,
                            fft_grid_old,
                            model_settings->getSmoothLength(),
                            missing_traces_simbox,
                            missing_traces_padding,
                            dead_traces_simbox,
                            FFTGrid::DATA,
                            scale,
                            is_segy,
                            is_storm);

    seis_cubes_[i]->endAccess();

    if(grid_tmp != NULL)
      delete grid_tmp;


    //Report on missing_traces_simbox, missing_traces_padding, dead_traces_simbox here?
    //In CommonData::ReadSeiscmicData it is checked that segy/storm file covers esimation_simbox
    //if (missingTracesSimbox > 0) {}
    if (missing_traces_padding > 0) {
      int nx     = simbox->getnx();
      int ny     = simbox->getny();
      int nxpad  = nxp - nx;
      int nypad  = nyp - ny;
      int nxypad = nxpad*ny + nx*nypad - nxpad*nypad;
        LogKit::LogMessage(LogKit::High, "Number of grid columns in padding that are outside area defined by seismic data : "
                           +NRLib::ToString(missing_traces_padding)+" of "+NRLib::ToString(nxypad)+"\n");
    }
    if (dead_traces_simbox > 0) {
      LogKit::LogMessage(LogKit::High, "Number of grid columns with no seismic data (nearest trace is dead) : "
                         +NRLib::ToString(dead_traces_simbox)+" of "+NRLib::ToString(simbox->getnx()*simbox->getny())+"\n");
    }

  }

  //Logging from processSeismic

  bool segy_volumes_read = false;
  for (int i = 0; i < number_of_angles_ ; i++) {
    int seismic_type = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetSeismicType();
    if (seismic_type == 0)
      segy_volumes_read = true;
  }
  if (segy_volumes_read) {
    LogKit::LogFormatted(LogKit::Low,"\nArea/resolution           x0           y0            lx         ly     azimuth         dx      dy\n");
    LogKit::LogFormatted(LogKit::Low,"-------------------------------------------------------------------------------------------------\n");
    for (int i = 0; i < number_of_angles_; i++) {
      int seismic_type = common_data->GetSeismicDataTimeLapse(this_timelapse_)[i].GetSeismicType();
      if (seismic_type == 0) {
        double geo_angle = (-1)*simbox->getAngle()*(180/M_PI);
        if (geo_angle < 0)
          geo_angle += 360.0;
        LogKit::LogFormatted(LogKit::Low,"Seismic data %d   %11.2f  %11.2f    %10.2f %10.2f    %8.3f    %7.2f %7.2f\n",i,
                             segy->GetGeometry()->GetX0(), segy->GetGeometry()->GetY0(),
                             segy->GetGeometry()->Getlx(), segy->GetGeometry()->Getly(), geo_angle,
                             segy->GetGeometry()->GetDx(), segy->GetGeometry()->GetDy());
      }
    }
  }

  //H Missing from processSeismic:
  //seisCube[i]->writeFile
  //seisCube[i]->writeCravaFile

  //H model_avo_static->addSeismicLogs with intervals? Hentes fram i BlockedLogs -> WriteRMSWell
  //Or should this have been set in CommonData::ReadWellData?
  if (model_general->GetMultiInterval() == false) {
    model_avo_static->AddSeismicLogs(model_general->GetBlockedWells(),
                                     seis_cubes_,
                                     number_of_angles_);
  }

  //Get Vp/Vs
  double vpvs = 0.0;
  double vsvp = 0.0;
  int    n_well_points = 0;
  if (model_settings->getVpVsRatioIntervals().size() > 0) { //model file
    vpvs = static_cast<double>(model_settings->getVpVsRatioIntervals().find(model_settings->getIntervalName(i_interval))->second);
    vsvp = 1 / vpvs;
  }
  else if (model_settings->getVpVsRatioFromWells()) { //wells
    VsVpFromWells(common_data,
                  i_interval,
                  vsvp,
                  n_well_points);
    vpvs = 1 / vsvp;
  }
  else {  //background
    vsvp = common_data->GetBackgroundVsVpRatioInterval(i_interval);
    vpvs = 1 / vsvp;
  }

  std::string interval_text = "";
  if (common_data->GetMultipleIntervalGrid()->GetNIntervals() > 1)
    interval_text = "for interval " + model_settings->getIntervalName(i_interval) + " ";

  //Reflection matrix: From file or global vp/vs: do not change per interval
  if (common_data->GetRefMatFromFileGlobalVpVs() == true) {
    reflection_matrix_ = common_data->GetReflectionMatrixTimeLapse(this_timelapse_);
  }
  else {  //Vp/Vs set temporary to 2 in CommonData: update reflection matrix per interval.

    if (model_settings->getVpVsRatioIntervals().size() > 0) {
      LogKit::LogFormatted(LogKit::Low,"\nMaking reflection matrix " + interval_text + "with Vp/Vs ratio specified in model file.\n");

      common_data->SetupDefaultReflectionMatrix(reflection_matrix_, vsvp, model_settings, number_of_angles_, this_timelapse_);
    }
    else if (model_settings->getVpVsRatioFromWells()) {
      LogKit::LogFormatted(LogKit::Low,"\nMaking reflection matrix " + interval_text + "with Vp and Vs from wells\n");
      if (n_well_points < 10)
        LogKit::LogFormatted(LogKit::Warning," Less than 10 total well-points was inside the interval. Vp-vs ratio will be taken from the background model.");

      common_data->SetupDefaultReflectionMatrix(reflection_matrix_, vsvp, model_settings, number_of_angles_, this_timelapse_);
    }
    else { //Rock-physics or background model (In manual: vp/vs from background model is default)

      //Vp/Vs from rock-physics: Get it from background model generated by rock-physics
      if (model_settings->getGenerateBackgroundFromRockPhysics()) {
        LogKit::LogFormatted(LogKit::Low,"\nMaking reflection matrix " + interval_text + "with Vp and Vs from the Rock Physics model.\n");
      }
      else {
       LogKit::LogFormatted(LogKit::Low,"\nMaking reflection matrix " + interval_text + "with Vp and Vs from the background model.\n");
      }
      common_data->SetupDefaultReflectionMatrix(reflection_matrix_, vsvp, model_settings, number_of_angles_, this_timelapse_);
    }
  }

  //Wavelet: estimated -> reestimate scale and noise (with updated vp/vs for this interval, waveletshape is from estimation in commondata.).
  std::vector<bool> wavelet_estimated = model_settings->getEstimateWavelet(this_timelapse_); //vector over angles
  wavelets_ = common_data->GetWavelet(this_timelapse_);

  //Stored in Commondata for wavelets (Per timelapse, per angle):
  //local_noise_scale  //Updated above (taken directly from commonData)
  //local_shift
  //local_scale
  //global_noise_estimate
  //sn_ratio
  const std::map<std::string, BlockedLogsCommon *> & orig_blocked_logs = common_data->GetBlockedLogs();

  for (int i = 0; i < number_of_angles_; i++) {

    wavelets_[i] = common_data->GetWavelet(this_timelapse_)[i];
    std::vector<SeismicStorage> orig_seis = common_data->GetSeismicDataTimeLapse(this_timelapse_);

    bool adjust_scale = (wavelet_estimated[i] == true || model_settings->getEstimateGlobalWaveletScale(this_timelapse_, i) == true);
    bool adjust_noise = model_settings->getEstimateSNRatio(this_timelapse_, i);
    if (adjust_scale == true || adjust_noise == true) {
      std::string err_text;
      int error = 0;
      Grid2D * noise_scaled_tmp;
      Grid2D * shift_grid_tmp;
      Grid2D * gain_grid_tmp;

      float sn_ratio_new = 0.0f;

      //Update with new Vp/Vs (new reflection matrix set above) before reestimating SNRatio and WaveletScale
      wavelets_[i]->SetReflectionCoeffs(reflection_matrix_[i]);

      LogKit::LogFormatted(LogKit::Low,"\nReestimating wavelet scale and noise " + interval_text + " and angle number " + NRLib::ToString(i) + ".\n");

      if (model_settings->getWaveletDim(i) == Wavelet::ONE_D) { //Reestimate scale and noise with vp/vs for this interval.
        //This is where it gets hairy. All estimation is done on estmation simbox scale, so create a local wavelet and resample.
        //Find the scaling of this wavelet, and apply it to the non-resampled wavelet.
        Wavelet * est_wavelet = new Wavelet1D(wavelets_[i]);
        const Simbox & estimation_simbox = common_data->GetEstimationSimbox();
        if(est_wavelet->getInFFTOrder() == false) //Indicates wavelet read from file; true would mean estimated wavelet, which has correct resolution.
          est_wavelet->resample(static_cast<float>(estimation_simbox.getdz()), estimation_simbox.getnz(), estimation_simbox.GetNZpad());
        est_wavelet->scale(1.0);
        std::vector<std::vector<double> > seis_logs(orig_blocked_logs.size());
        int w = 0;
        std::map<std::string, BlockedLogsCommon *> mapped_blocked_logs;
        Simbox estimation_box = common_data->GetEstimationSimbox();
        for(std::map<std::string, BlockedLogsCommon *>::const_iterator it = orig_blocked_logs.begin(); it != orig_blocked_logs.end(); it++) {
          std::map<std::string, BlockedLogsCommon *>::const_iterator iter = orig_blocked_logs.find(it->first);
          BlockedLogsCommon * blocked_log = new BlockedLogsCommon(*(iter->second));
          blocked_log->VolumeFocus(*simbox);
          seis_logs[w].resize(blocked_log->GetNumberOfBlocks());
          blocked_log->GetBlockedGrid(&(orig_seis[i]), &estimation_box, seis_logs[w]);
          mapped_blocked_logs[iter->first] = blocked_log;

          w++;
        }

        sn_ratio_new = est_wavelet->calculateSNRatioAndLocalWavelet(simbox,
                                                                    &(estimation_simbox),
                                                                    seis_logs,
                                                                    mapped_blocked_logs,
                                                                    model_settings,
                                                                    err_text,
                                                                    error,
                                                                    i, //angle
                                                                    noise_scaled_tmp,
                                                                    shift_grid_tmp,
                                                                    gain_grid_tmp,
                                                                    common_data->GetSNRatioTimeLapse(this_timelapse_)[i],
                                                                    1.0,    //scale, set to 1 due to resampling.
                                                                    adjust_noise,   // doEstimateSNRatio
                                                                    adjust_scale,   // doEstimateGlobalScale
                                                                    false,  // doEstimateLocalNoise
                                                                    false,  // doEstimateLocalShift
                                                                    false,  // doEstimateLocalScale
                                                                    false); // doEstimateWavelet

        //Compute scaling and scale original wavelet
        float gain = est_wavelet->getScale();
        wavelets_[i]->scale(gain);
      }
    }
    else {
      wavelets_[i] = common_data->GetWavelet(this_timelapse_)[i];
      sn_ratio_[i] = common_data->GetSNRatioTimeLapse(this_timelapse_)[i];
    }
  }

  if (model_settings->getEstimateWaveletNoise())
    CommonData::GenerateSyntheticSeismicLogs(wavelets_, model_general->GetBlockedWells(), reflection_matrix_, simbox);

  //Compute variances (Copied from avoinversion.cpp in order to avoid putting matchenergies there)
  fftw_real * corrT = seismic_parameters.extractParamCorrFromCovVp(nzp);

  ComputeDataVariance(seis_cubes_,
                      data_variance_,
                      nx,  ny,  nz,
                      nxp, nyp, nzp);

  SetupErrorCorrelation(local_noise_scale_,
                        data_variance_,
                        error_variance_,
                        err_theta_cov_);

  Wavelet1D ** error_smooth = new Wavelet1D*[number_of_angles_];
  float      * param_var    = new float[number_of_angles_] ;
  float      * wd_corr_mvar = new float[number_of_angles_];

  for (int i=0; i < number_of_angles_; i++) {
    Wavelet1D * wavelet1D = wavelets_[i]->createWavelet1DForErrorNorm();
    error_smooth[i]       = new Wavelet1D(wavelet1D, Wavelet::FIRSTORDERFORWARDDIFF);
    delete wavelet1D;

    std::string angle     = NRLib::ToString(theta_deg_[i], 1);
    std::string file_name = IO::PrefixWavelet() + std::string("Diff_") + angle + IO::SuffixGeneralData();
    error_smooth[i]->printToFile(file_name);
  }

  // Compute variation in parameters
  NRLib::Matrix prior_var_0 = seismic_parameters.getPriorVar0();

  for (int i = 0; i < number_of_angles_; i++) {
    param_var[i]=0.0;
    for (int l=0; l<3 ; l++) {
      for (int m=0 ; m<3 ; m++) {
        param_var[i] += static_cast<float>(prior_var_0(l,m))*reflection_matrix_[i][l]*reflection_matrix_[i][m];
      }
    }
  }

  // Compute variation in wavelet
  for (int l = 0; l < number_of_angles_; l++) {
    wd_corr_mvar[l] = ComputeWDCorrMVar(error_smooth[l], corrT, nzp);
  }

  // Compute signal and model variance and theoretical signal-to-noise-ratio
  for (int l=0; l < number_of_angles_; l++) {
    model_variance_[l]  = wd_corr_mvar[l]*param_var[l];
    signal_variance_[l] = error_variance_[l] + model_variance_[l];
  }

  std::vector<bool> match_energies = model_settings->getMatchEnergies(this_timelapse_);

  for (int l = 0; l < number_of_angles_; l++) {
    if (match_energies[l]) {
      LogKit::LogFormatted(LogKit::Low,"Matching syntethic and empirical energies:\n");
      float gain = sqrt((error_variance_[l]/model_variance_[l])*(sn_ratio_[l] - 1.0f));
      wavelets_[l]->scale(gain);
      if ((model_settings->getWaveletOutputFlag() & IO::GLOBAL_WAVELETS) > 0 ||
        (model_settings->getEstimationMode() && model_settings->getEstimateWavelet(this_timelapse_)[l]))
      {
        std::string angle     = NRLib::ToString(theta_deg_[l], 1);
        std::string file_name = IO::PrefixWavelet() + std::string("EnergyMatched_") + angle;
        wavelets_[l]->writeWaveletToFile(file_name, 1.0,false); // dt_max = 1.0;
      }
      model_variance_[l] *= gain*gain;
      signal_variance_[l] = error_variance_[l] + model_variance_[l];
    }
    theo_sn_ratio_[l] = signal_variance_[l]/error_variance_[l];
  }

  if (segy != NULL)
    delete segy;
  if (storm != NULL)
    delete storm;

  delete [] param_var;
  delete [] wd_corr_mvar;
  for (int i = 0; i < number_of_angles_; i++)
    delete error_smooth[i];
  delete [] error_smooth;

  failed_ = failed_loading_model;
}

ModelAVODynamic::~ModelAVODynamic(void)
{
  for (int i=0; i <number_of_angles_;i++) {
    if (wavelets_[i] != NULL)
      delete wavelets_[i];
  }

  if (reflection_matrix_ != NULL) {
    for (int i = 0; i < number_of_angles_; i++)
      delete [] reflection_matrix_[i] ;
    delete [] reflection_matrix_ ;
  }

  for (int i = 0 ; i < static_cast<int>(local_noise_scale_.size()) ; i++) {
    if (local_noise_scale_[i] != NULL)
      delete local_noise_scale_[i];
  }

  for (int i=0;i<number_of_angles_;i++)
    if (seis_cubes_[i] != NULL)
      delete seis_cubes_[i];
}

void
ModelAVODynamic::ReleaseGrids(void)
{
  //seisCube_ = NULL;
}

void ModelAVODynamic::VsVpFromWells(CommonData * common_data,
                                    int          i_interval,
                                    double     & vs_vp,
                                    int        & N)
{
  const std::vector<NRLib::Well> & wells = common_data->GetWells();
  size_t n_wells = wells.size();

  for (size_t i = 0; i < n_wells; i++) {
    const NRLib::Surface<double> & top  = common_data->GetMultipleIntervalGrid()->GetIntervalSimbox(i_interval)->GetTopSurface();
    const NRLib::Surface<double> & base = common_data->GetMultipleIntervalGrid()->GetIntervalSimbox(i_interval)->GetBotSurface();

    double mean_vs_vp = 0.0; // Average Vs/Vp for this well
    int    n_vs_vp    = 0; // Number of samples behind Vs/Vp estimate

    FindMeanVsVp(wells[i], top, base, mean_vs_vp, n_vs_vp);

    N     += n_vs_vp;
    vs_vp += mean_vs_vp*n_vs_vp;
  }

  vs_vp /= N;
}

void ModelAVODynamic::FindMeanVsVp(const NRLib::Well            & well,
                                   const NRLib::Surface<double> & top,
                                   const NRLib::Surface<double> & bot,
                                   double                       & mean_vs_vp,
                                   int                          & n_vs_vp)
{
  size_t n_data = well.GetNData();
  std::vector<bool> active_cell(n_data, true);

  const std::vector<double> & x_pos = well.GetContLog("X_pos");
  const std::vector<double> & y_pos = well.GetContLog("Y_pos");
  const std::vector<double> & z_pos = well.GetContLog("Z_pos");

  for (size_t i = 0; i < n_data; i++) {
    double z_top  = top.GetZ(x_pos[i], y_pos[i]);
    double z_base = bot.GetZ(x_pos[i], y_pos[i]);

    if ((z_pos[i] < z_top) || (z_pos[i] > z_base))
      active_cell[i] = false;
  }

  mean_vs_vp = 0.0;
  n_vs_vp    = 0;

  const std::vector<double> & vp_background_resolution = well.GetContLogBackgroundResolution("Vp");
  const std::vector<double> & vs_background_resolution = well.GetContLogBackgroundResolution("Vs");

  for (size_t i = 0; i < n_data; i++) {
    if (vp_background_resolution[i] != RMISSING && vs_background_resolution[i] != RMISSING && active_cell[i] == true) {
      mean_vs_vp += vs_background_resolution[i] / vp_background_resolution[i];
      n_vs_vp    += 1;
    }
  }

  mean_vs_vp /= n_vs_vp;
}

bool
ModelAVODynamic::FindTimeGradientSurface(const std::string    & refTimeFile,
                                         const Simbox         * simbox,
                                         NRLib::Grid2D<float> & refTimeGradX,
                                         NRLib::Grid2D<float> & refTimeGradY)
{
  double x, y;
  bool inside = true;
  unsigned int nx = static_cast<unsigned int> (simbox->getnx());
  unsigned int ny = static_cast<unsigned int> (simbox->getny());
  float dx = static_cast<float> (simbox->getdx());
  float dy = static_cast<float> (simbox->getdy());

  if (!NRLib::IsNumber(refTimeFile)) {
    NRLib::RegularSurfaceRotated<float> t0surface(refTimeFile);

    simbox->getXYCoord(0,0,x,y);
    if (t0surface.IsInsideSurface(x,y)) {
      simbox->getXYCoord(0,ny-1,x,y);
      if (t0surface.IsInsideSurface(x,y)) {
        simbox->getXYCoord(nx-1,0,x,y);
        if (t0surface.IsInsideSurface(x,y)) {
          simbox->getXYCoord(nx-1,ny-1,x,y);
          if (!t0surface.IsInsideSurface(x,y))
            inside = false;
        }
        else
          inside = false;
      }
      else
        inside = false;
    }
    else
      inside = false;

    if (inside) {
      refTimeGradX.Resize(nx, ny, RMISSING);
      refTimeGradY.Resize(nx, ny, RMISSING);
      for (int i = nx-1; i >= 0; i--) {
        for (int j = ny-1; j >= 0; j--) {
          simbox->getXYCoord(i,j,x,y);
          float z_high = t0surface.GetZInside(x,y);
          simbox->getXYCoord(i,j-1,x,y); //XYCoord is ok even if j = -1, but point is outside simbox
          float z_low = t0surface.GetZInside(x,y);
          if (!t0surface.IsMissing(z_low))
            refTimeGradY(i,j) = (z_high - z_low) / dy;
          else
            refTimeGradY(i,j) = refTimeGradY(i,j+1);

          simbox->getXYCoord(i-1,j,x,y); //XYCoord is ok even if j = -1, but point is outside simbox
          z_low = t0surface.GetZInside(x,y);
          if (!t0surface.IsMissing(z_low))
            refTimeGradX(i,j) = (z_high - z_low) / dx;
          else
            refTimeGradX(i,j) = refTimeGradX(i+1,j);
        }
      }
    }
  }
  else {
    refTimeGradX.Resize(nx, ny, 0.0);
    refTimeGradY.Resize(nx, ny, 0.0);
  }

  return(inside);
}

void
ModelAVODynamic::ComputeDataVariance(std::vector<FFTGrid *> & seis_data,
                                     float                  * data_variance,
                                     int                      nx,
                                     int                      ny,
                                     int                      nz,
                                     int                      nxp,
                                     int                      nyp,
                                     int                      nzp)
{
  //copied from avoinversion.cpp
  //
  // Compute variation in raw seismic
  //

  int rnxp = 2*(nxp/2+1);
  for (int l=0; l < number_of_angles_; l++) {
    double  totvar = 0;
    long int ndata = 0;
    data_variance[l]=0.0;
    seis_data[l]->setAccessMode(FFTGrid::READ);
    for (int k=0; k < nzp; k++) {
      double tmpvar1 = 0;
      for (int j=0; j < nyp; j++) {
        double tmpvar2 = 0;
        for (int i=0; i < rnxp; i++) {
          float tmp=seis_data[l]->getNextReal();
          if (k < nz && j < ny &&  i < nx && tmp != 0.0) {
            tmpvar2 += double(tmp*tmp);
            ndata++;
          }
        }
        tmpvar1 += tmpvar2;
      }
      totvar += tmpvar1;
    }
    seis_data[l]->endAccess();
    if (ndata == 0) {
      data_variance[l] = 0.0;
      LogKit::LogFormatted(LogKit::Low,"\nWARNING: All seismic data in stack "+NRLib::ToString(l)+" have zero amplitude.\n");
      TaskList::addTask("Check the seismic data for stack"+NRLib::ToString(l)+". All data have zero amplitude.");
    }
    else {
      data_variance[l] = static_cast<float>(totvar/static_cast<double>(ndata));
    }
  }
}

void
ModelAVODynamic::SetupErrorCorrelation(const std::vector<Grid2D *> & noise_scale,
                                       const float                 * data_variance,
                                       float                       * error_variance,
                                       double                     ** err_theta_cov)
{
  //
  //  Setup error correlation matrix
  //

  for (int l = 0; l < number_of_angles_; l++) {
    if (use_local_noise_ == true) {
      double minScale = noise_scale[l]->FindMin(RMISSING);
      error_variance[l] = float(data_variance[l]*minScale/sn_ratio_[l]);
    }
    else
      error_variance[l] = data_variance[l]/sn_ratio_[l];
  }

  for (int i = 0; i < number_of_angles_; i++) {
    for (int j = 0; j < number_of_angles_; j++) {

      err_theta_cov[i][j] = static_cast<float>(sqrt(error_variance_[i])
                                              *sqrt(error_variance_[j])
                                              *angular_corr_[i][j]);
    }
  }
}

float
ModelAVODynamic::ComputeWDCorrMVar(Wavelet1D* WD,
                                   fftw_real* corrT,
                                   int        nzp)
{
  float var = 0.0;
  int i, j, corrInd;

  for (i=0; i < nzp; i++) {
    for (j=0; j < nzp; j++) {
      corrInd = std::max(i-j,j-i);
      var += WD->getRAmp(i)*corrT[corrInd]*WD->getRAmp(j);
    }
  }

  return var;
}

