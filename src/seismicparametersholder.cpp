#include <cstdio>
#include <complex>

#include "nrlib/flens/nrlib_flens.hpp"

#include "src/seismicparametersholder.h"
#include "src/fftfilegrid.h"
#include "src/fftgrid.h"
#include "src/modelgeneral.h"
#include "src/tasklist.h"
#include "src/modelsettings.h"
#include "lib/lib_matr.h"
#include "nrlib/random/normal.hpp"



SeismicParametersHolder::SeismicParametersHolder(void)
{
  meanVp_            = NULL;
  meanVs_            = NULL;
  meanRho_           = NULL;
  covVp_             = NULL;
  covVs_             = NULL;
  covRho_            = NULL;
  crCovVpVs_         = NULL;
  crCovVpRho_        = NULL;
  crCovVsRho_        = NULL;
  postVp_            = NULL;
  postVs_            = NULL;
  postRho_           = NULL;
  postVpKriged_      = NULL;
  postVsKriged_      = NULL;
  postRhoKriged_     = NULL;
  block_grid_        = NULL;
  facies_prob_undef_ = NULL;
  quality_grid_      = NULL;
  corr_T_            = NULL;
  corr_T_filtered_   = NULL;
}

//--------------------------------------------------------------------
SeismicParametersHolder::~SeismicParametersHolder(void)
{
  //if (covVp_ != NULL)//H Failes if they are deleted under CreateStormGrid
  //  delete covVp_;

  //if (covVs_ != NULL)
  //  delete covVs_;

  //if (covRho_ != NULL)
  //  delete covRho_;

  //if (crCovVpVs_ != NULL)
  //  delete crCovVpVs_ ;

  //if (crCovVpRho_ != NULL)
  //  delete crCovVpRho_ ;

  //if (crCovVsRho_ != NULL)
  //  delete crCovVsRho_;

  //if (meanVp_ != NULL)
  //  delete meanVp_;

  //if (meanVs_ != NULL)
  //  delete meanVs_;

  //if (meanRho_ != NULL)
  //  delete meanRho_;

  //if (postVp_ != NULL)
  //  delete postVp_;

  //if (postVs_ != NULL)
  //  delete postVs_;

  //if (postRho_ != NULL)
  //  delete postRho_;

  //if (postVpKriged_ != NULL)
  //  delete postVpKriged_;

  //if (postVsKriged_ != NULL)
  //  delete postVsKriged_;

  //if (postRhoKriged_ != NULL)
  //  delete postRhoKriged_;

  //if (block_grid_ != NULL)
  //  delete block_grid_;

  //if (facies_prob_undef_ != NULL)
  //  delete facies_prob_undef_;

  //if (quality_grid_ != NULL)
  //  delete quality_grid_;

  //for (size_t i = 0; i < simulations_seed0_.size(); i++) {
  //  if (simulations_seed0_[i] != NULL)
  //    delete simulations_seed0_[i];
  //}

  //for (size_t i = 0; i < simulations_seed1_.size(); i++) {
  //  if (simulations_seed1_[i] != NULL)
  //    delete simulations_seed1_[i];
  //}

  //for (size_t i = 0; i < simulations_seed2_.size(); i++) {
  //  if (simulations_seed2_[i] != NULL)
  //    delete simulations_seed2_[i];
  //}

  //for (size_t i = 0; i < facies_prob_.size(); i++) {
  //  if (facies_prob_[i] != NULL)
  //    delete facies_prob_[i];
  //}

  //for (size_t i = 0; i < facies_prob_geo_.size(); i++) {
  //  if (facies_prob_geo_[i] != NULL)
  //    delete facies_prob_geo_[i];
  //}

  //for (size_t i = 0; i < lh_cube_.size(); i++) {
  //  if (lh_cube_[i] != NULL)
  //    delete lh_cube_[i];
  //}


}
//--------------------------------------------------------------------

void
SeismicParametersHolder::setBackgroundParameters(FFTGrid * meanVp,
                                                 FFTGrid * meanVs,
                                                 FFTGrid * meanRho)
{
  meanVp_   = meanVp;
  meanVs_   = meanVs;
  meanRho_  = meanRho;
}

void
SeismicParametersHolder::setBackgroundParametersInterval(const std::vector<NRLib::Grid<float> *> & mean_parameters,
                                                         int                                       nx_pad,
                                                         int                                       ny_pad,
                                                         int                                       nz_pad)
{
  meanVp_  = new FFTGrid(mean_parameters[0], nx_pad, ny_pad, nz_pad);
  meanVs_  = new FFTGrid(mean_parameters[1], nx_pad, ny_pad, nz_pad);
  meanRho_ = new FFTGrid(mean_parameters[2], nx_pad, ny_pad, nz_pad);
}

//--------------------------------------------------------------------

void
SeismicParametersHolder::setCorrelationParameters(bool                                  cov_estimated,
                                                  const NRLib::Matrix                 & priorVar0,
                                                  const std::vector<NRLib::Matrix>    & auto_cov,
                                                  const std::vector<double>           & priorCorrT,
                                                  const Surface                       * priorCorrXY,
                                                  const int                           & minIntFq,
                                                  const float                         & corrGradI,
                                                  const float                         & corrGradJ,
                                                  const int                           & nx,
                                                  const int                           & ny,
                                                  const int                           & nz,
                                                  const int                           & nxPad,
                                                  const int                           & nyPad,
                                                  const int                           & nzPad,
                                                  double                                dz)
{
  priorVar0_      = priorVar0;
  cov_estimated_  = cov_estimated;

  createCorrGrids(nx, ny, nz, nxPad, nyPad, nzPad, false);

  InitializeCorrelations(cov_estimated,
                         priorCorrXY,
                         auto_cov,
                         priorCorrT,
                         corrGradI,
                         corrGradJ,
                         minIntFq,
                         nzPad,
                         dz);
}
//--------------------------------------------------------------------

void
SeismicParametersHolder::createCorrGrids(int  nx,
                                         int  ny,
                                         int  nz,
                                         int  nxp,
                                         int  nyp,
                                         int  nzp,
                                         bool fileGrid)
{
  covVp_      = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);
  covVs_      = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);
  covRho_     = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);
  crCovVpVs_  = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);
  crCovVpRho_ = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);
  crCovVsRho_ = createFFTGrid(nx,ny,nz,nxp,nyp,nzp,fileGrid);

  covVp_     ->setType(FFTGrid::COVARIANCE);
  covVs_     ->setType(FFTGrid::COVARIANCE);
  covRho_    ->setType(FFTGrid::COVARIANCE);
  crCovVpVs_ ->setType(FFTGrid::COVARIANCE);
  crCovVpRho_->setType(FFTGrid::COVARIANCE);
  crCovVsRho_->setType(FFTGrid::COVARIANCE);

  covVp_     ->createRealGrid();
  covVs_     ->createRealGrid();
  covRho_    ->createRealGrid();
  crCovVpVs_ ->createRealGrid();
  crCovVpRho_->createRealGrid();
  crCovVsRho_->createRealGrid();
}
//-------------------------------------------------------------------
FFTGrid *
SeismicParametersHolder::createFFTGrid(int nx,  int ny,  int nz,
                                       int nxp, int nyp, int nzp,
                                       bool fileGrid)
{
  FFTGrid * fftGrid;
  if(fileGrid)
    fftGrid = new FFTFileGrid(nx, ny, nz, nxp, nyp, nzp);
  else
    fftGrid = new FFTGrid(nx, ny, nz, nxp, nyp, nzp);
  return(fftGrid);
}

//-------------------------------------------------------------------
void
SeismicParametersHolder::InitializeCorrelations(bool                                  cov_estimated, //or from file
                                                const Surface                       * prior_corr_xy,
                                                const std::vector<NRLib::Matrix>    & auto_cov,
                                                const std::vector<double>           & prior_corr_t,
                                                const float                         & corr_grad_I,
                                                const float                         & corr_grad_J,
                                                const int                           & low_int_cut,
                                                const int                           & nzp,
                                                double                                dz)
{
  //
  // Erik N: If correlations are estimated, parameter covariance and temporal correlation
  // are now integrated in auto_cov
  //
  if (cov_estimated) {
    double eps = std::pow(10.0,-10.0);
    std::vector<std::vector<fftw_real *> > circ_auto_cov;
    //NRLib::Vector pos(auto_cov.size());
    //NRLib::Vector neg(auto_cov.size());
    circ_auto_cov.resize(3);
    for(int i = 0; i < 3; i++){
      circ_auto_cov[i].resize(3);
      for(int j = 0; j < 3; j++){
        std::vector<double> corr_t_positive(auto_cov.size());
        std::vector<double> corr_t_negative(auto_cov.size());
        assert(auto_cov[0](i,j) == auto_cov[0](j,i)); // This condition must be true
        //LogKit::LogFormatted(LogKit::Low,"\nAuto Cov "+ CommonData::ConvertIntToString(i) + CommonData::ConvertIntToString(j) + ": " + CommonData::ConvertFloatToString( auto_cov[0](i,j)) +"\n");
        for (size_t k = 0; k < auto_cov.size(); k++){
          if(std::abs(auto_cov[0](i,j)) > eps){ // If the autocovariance in lag 0 is 0 to floating point precision the algorithm fails
            corr_t_positive[k] = auto_cov[k](i,j);//auto_cov[0](i,j); // ComputeCircAutoCov scales the values with the first element
            corr_t_negative[k] = auto_cov[k](j,i);//auto_cov[0](j,i);
            //pos(k) = corr_t_positive[k];
            //neg(k) = corr_t_negative[k];
          }
          else{
            corr_t_positive[k] = 0;
            corr_t_negative[k] = 0;
          }
        }
        //std::string s1 = "vec_pos_" + CommonData::ConvertIntToString(i) + CommonData::ConvertIntToString(j) + ".dat";
        //std::string s2 = "vec_neg_" + CommonData::ConvertIntToString(i) + CommonData::ConvertIntToString(j) + ".dat";
        //NRLib::WriteVectorToFile(s1, pos);
        //NRLib::WriteVectorToFile(s2, neg);
        circ_auto_cov [i][j]= ComputeCircAutoCov(corr_t_positive, corr_t_negative, nzp);
      }
    }


    //
    // Ensure that the covariance structure is positive definite by
    // removing negative eigenvalues in the Fourier domain
    //
    LogKit::LogFormatted(LogKit::Low,"\nMaking estimated autocovariance positive definite..\n");
    MakeCircAutoCovPosDef(circ_auto_cov, nzp);


    //
    // Taper the circular autocov vector
    //
    LogKit::LogFormatted(LogKit::Low,"\nTapering autocovariance estimates..\n");
    TaperCircAutoCovFunction(circ_auto_cov, nzp, dz);

    /*
        NRLib::Matrix cov(3,3);
    for(int k = 0; k < nzp; k++){
      for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
          cov(i,j) = circ_auto_cov[i][j][k];
        }
      }
      std::string s = "cov_" + CommonData::ConvertIntToString(k);
      NRLib::WriteMatrix(s, cov);
    }
    exit(1);
    */

    covVp_      ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[0][0], corr_grad_I, corr_grad_J);
    covVs_      ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[1][1], corr_grad_I, corr_grad_J);
    covRho_     ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[2][2], corr_grad_I, corr_grad_J);
    crCovVpVs_  ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[0][1], corr_grad_I, corr_grad_J);
    crCovVpRho_ ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[0][2], corr_grad_I, corr_grad_J);
    crCovVsRho_ ->FillInLateralCorr(prior_corr_xy, circ_auto_cov[1][2], corr_grad_I, corr_grad_J);

    /*
    covVp_      ->multiplyByScalar(static_cast<float>(auto_cov[0](0,0)));
    covVs_      ->multiplyByScalar(static_cast<float>(auto_cov[0](1,1)));
    covRho_     ->multiplyByScalar(static_cast<float>(auto_cov[0](2,2)));
    crCovVpVs_  ->multiplyByScalar(static_cast<float>(auto_cov[0](0,1)));
    crCovVpRho_ ->multiplyByScalar(static_cast<float>(auto_cov[0](0,2)));
    crCovVsRho_ ->multiplyByScalar(static_cast<float>(auto_cov[0](1,2)));
    */

    for(int i = 0; i < 3; i++){
      for (int j = 0; j < 3; j++){
        fftw_free(circ_auto_cov[i][j]);
      }
    }
  }
  else{
    fftw_real * circ_corr_t = computeCircCorrT(prior_corr_t, low_int_cut, nzp);

    covVp_     ->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);
    covVs_     ->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);
    covRho_    ->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);
    crCovVpVs_ ->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);
    crCovVpRho_->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);
    crCovVsRho_->fillInParamCorr(prior_corr_xy, circ_corr_t, corr_grad_I, corr_grad_J);

    covVp_     ->multiplyByScalar(static_cast<float>(priorVar0_(0,0)));
    covVs_     ->multiplyByScalar(static_cast<float>(priorVar0_(1,1)));
    covRho_    ->multiplyByScalar(static_cast<float>(priorVar0_(2,2)));
    crCovVpVs_ ->multiplyByScalar(static_cast<float>(priorVar0_(0,1)));
    crCovVpRho_->multiplyByScalar(static_cast<float>(priorVar0_(0,2)));
    crCovVsRho_->multiplyByScalar(static_cast<float>(priorVar0_(1,2)));

    fftw_free(circ_corr_t);
  }
}

//--------------------------------------------------------------------
NRLib::Matrix
SeismicParametersHolder::getPriorVar0(void) const
{
  return priorVar0_;
}

//--------------------------------------------------------------------
void
SeismicParametersHolder::allocateGrids(const int nx, const int ny, const int nz, const int nxPad, const int nyPad, const int nzPad)
{
  createCorrGrids(nx, ny, nz, nxPad, nyPad, nzPad, false);

  meanVp_  = ModelGeneral::CreateFFTGrid(nx, ny, nz, nxPad, nyPad, nzPad, false);
  meanVs_  = ModelGeneral::CreateFFTGrid(nx, ny, nz, nxPad, nyPad, nzPad, false);
  meanRho_ = ModelGeneral::CreateFFTGrid(nx, ny, nz, nxPad, nyPad, nzPad, false);

  meanVp_ ->setType(FFTGrid::PARAMETER);
  meanVs_ ->setType(FFTGrid::PARAMETER);
  meanRho_->setType(FFTGrid::PARAMETER);

  meanVp_ ->createGrid();
  meanVs_ ->createGrid();
  meanRho_->createGrid();

  meanVp_ ->fillInConstant(0.0);
  meanVs_ ->fillInConstant(0.0);
  meanRho_->fillInConstant(0.0);

  covVp_ ->fillInConstant(0.0);
  covVs_ ->fillInConstant(0.0);
  covRho_->fillInConstant(0.0);

  crCovVpVs_ ->fillInConstant(0.0);
  crCovVpRho_->fillInConstant(0.0);
  crCovVsRho_->fillInConstant(0.0);
}
//--------------------------------------------------------------------
void
SeismicParametersHolder::invFFTAllGrids()
{
  LogKit::LogFormatted(LogKit::High,"\nBacktransforming background grids from FFT domain to time domain...");

  if(meanVp_->getIsTransformed())
    meanVp_->invFFTInPlace();

  if(meanVs_->getIsTransformed())
    meanVs_->invFFTInPlace();

  if(meanRho_->getIsTransformed())
    meanRho_->invFFTInPlace();
  LogKit::LogFormatted(LogKit::High,"...done\n");

  invFFTCovGrids();
}

//--------------------------------------------------------------------
void
SeismicParametersHolder::FFTAllGrids()
{
  LogKit::LogFormatted(LogKit::High,"\nTransforming background grids from time domain to FFT domain ...");

  if(!meanVp_->getIsTransformed())
    meanVp_->fftInPlace();

  if(!meanVs_->getIsTransformed())
    meanVs_->fftInPlace();

  if(!meanRho_->getIsTransformed())
    meanRho_->fftInPlace();
  LogKit::LogFormatted(LogKit::High,"...done\n");

  FFTCovGrids();

}
//-----------------------------------------------------------------------------------------

void
SeismicParametersHolder::invFFTCovGrids()
{
  LogKit::LogFormatted(LogKit::High,"\nBacktransforming correlation grids from FFT domain to time domain...");

  if (covVp_->getIsTransformed())
    covVp_->invFFTInPlace();

  if (covVs_->getIsTransformed())
    covVs_->invFFTInPlace();

  if (covRho_->getIsTransformed())
    covRho_->invFFTInPlace();

  if (crCovVpVs_->getIsTransformed())
    crCovVpVs_->invFFTInPlace();

  if (crCovVpRho_->getIsTransformed())
    crCovVpRho_->invFFTInPlace();

  if (crCovVsRho_->getIsTransformed())
    crCovVsRho_->invFFTInPlace();

  LogKit::LogFormatted(LogKit::High,"...done\n");
}
//--------------------------------------------------------------------
void
SeismicParametersHolder::FFTCovGrids()
{
  LogKit::LogFormatted(LogKit::High,"\nTransforming correlation grids in seismic parameters holder from time domain to FFT domain...");

  if (!covVp_->getIsTransformed())
    covVp_->fftInPlace();

  if (!covVs_->getIsTransformed())
    covVs_->fftInPlace();

  if (!covRho_->getIsTransformed())
    covRho_->fftInPlace();

  if (!crCovVpVs_->getIsTransformed())
    crCovVpVs_->fftInPlace();

  if (!crCovVpRho_->getIsTransformed())
    crCovVpRho_->fftInPlace();

  if (!crCovVsRho_->getIsTransformed())
    crCovVsRho_->fftInPlace();

  LogKit::LogFormatted(LogKit::High,"...done\n");
}

//--------------------------------------------------------------------------------------------------
void
SeismicParametersHolder::getNextParameterCovariance(fftw_complex **& parVar) const
{

  fftw_complex ii;
  fftw_complex jj;
  fftw_complex kk;
  fftw_complex ij;
  fftw_complex ik;
  fftw_complex jk;

  fftw_complex iiTmp = covVp_     ->getNextComplex();
  fftw_complex jjTmp = covVs_     ->getNextComplex();
  fftw_complex kkTmp = covRho_    ->getNextComplex();
  fftw_complex ijTmp = crCovVpVs_ ->getNextComplex();
  fftw_complex ikTmp = crCovVpRho_->getNextComplex();
  fftw_complex jkTmp = crCovVsRho_->getNextComplex();

  if(priorVar0_(0,0) != 0)
    iiTmp.re = iiTmp.re / static_cast<float>(priorVar0_(0,0));

  if(priorVar0_(1,1) != 0)
    jjTmp.re = jjTmp.re / static_cast<float>(priorVar0_(1,1));

  if(priorVar0_(2,2) != 0)
    kkTmp.re = kkTmp.re / static_cast<float>(priorVar0_(2,2));

  if(priorVar0_(0,1) != 0)
    ijTmp.re = ijTmp.re / static_cast<float>(priorVar0_(0,1));

  if(priorVar0_(0,2) != 0)
    ikTmp.re = ikTmp.re / static_cast<float>(priorVar0_(0,2));

  if(priorVar0_(1,2) != 0)
    jkTmp.re = jkTmp.re / static_cast<float>(priorVar0_(1,2));

  ii.re = float( sqrt(iiTmp.re * iiTmp.re));
  ii.im = 0.0;
  jj.re = float( sqrt(jjTmp.re * jjTmp.re));
  jj.im = 0.0;
  kk.re = float( sqrt(kkTmp.re * kkTmp.re));
  kk.im = 0.0;
  ij.re = float( sqrt(ijTmp.re * ijTmp.re));
  ij.im = 0.0;
  ik.re = float( sqrt(ikTmp.re * ikTmp.re));
  ik.im = 0.0;
  jk.re = float( sqrt(jkTmp.re * jkTmp.re));
  jk.im = 0.0;

  parVar[0][0].re = ii.re * static_cast<float>(priorVar0_(0,0));
  parVar[0][0].im = ii.im;

  parVar[1][1].re = jj.re * static_cast<float>(priorVar0_(1,1));
  parVar[1][1].im = jj.im;

  parVar[2][2].re = kk.re * static_cast<float>(priorVar0_(2,2));
  parVar[2][2].im = kk.im;

  parVar[0][1].re = ij.re * static_cast<float>(priorVar0_(0,1));
  parVar[0][1].im = ij.im;
  parVar[1][0].re = ij.re * static_cast<float>(priorVar0_(0,1));
  parVar[1][0].im = -ij.im;

  parVar[0][2].re = ik.re * static_cast<float>(priorVar0_(0,2));
  parVar[0][2].im = ik.im;
  parVar[2][0].re = ik.re * static_cast<float>(priorVar0_(0,2));
  parVar[2][0].im = -ik.im;

  parVar[1][2].re = jk.re * static_cast<float>(priorVar0_(1,2));
  parVar[1][2].im = jk.im;
  parVar[2][1].re = jk.re * static_cast<float>(priorVar0_(1,2));
  parVar[2][1].im = -jk.im;
}


//--------------------------------------------------------------------
fftw_real *  SeismicParametersHolder::ComputeCircAutoCov(const std::vector<double>            & corr_t_pos,
                                                         const std::vector<double>            & corr_t_neg,
                                                         int                                    nzp) const
{
  //assert(auto_cov[0].numCols() >= static_cast<int>(j) && auto_cov[0].numRows() >= static_cast<int>(i));

  bool even = true;
  if (nzp%2 != 0)
    even = false;

  size_t n = corr_t_pos.size();

  fftw_real * circ_auto_cov = reinterpret_cast<fftw_real*>(fftw_malloc(2*(nzp/2+1)*sizeof(fftw_real)));
  //fftw_real * circ_auto_cov = reinterpret_cast<fftw_real*>(fftw_malloc(nzp*sizeof(fftw_real)));
  //size_t vector_len = corr_t_pos.size();


  for (int k = 0; k < nzp; k++)
    circ_auto_cov[k] = 0;       // Initialize to 0

  for (size_t k = 0; k < n; k++){
    circ_auto_cov[k] = static_cast<fftw_real>(corr_t_pos[k]);
    if (k < n -1)
      circ_auto_cov[nzp-k-1] = static_cast<fftw_real>(corr_t_neg[k + 1]);
  }

  for(int k = nzp; k < 2*(nzp/2+1); k++)
    circ_auto_cov[k] = RMISSING;


  /*
  for(int k = 0; k < 2*(nzp/2+1); k++ ) {
    if(k < nzp) {
      size_t refk;

      if(k < nzp/2+1){
        refk      = k;
        positive  = true;
      }
      else{
        refk      = nzp - k;
        positive  = false;
      }

      if(refk < n){
        //circ_auto_cov[k] = static_cast<fftw_real>(auto_cov[refk](i,j));
        if (positive)
          circ_auto_cov[k] = static_cast<fftw_real>(corr_t_pos[refk]);
        else
          circ_auto_cov[k] = static_cast<fftw_real>(corr_t_neg[refk]);
      }
      else
        circ_auto_cov[k] = 0.0;
    }
    else
      circ_auto_cov[k] = RMISSING;
  }
  */

  return circ_auto_cov;
}

//--------------------------------------------------------------------
fftw_real *
SeismicParametersHolder::computeCircCorrT(const std::vector<double> & priorCorrT,
                                          const int                 & minIntFq,
                                          const int                 & nzp) const
{
  assert(priorCorrT[0] != 0);

  int n = static_cast<int>(priorCorrT.size());

  fftw_real * circCorrT = reinterpret_cast<fftw_real*>(fftw_malloc(2*(nzp/2+1)*sizeof(fftw_real)));

  for(int k = 0; k < 2*(nzp/2+1); k++ ) {
    if(k < nzp) {
      int refk;

      if(k < nzp/2+1)
        refk = k;
      else
        refk = nzp - k;

      if(refk < n)
        circCorrT[k] = static_cast<fftw_real>(priorCorrT[refk]);
      else
        circCorrT[k] = 0.0;
    }
    else
      circCorrT[k] = RMISSING;
  }

  makeCircCorrTPosDef(circCorrT, minIntFq, nzp);

  return circCorrT;

}


//--------------------------------------------------------------------

void  SeismicParametersHolder::MakeCircAutoCovPosDef(std::vector<std::vector<fftw_real *> >   & circ_auto_cov,
                                                     int                                        nzp) const
{
  const int vector_len = static_cast<size_t>(nzp/2) + 1;

  // FFT of each circular auto cov function

  std::vector<std::vector<fftw_complex * > >  fft_circ_auto_cov(3); // should not be deleted
  std::vector<std::vector<fftw_real *> >      circ_auto_cov_copy(3);
  for(int i = 0; i < 3; i++){
    fft_circ_auto_cov[i].resize(3, NULL);
    circ_auto_cov_copy[i].resize(3, NULL);
    for (int j = 0; j < 3; j++){
      circ_auto_cov_copy[i][j] = new fftw_real[nzp];
      //fft_circ_auto_cov[i][j] = new fftw_complex[nzp];
      for (int k = 0; k < nzp; k++){
        circ_auto_cov_copy[i][j][k] =       circ_auto_cov[i][j][k];
      }
      fft_circ_auto_cov[i][j] = FFTGrid::fft1DzInPlace(circ_auto_cov[i][j], nzp);
    }
  }

  // Scale by the variance of each parameter in lag 0
  NRLib::ComplexMatrix     diag_std_var_inv(3, 3);
  NRLib::ComplexMatrix     diag_std_var(3,3);
  std::complex<double>     zero_complex(0.0,0.0);
  NRLib::InitializeComplexMatrix(diag_std_var_inv, zero_complex);
  NRLib::InitializeComplexMatrix(diag_std_var, zero_complex);

  for (int i = 0; i < 3; i++){
    diag_std_var_inv(i,i) = static_cast<double>(1.0/std::sqrt(circ_auto_cov_copy[i][i][0]));
    diag_std_var(i,i) = static_cast<double>(std::sqrt(circ_auto_cov_copy[i][i][0]));
  }

  NRLib::ComplexMatrix   eig_vectors(3,3);
  NRLib::ComplexVector   eig_values(3);
  std::vector<NRLib::ComplexMatrix>   cov_dens_adjusted(nzp);
  NRLib::ComplexMatrix   eigen_values_mat(3,3);

  NRLib::ComplexMatrix cov_dens_i(3,3);
  NRLib::ComplexMatrix product_mat_1(3,3);
  NRLib::ComplexMatrix product_mat_2(3,3);
  NRLib::ComplexMatrix product_mat_3(3,3);
  for (int i = 0; i < nzp; i++){
    cov_dens_adjusted[i].resize(3,3);
    NRLib::InitializeComplexMatrix(cov_dens_adjusted[i], zero_complex);
  }
  NRLib::InitializeComplexMatrix(cov_dens_i, zero_complex);
  NRLib::InitializeComplexMatrix(product_mat_1, zero_complex);
  NRLib::InitializeComplexMatrix(product_mat_2, zero_complex);
  NRLib::InitializeComplexMatrix(eigen_values_mat, zero_complex);
  for (int i = 0; i < vector_len; i++){
    // need to convert fftw_complex to std::complex to use NRLib::ComplexMatrix
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < 3; k++){
        if (i < vector_len){
          std::complex<double> temp_complex_std(fft_circ_auto_cov[j][k][i].re, fft_circ_auto_cov[j][k][i].im);
          cov_dens_i(j,k) =  temp_complex_std;
        }
      }
    }

    product_mat_1     = diag_std_var_inv*cov_dens_i;      // temp variable
    product_mat_2      = product_mat_1*diag_std_var_inv;  // temp variable
    NRLib::ComputeEigenVectorsComplex(product_mat_2, eig_values, eig_vectors);
    for(int j = 0; j < 3; j++){
      std::complex<double> eig_val = eig_values(j);
      std::vector<std::complex<double> > eig_vec(3);
      eigen_values_mat(j,j) = abs(eig_values(j));         // Make negative eigenvalues positive
    }

    product_mat_1 = diag_std_var * eig_vectors;                               // temp variable
    product_mat_2 = product_mat_1 * eigen_values_mat;                         // temp variable
    product_mat_3 = product_mat_2 * NRLib::conjugateTranspose(eig_vectors);   // temp variable
    cov_dens_adjusted[i] = product_mat_3 * diag_std_var;
  }

  // convert from std::complex to fftw_complex
  fftw_complex temp_complex_fftw;
  for(int j = 0; j < 3; j++){
    for(int k = 0; k < 3; k++){
      for(int i = 0; i < vector_len; i++){
        temp_complex_fftw.re = static_cast<fftw_real>(cov_dens_adjusted[i](j,k).real());
        temp_complex_fftw.im = static_cast<fftw_real>(cov_dens_adjusted[i](j,k).imag());
        fft_circ_auto_cov[j][k][i] = temp_complex_fftw;
      }
      circ_auto_cov[j][k] = FFTGrid::invFFT1DzInPlace(fft_circ_auto_cov[j][k], nzp);
    }
  }

  // multiply by fft factor
  float factor = static_cast<float>(1.0/static_cast<double>(nzp));
  for(int j = 0; j < 3; j++){
    for(int k = 0; k < 3; k++){
      for(int i = 0; i < nzp; i++){
        circ_auto_cov[j][k][i] *= factor;
      }
    }
  }


  for (int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      delete [] circ_auto_cov_copy[i][j];
    }
  }
}


void      SeismicParametersHolder::TaperCircAutoCovFunction(std::vector<std::vector<fftw_real *> >    & circ_auto_cov,
                                                            int                                         nzp,
                                                            double                                      dz) const
{
  double eps                                  = std::pow(10.0,-10.0);
  double std_dev                              = static_cast<double>(50)/dz;   // 50 ms is the standard deviation, ref Odd Kolbjørnsen
  if (std_dev > static_cast<double>(nzp)/2.0)
    std_dev                                   = static_cast<double>(nzp)/2.0; // If the interval is short, set std dev to nzp/2

  fftw_real * taper                           = new fftw_real[nzp];
  for (int i = 0; i < nzp; i++){
    if (i < nzp/2 + 1){
      if (std::exp(-0.5*std::pow(static_cast<float>(i)/std_dev,2)) > eps)
        taper[i] = static_cast<fftw_real>(std::exp(-0.5*std::pow(static_cast<float>(i)/std_dev,2)));
      else
        taper[i] = 0.0;
    }
    else{
      if (std::exp(-0.5*std::pow(static_cast<float>(nzp - i)/std_dev,2)) > eps)
        taper[i] = static_cast<fftw_real>(std::exp(-0.5*std::pow(static_cast<float>(nzp - i)/std_dev,2)));
      else
        taper[i] = 0.0;
    }
  }

  for(int i = 0; i < 3; i++){
    for (int j = 0; j < 3; j++){
      for (int k = 0; k < nzp; k++){
        circ_auto_cov[i][j][k] *= taper[k];
      }
    }
  }
  delete [] taper;
}

void      SeismicParametersHolder::makeCircCorrTPosDef(fftw_real * circCorrT,
                                                       const int & minIntFq,
                                                       const int & nzp) const
{
  fftw_complex * fftCircCorrT;
  fftCircCorrT = FFTGrid::fft1DzInPlace(circCorrT, nzp);

  for(int k=0; k<nzp/2+1; k++) {
    if(k <= minIntFq)
      fftCircCorrT[k].re = 0.0 ;
    else
      fftCircCorrT[k].re = float(sqrt(fftCircCorrT[k].re * fftCircCorrT[k].re +
                                      fftCircCorrT[k].im * fftCircCorrT[k].im ));
    fftCircCorrT[k].im = 0.0;
  }

  circCorrT   = FFTGrid::invFFT1DzInPlace(fftCircCorrT, nzp);
  //
  // NBNB-PAL: If the number of layers is too small CircCorrT[0] = 0. How
  //           do we avoid this, or how do we flag the problem?
  //
  double scale = 0;
  if (circCorrT[0] > 1.e-5f) // NBNB-PAL: Temporary solution for above mentioned problem
    scale = float( 1.0f/circCorrT[0] );
  else  {
    LogKit::LogFormatted(LogKit::Low,"\nERROR: The circular temporal correlation (CircCorrT) is undefined. You\n");
    LogKit::LogFormatted(LogKit::Low,"       probably need to increase the number of layers...\n\nAborting\n");
    exit(1);
  }

  for(int k=0; k<nzp; k++)
    circCorrT[k] *= static_cast<float>(scale);
}


//--------------------------------------------------------------------
float *
SeismicParametersHolder::getPriorCorrTFiltered(int nz, int nzp) const
{
  // This is the cyclic and filtered version of CorrT which
  // has one or more zeros in the middle

  fftw_real * circCorrT = extractParamCorrFromCovVp(nzp);

  float * priorCorrTFiltered = new float[nzp];

  for(int i=0; i<nzp; i++ ) {
    int refk;

    if( i < nzp/2+1 )
      refk = i;
    else
      refk = nzp - i;

    if(refk < nz && circCorrT != NULL)
      priorCorrTFiltered[i] = circCorrT[refk];
    else
      priorCorrTFiltered[i] = 0.0;
  }

  fftw_free(circCorrT);

  return priorCorrTFiltered;
}

//--------------------------------------------------------------------
void
SeismicParametersHolder::printPriorVariances(void) const
{
  LogKit::LogFormatted(LogKit::Low,"\nVariances and correlations for parameter residuals:\n");
  LogKit::LogFormatted(LogKit::Low,"\n");
  LogKit::LogFormatted(LogKit::Low,"Variances           ln Vp     ln Vs    ln Rho         \n");
  LogKit::LogFormatted(LogKit::Low,"---------------------------------------------------------------\n");
  LogKit::LogFormatted(LogKit::Low,"Inversion grid:   %.1e   %.1e   %.1e (used by program)\n",priorVar0_(0,0),priorVar0_(1,1),priorVar0_(2,2));

  float corr01 = static_cast<float>(priorVar0_(0,1)/(sqrt(priorVar0_(0,0)*priorVar0_(1,1))));
  float corr02 = static_cast<float>(priorVar0_(0,2)/(sqrt(priorVar0_(0,0)*priorVar0_(2,2))));
  float corr12 = static_cast<float>(priorVar0_(1,2)/(sqrt(priorVar0_(1,1)*priorVar0_(2,2))));
  LogKit::LogFormatted(LogKit::Low,"\n");
  LogKit::LogFormatted(LogKit::Low,"Corr   | ln Vp     ln Vs    ln Rho \n");
  LogKit::LogFormatted(LogKit::Low,"-------+---------------------------\n");
  LogKit::LogFormatted(LogKit::Low,"ln Vp  | %5.2f     %5.2f     %5.2f \n",1.0f, corr01, corr02);
  LogKit::LogFormatted(LogKit::Low,"ln Vs  |           %5.2f     %5.2f \n",1.0f, corr12);
  LogKit::LogFormatted(LogKit::Low,"ln Rho |                     %5.2f \n",1.0f);
  LogKit::LogFormatted(LogKit::Low,"\n");

  if (std::abs(corr01) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vp-Vs correlation is wrong (%.2f).\n",corr01);
    TaskList::addTask("Check your prior correlations. Corr(Vp,Vs) is out of bounds.");
  }
  if (std::abs(corr02) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vp-Rho correlation is wrong (%.2f).\n",corr02);
    TaskList::addTask("Check your prior correlations. Corr(Vp,Rho) is out of bounds.");
  }
  if (std::abs(corr12) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vs-Rho correlation is wrong (%.2f).\n",corr12);
    TaskList::addTask("Check your prior correlations. Corr(Vs,Rho) is out of bounds.");
  }
}
//--------------------------------------------------------------------
void
SeismicParametersHolder::printPostVariances(const NRLib::Matrix & postVar0) const
{
  LogKit::WriteHeader("Posterior Covariance");

  LogKit::LogFormatted(LogKit::Low,"\nVariances and correlations for parameter residuals:\n");
  LogKit::LogFormatted(LogKit::Low,"\n");
  LogKit::LogFormatted(LogKit::Low,"               ln Vp     ln Vs    ln Rho \n");
  LogKit::LogFormatted(LogKit::Low,"-----------------------------------------\n");
  LogKit::LogFormatted(LogKit::Low,"Variances:   %.1e   %.1e   %.1e    \n",postVar0(0,0),postVar0(1,1),postVar0(2,2));
  LogKit::LogFormatted(LogKit::Low,"\n");
  float corr01 = static_cast<float>(postVar0(0,1)/(sqrt(postVar0(0,0)*postVar0(1,1))));
  float corr02 = static_cast<float>(postVar0(0,2)/(sqrt(postVar0(0,0)*postVar0(2,2))));
  float corr12 = static_cast<float>(postVar0(1,2)/(sqrt(postVar0(1,1)*postVar0(2,2))));
  LogKit::LogFormatted(LogKit::Low,"Corr   | ln Vp     ln Vs    ln Rho \n");
  LogKit::LogFormatted(LogKit::Low,"-------+---------------------------\n");
  LogKit::LogFormatted(LogKit::Low,"ln Vp  | %5.2f     %5.2f     %5.2f \n",1.0f, corr01, corr02);
  LogKit::LogFormatted(LogKit::Low,"ln Vs  |           %5.2f     %5.2f \n",1.0f, corr12);
  LogKit::LogFormatted(LogKit::Low,"ln Rho |                     %5.2f \n",1.0f);

  if (std::abs(corr01) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vp-Vs correlation is wrong (%.2f).\n",corr01);
    TaskList::addTask("Check your posterior correlations. Corr(Vp,Vs) is out of bounds.");
  }
  if (std::abs(corr02) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vp-Rho correlation is wrong (%.2f).\n",corr02);
    TaskList::addTask("Check your posterior correlations. Corr(Vp,Rho) is out of bounds.");
  }
  if (std::abs(corr12) > 1.0) {
    LogKit::LogFormatted(LogKit::Warning,"\nWARNING: The Vs-Rho correlation is wrong (%.2f).\n",corr12);
    TaskList::addTask("Check your posterior correlations. Corr(Vs,Rho) is out of bounds.");
  }
}

//--------------------------------------------------------------------
//void
//SeismicParametersHolder::writeFilePostVariances(const NRLib::Matrix      & postVar0,
//                                                const std::vector<float> & postCovVp00,
//                                                const std::vector<float> & postCovVs00,
//                                                const std::vector<float> & postCovRho00) const
//{
//  std::string baseName = IO::PrefixPosterior() + IO::FileParameterCov() + IO::SuffixGeneralData();
//  std::string fileName = IO::makeFullFileName(IO::PathToCorrelations(), baseName);
//
//  std::ofstream file;
//  NRLib::OpenWrite(file, fileName);
//  file << std::fixed;
//  file << std::right;
//  file << std::setprecision(6);
//  for(int i=0 ; i<3 ; i++) {
//    for(int j=0 ; j<3 ; j++) {
//      file << std::setw(10) << postVar0(i,j) << " ";
//    }
//    file << "\n";
//  }
//  file.close();
//
//  std::string baseName1 = IO::PrefixPosterior() + IO::PrefixTemporalCorr()+"Vp" +IO::SuffixGeneralData();
//  std::string baseName2 = IO::PrefixPosterior() + IO::PrefixTemporalCorr()+"Vs" +IO::SuffixGeneralData();
//  std::string baseName3 = IO::PrefixPosterior() + IO::PrefixTemporalCorr()+"Rho"+IO::SuffixGeneralData();
//  writeFilePostCorrT(postCovVp00,  IO::PathToCorrelations(), baseName1);
//  writeFilePostCorrT(postCovVs00,  IO::PathToCorrelations(), baseName2);
//  writeFilePostCorrT(postCovRho00, IO::PathToCorrelations(), baseName3);
//}

//--------------------------------------------------------------------
//void
//SeismicParametersHolder::writeFilePostCovGrids(Simbox const * simbox) const
//{
//  std::string fileName;
//  fileName = IO::PrefixPosterior() + IO::PrefixCovariance() + "Vp";
//  covVp_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  covVp_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior covariance for Vp");
//  covVp_ ->endAccess();
//
//  fileName = IO::PrefixPosterior() + IO::PrefixCovariance() + "Vs";
//  covVs_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  covVs_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior covariance for Vs");
//  covVs_ ->endAccess();
//
//  fileName = IO::PrefixPosterior() + IO::PrefixCovariance() + "Rho";
//  covRho_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  covRho_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior covariance for density");
//  covRho_ ->endAccess();
//
//  fileName = IO::PrefixPosterior() + IO::PrefixCrossCovariance() + "VpVs";
//  crCovVpVs_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  crCovVpVs_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior cross-covariance for (Vp,Vs)");
//  crCovVpVs_ ->endAccess();
//
//  fileName = IO::PrefixPosterior() + IO::PrefixCrossCovariance() + "VpRho";
//  crCovVpRho_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  crCovVpRho_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior cross-covariance for (Vp,density)");
//  crCovVpRho_ ->endAccess();
//
//  fileName = IO::PrefixPosterior() + IO::PrefixCrossCovariance() + "VsRho";
//  crCovVsRho_ ->setAccessMode(FFTGrid::RANDOMACCESS);
//  crCovVsRho_ ->writeFile(fileName, IO::PathToCorrelations(), simbox, "Posterior cross-covariance for (Vs,density)");
//  crCovVsRho_ ->endAccess();
//}

//-------------------------------------------------------------------
fftw_real *
SeismicParametersHolder::extractParamCorrFromCovVp(int nzp) const
{
  assert(covVp_->getIsTransformed() == false);

  covVp_->setAccessMode(FFTGrid::RANDOMACCESS);

  fftw_real * circCorrT = reinterpret_cast<fftw_real*>(fftw_malloc(2*(nzp/2+1)*sizeof(fftw_real)));
  //int         refk;
  float       constant = covVp_->getRealValue(0,0,0);

  for(int k = 0 ; k < 2*(nzp/2+1) ; k++ ){
    if(k < nzp)
      circCorrT[k] = covVp_->getRealValue(0,0,k,true)/constant;
    else
      circCorrT[k] = RMISSING;
  }

  covVp_->endAccess();

  return circCorrT;//fftw_free(circCorrT);
}
//--------------------------------------------------------------------
void
SeismicParametersHolder::updatePriorVar()
{
  priorVar0_(0,0) = getOrigin(covVp_);
  priorVar0_(1,1) = getOrigin(covVs_);
  priorVar0_(2,2) = getOrigin(covRho_);
  priorVar0_(0,1) = getOrigin(crCovVpVs_);
  priorVar0_(1,0) = priorVar0_(0,1);
  priorVar0_(2,0) = getOrigin(crCovVpRho_);
  priorVar0_(0,2) = priorVar0_(2,0);
  priorVar0_(2,1) = getOrigin(crCovVsRho_);
  priorVar0_(1,2) = priorVar0_(2,1);

}
//--------------------------------------------------------------------
float
SeismicParametersHolder::getOrigin(FFTGrid * grid) const
{
  grid->setAccessMode(FFTGrid::RANDOMACCESS);
  float value = grid->getRealValue(0,0,0);
  grid->endAccess();
  return value;
}

//--------------------------------------------------------------------
std::vector<float>
SeismicParametersHolder::createPostCov00(FFTGrid * postCov) const
{
  int nz = postCov->getNz();
  std::vector<float> postCov00(nz);

  postCov->setAccessMode(FFTGrid::RANDOMACCESS);
  for(int k=0 ; k < nz ; k++)
    postCov00[k] = postCov->getRealValue(0,0,k);

  postCov->endAccess();
  return postCov00;
}

void SeismicParametersHolder::releaseGrids()
{
  if (postVp_ != NULL)
    delete postVp_;

  if (postVs_ != NULL)
    delete postVs_;

  if (postRho_ != NULL)
    delete postRho_;

  if (postVpKriged_ != NULL)
    delete postVpKriged_;

  if (postVsKriged_ != NULL)
    delete postVsKriged_;

  if (postRhoKriged_ != NULL)
    delete postRhoKriged_;

  if (block_grid_ != NULL)
    delete block_grid_;

  if (facies_prob_undef_ != NULL)
    delete facies_prob_undef_;

  if (quality_grid_ != NULL)
    delete quality_grid_;

  for (size_t i = 0; i < simulations_seed0_.size(); i++) {
    if (simulations_seed0_[i] != NULL)
      delete simulations_seed0_[i];
  }

  for (size_t i = 0; i < simulations_seed1_.size(); i++) {
    if (simulations_seed1_[i] != NULL)
      delete simulations_seed1_[i];
  }

  for (size_t i = 0; i < simulations_seed2_.size(); i++) {
    if (simulations_seed2_[i] != NULL)
      delete simulations_seed2_[i];
  }

  //for (size_t i = 0; i < synt_seismic_data_.size(); i++) {
  //  if (synt_seismic_data_[i] != NULL)
  //    delete synt_seismic_data_[i];
  //}

  //for (size_t i = 0; i < synt_residuals_.size(); i++) {
  //  if (synt_residuals_[i] != NULL)
  //    delete synt_residuals_[i];
  //}

  for (size_t i = 0; i < facies_prob_.size(); i++) {
    if (facies_prob_[i] != NULL)
      delete facies_prob_[i];
  }

  for (size_t i = 0; i < facies_prob_geo_.size(); i++) {
    if (facies_prob_geo_[i] != NULL)
      delete facies_prob_geo_[i];
  }

  for (size_t i = 0; i < lh_cube_.size(); i++) {
    if (lh_cube_[i] != NULL)
      delete lh_cube_[i];
  }
}

//FFTGrid*
//SeismicParametersHolder::copyFFTGrid(FFTGrid * fft_grid_old)
//{
//  FFTGrid* fft_grid;
//  if(fft_grid_old->isFile())
//    fft_grid = new FFTFileGrid(reinterpret_cast<FFTFileGrid*>(fft_grid_old));
//  else
//    fft_grid = new FFTGrid(fft_grid_old);
//
//  return(fft_grid);
//}

FFTGrid*
SeismicParametersHolder::copyFFTGrid(FFTGrid * fft_grid_old)
{
  FFTGrid* fft_grid;
  if(fft_grid_old->isFile()) {
    fft_grid_old->endAccess();
    fft_grid = new FFTFileGrid(reinterpret_cast<FFTFileGrid*>(fft_grid_old));
    //fft_grid = new FFTGrid(reinterpret_cast<FFTGrid*>(fft_grid_old));
  }
  else
    fft_grid = new FFTGrid(fft_grid_old);

  return(fft_grid);
}

void SeismicParametersHolder::releaseExpGrids() const
{
  if (meanVp_ != NULL)
    delete meanVp_;
  if (meanVs_ != NULL)
    delete meanVs_;
  if (meanRho_ != NULL)
    delete meanRho_;
}
