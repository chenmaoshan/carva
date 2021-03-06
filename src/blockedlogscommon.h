/***************************************************************************
*      Copyright (C) 2008 by Norwegian Computing Center and Statoil        *
***************************************************************************/

#ifndef BLOCKEDLOGSCOMMON_H
#define BLOCKEDLOGSCOMMON_H

#include "src/commondata.h"
#include "fftw.h"
#include <map>
#include <string>
#include "nrlib/well/well.hpp"
#include "src/definitions.h"
#include "src/fftgrid.h"
#include "src/seismicstorage.h"

class CravaTrend;
class MultiIntervalGrid;

class BlockedLogsCommon{
public:

  // Constructor for correlation estimation blocking
  BlockedLogsCommon(NRLib::Well                      * well_data,
                    const std::vector<std::string>   & cont_logs_to_be_blocked,
                    const std::vector<std::string>   & disc_logs_to_be_blocked,
                    const MultiIntervalGrid          * multiple_interval_grid,
                    bool                               interpolate,
                    bool                             & is_inside,
                    std::string                      & err_text);

  // Constructor for blocking in the surrounding estimation simbox
  BlockedLogsCommon(const NRLib::Well                * well_data,
                    const std::vector<std::string>   & cont_logs_to_be_blocked,
                    const std::vector<std::string>   & disc_logs_to_be_blocked,
                    const Simbox                     * const estimation_simbox,
                    bool                               interpolate,
                    bool                               visible_only,
                    bool                             & is_inside,
                    std::string                      & err_text);

  // Constructor for wavelet estimation blocking
  //BlockedLogsCommon(const NRLib::Well   * well_data,
  //                  const StormContGrid & stormgrid);

  // Constructor for blocked logs for rock physics
  BlockedLogsCommon(const NRLib::Well              * well_data,
                    const Simbox                   * simbox,
                    const CravaTrend               & trend_cubes,
                    const std::vector<std::string> & cont_logs_to_be_blocked,
                    const std::vector<std::string> & disc_logs_to_be_blocked,
                    std::string                    & err_text);

  //Copy constructor
  BlockedLogsCommon(const BlockedLogsCommon & logs);

  ~BlockedLogsCommon();


  //GET FUNCTIONS --------------------------------

  // Number of blocks with data per interval name s
  int                                    GetNBlocksWithData(std::string s) const { return n_blocks_with_data_.find(s)->second                         ;}
  // Total number of blocks with data across all intervals
  int                                    GetNBlocksWithDataTot() const { return n_blocks_with_data_tot_                                               ;}
  std::string                            GetWellName()         const   { return well_name_                                                            ;}
  // Total number of blocks
  int                                    GetNumberOfBlocks()   const   { return n_blocks_                                                             ;}
  int                                    GetNFacies()          const   { return static_cast<int>(facies_map_.size())                                  ;}
  std::vector<double>                  & GetXposBlocked(void)          { return x_pos_blocked_                                                        ;}
  std::vector<double>                  & GetYposBlocked(void)          { return y_pos_blocked_                                                        ;}
  std::vector<double>                  & GetZposBlocked(void)          { return z_pos_blocked_                                                        ;}
  const std::vector<double>            & GetXposBlocked(void)  const   { return x_pos_blocked_                                                        ;}
  const std::vector<double>            & GetYposBlocked(void)  const   { return y_pos_blocked_                                                        ;}
  const std::vector<double>            & GetZposBlocked(void)  const   { return z_pos_blocked_                                                        ;}
  const std::vector<double>            & GetXposRawLogs()      const   { return x_pos_raw_logs_                                                       ;}
  const std::vector<double>            & GetYposRawLogs()      const   { return y_pos_raw_logs_                                                       ;}
  const std::vector<double>            & GetZposRawLogs()      const   { return z_pos_raw_logs_                                                       ;}

  const std::map<int, std::string>     & GetFaciesMap()        const   { return facies_map_                                                           ;}
  std::vector<int>                     & GetSposVector()               { return s_pos_                                                                ;}
  std::vector<int>                     & GetIposVector()               { return i_pos_                                                                ;}
  std::vector<int>                     & GetJposVector()               { return j_pos_                                                                ;}
  std::vector<int>                     & GetKposVector()               { return k_pos_                                                                ;}
  const std::vector<int>               & GetSposVector()       const   { return s_pos_                                                                ;}
  const std::vector<int>               & GetIposVector()       const   { return i_pos_                                                                ;}
  const std::vector<int>               & GetJposVector()       const   { return j_pos_                                                                ;}
  const std::vector<int>               & GetKposVector()       const   { return k_pos_                                                                ;}

  std::map<std::string, std::vector<double> > & GetContLogsBlocked()   { return continuous_logs_blocked_                                              ;}
  std::map<std::string, std::vector<int> > & GetDiscLogsBlocked()      { return discrete_logs_blocked_                                                ;}
  const std::vector<double>            & GetContLogBlocked(std::string s) const {return continuous_logs_blocked_.find(s)->second                      ;}
  const std::vector<int>               & GetDiscLogBlocked(std::string s) const {return discrete_logs_blocked_.find(s)->second                        ;}
  const std::vector<double>            & GetVpBlocked(void)    const   { return continuous_logs_blocked_.find("Vp")->second                           ;}
        std::vector<double>            & GetVpBlocked(void)            { return continuous_logs_blocked_.find("Vp")->second                           ;}
  const std::vector<double>            & GetVsBlocked(void)    const   { return continuous_logs_blocked_.find("Vs")->second                           ;}
        std::vector<double>            & GetVsBlocked(void)            { return continuous_logs_blocked_.find("Vs")->second                           ;}
  const std::vector<double>            & GetRhoBlocked(void)   const   { return continuous_logs_blocked_.find("Rho")->second                          ;}
        std::vector<double>            & GetRhoBlocked(void)           { return continuous_logs_blocked_.find("Rho")->second                          ;}
  const std::vector<double>            & GetMDBlocked(void)    const   { return continuous_logs_blocked_.find("MD")->second                           ;}
  std::vector<int>                     & GetFaciesBlocked()            { return facies_blocked_                                                       ;}
  const std::vector<int>               & GetFaciesBlocked(void) const  { return facies_blocked_                                                       ;}
  const std::vector<double>            & GetVpRawLogs(void)  const     { return continuous_raw_logs_.find("Vp")->second                               ;}
  const std::vector<double>            & GetVsRawLogs(void)  const     { return continuous_raw_logs_.find("Vs")->second                               ;}
  const std::vector<double>            & GetRhoRawLogs(void) const     { return continuous_raw_logs_.find("Rho")->second                              ;}

  std::map<std::string, std::vector<double> > & GetContLogsSeismicRes(){ return cont_logs_seismic_resolution_                                         ;}
  const std::vector<double>            & GetVpSeismicResolution(void)  const { return cont_logs_seismic_resolution_.find("Vp")->second                ;}
  const std::vector<double>            & GetVsSeismicResolution(void)  const { return cont_logs_seismic_resolution_.find("Vs")->second                ;}
  const std::vector<double>            & GetRhoSeismicResolution(void) const { return cont_logs_seismic_resolution_.find("Rho")->second               ;}

  std::map<std::string, std::vector<double> > & GetContLogsHighCutBg() { return cont_logs_highcut_background_                                         ;}
  const std::vector<double>            & GetContLogHighCutBackground(std::string s) const {return cont_logs_highcut_background_.find(s)->second       ;}
  const std::vector<double>            & GetVpHighCutBackground(void)  const { return cont_logs_highcut_background_.find("Vp")->second                ;}
  const std::vector<double>            & GetVsHighCutBackground(void)  const { return cont_logs_highcut_background_.find("Vs")->second                ;}
  const std::vector<double>            & GetRhoHighCutBackground(void) const { return cont_logs_highcut_background_.find("Rho")->second               ;}

  std::map<std::string, std::vector<double> > & GetContLogsHighCutSeismic() { return cont_logs_highcut_seismic_                                       ;}
  const std::vector<double>            & GetVpHighCutSeismic(void)  const { return cont_logs_highcut_seismic_.find("Vp")->second                      ;}
  const std::vector<double>            & GetVsHighCutSeismic(void)  const { return cont_logs_highcut_seismic_.find("Vs")->second                      ;}
  const std::vector<double>            & GetRhoHighCutSeismic(void) const { return cont_logs_highcut_seismic_.find("Rho")->second                     ;}

  std::map<std::string, std::vector<double> > & GetContLogsPredicted() { return continuous_logs_predicted_                                            ;}
  const std::vector<double>            & GetVpPredicted(void)  const { return continuous_logs_predicted_.find("Vp")->second                           ;}
  const std::vector<double>            & GetVsPredicted(void)  const { return continuous_logs_predicted_.find("Vs")->second                           ;}
  const std::vector<double>            & GetRhoPredicted(void) const { return continuous_logs_predicted_.find("Rho")->second                          ;}

  const std::vector<double>            & GetVpFaciesFiltered(void)  const { return vp_facies_filtered_                                                ;}
  const std::vector<double>            & GetRhoFaciesFiltered(void) const { return rho_facies_filtered_                                               ;}

  const std::vector<double>            & GetFaciesProb(int facies)     const { return facies_prob_.find(facies)->second                               ;}
  const std::vector<double>            & GetRealSeismicData(int angle) const { return real_seismic_data_.find(angle)->second                          ;}
  std::vector<std::vector<double> >    & GetActualSyntSeismicData()          { return actual_synt_seismic_data_                                       ;}
  std::vector<std::vector<double> >    & GetWellSyntSeismicData()            { return well_synt_seismic_data_                                         ;}

  const std::vector<int>               & GetIntervalLog()              const { return interval_log_                                                   ;}

  int                                    GetNRealSeismicData()               { return static_cast<int>(real_seismic_data_.size())                     ;}
  int                                    GetNFaciesProb()                    { return static_cast<int>(facies_prob_.size())                           ;}

  int                                    GetNAngles()                        { return n_angles_                                                       ;}

  //const std::vector<double>            & GetActualSyntSeismicData(int angle) const { return actual_synt_seismic_data_.find(angle)->second ;}
  //const std::vector<double>            & GetWellSyntSeismicData(int angle) const { return well_synt_seismic_data_.find(angle)->second ;}

  bool                                   GetIsDeviated(void)                const { return is_deviated_                                               ;}

  bool                                   GetUseForFaciesProbabilities(void) const { return (use_for_facies_probabilities_ > 0)                        ;}
  bool                                   GetUseForWaveletEstimation(void)   const { return (use_for_wavelet_estimation_ > 0)                          ;}
  bool                                   GetUseForFiltering(void)           const { return (use_for_filtering_ > 0)                                   ;}
  bool                                   GetUseForBackgroundTrend(void)     const { return (use_for_background_trend_ > 0)                            ;}
  bool                                   GetUseForRockPhysics(void)         const { return (use_for_rock_physics_ > 0)                                ;}

  double                                 GetDz(void)                        const { return dz_                                                        ;}

  int                                    GetFirstB(void)                    const { return first_B_                                                   ;}
  int                                    GetLastB(void)                     const { return last_B_                                                    ;}

  bool                                   HasSyntheticVsLog(void)            const { return (real_vs_log_ == 0)                                        ;}
  bool                                   HasContLog(std::string s)     { return (continuous_logs_blocked_.find(s) != continuous_logs_blocked_.end())  ;}
  bool                                   HasDiscLog(std::string s)     { return (discrete_logs_blocked_.find(s) != discrete_logs_blocked_.end())      ;}

  void                                   GetVerticalTrend(const std::vector<double>  & blocked_log,
                                                          std::vector<double>        & trend) const;

  void                                   GetVerticalTrend(const int        * blocked_log,
                                                          std::vector<int> & trend) const;

  void                                   GetVerticalTrendLimited(const std::vector<double>       & blocked_log,
                                                                 std::vector<double>             & trend,
                                                                 const std::vector<Surface *>    & limits) const;

  void                                   GetBlockedGrid(SeismicStorage         * grid,
                                                        const Simbox           * estimation_simbox,
                                                        std::vector<double>    & blocked_log,
                                                        int                      i_offset = 0,
                                                        int                      j_offset = 0) const;

  void                                   GetBlockedGrid(const NRLib::Grid<float> * grid,
                                                        std::vector<double>      & blocked_log,
                                                        int                        i_offset = 0,
                                                        int                        j_offset = 0) const;

  std::vector<double>                    GetVpForFacies(const std::string & facies_name);

  std::vector<double>                    GetVsForFacies(const std::string & facies_name);

  std::vector<double>                    GetRhoForFacies(const std::string & facies_name);

  std::vector<double>                    GetPorosityForFacies(const std::string & facies_name);

  std::vector<double>                    GetBulkForFacies(const std::string & facies_name);

  std::vector<double>                    GetShearForFacies(const std::string & facies_name);

  const std::vector<double>            & GetS1(void) {return s1_;}

  const std::vector<double>            & GetS2(void) {return s2_;}


  //SET FUNCTIONS --------------------------------

  void                                   SetNAngles(int n_angles)                                       { n_angles_                     = n_angles                     ;}
  void                                   SetDeviated(bool deviated)                                     { is_deviated_                  = deviated                     ;}
  void                                   SetRealVsLog(int real_vs_log)                                  { real_vs_log_                  = real_vs_log                  ;}
  void                                   SetUseForFaciesProbabilities(int use_for_facies_probabilities) { use_for_facies_probabilities_ = use_for_facies_probabilities ;}
  void                                   SetUseForBackgroundTrend(int use_for_background_trend)         { use_for_background_trend_     = use_for_background_trend     ;}
  void                                   SetUseForFiltering(int use_for_filtering)                      { use_for_filtering_            = use_for_filtering            ;}
  void                                   SetUseForWaveletEstimation(int use_for_wavelet_estimation)     { use_for_wavelet_estimation_   = use_for_wavelet_estimation   ;}
  void                                   SetUseForRockPhysics(int use_for_rock_physics)                 { use_for_rock_physics_         = use_for_rock_physics         ;}
  void                                   SetWellName(std::string & well_name)                           { well_name_                    = well_name                    ;}

  void                                   SetRealSeismicData(int angle, std::vector<double> log)         { real_seismic_data_.insert(std::pair<int, std::vector<double> >(angle, log)) ;}
  void                                   SetFaciesProb(int facies, std::vector<double> log)             { facies_prob_.insert(std::pair<int, std::vector<double> >(facies, log))      ;}

  void                                   SetVpFaciesFiltered(std::vector<double> vp_facies)             { vp_facies_filtered_  = vp_facies                                             ;}
  void                                   SetRhoFaciesFiltered(std::vector<double> rho_facies)           { rho_facies_filtered_ = rho_facies                                            ;}

  void                                   SetIntervalLog(std::vector<int> interval_log)                  { interval_log_ = interval_log                                                 ;}

  void                                   SetVpPredicted(std::vector<double> log)                        { continuous_logs_predicted_.insert(std::pair<std::string, std::vector<double> >("Vp", log))     ;}
  void                                   SetVsPredicted(std::vector<double> log)                        { continuous_logs_predicted_.insert(std::pair<std::string, std::vector<double> >("Vs", log))     ;}
  void                                   SetRhoPredicted(std::vector<double> log)                       { continuous_logs_predicted_.insert(std::pair<std::string, std::vector<double> >("Rho", log))    ;}

  void                                   SetVpSeismicResolution(std::vector<double> log)                { cont_logs_seismic_resolution_.insert(std::pair<std::string, std::vector<double> >("Vp", log))  ;}
  void                                   SetVsSeismicResolution(std::vector<double> log)                { cont_logs_seismic_resolution_.insert(std::pair<std::string, std::vector<double> >("Vs", log))  ;}
  void                                   SetRhoSeismicResolution(std::vector<double> log)               { cont_logs_seismic_resolution_.insert(std::pair<std::string, std::vector<double> >("Rho", log)) ;}

  // OTHER FUNCTIONS -----------------------------------

  void                                   FindOptimalWellLocation(std::vector<SeismicStorage *> & seismic_data,
                                                                 const Simbox                  * estimation_simbox,
                                                                 const Simbox                  & inversion_simbox,
                                                                 const NRLib::Matrix           & refl_coef,
                                                                 int                             n_angles,
                                                                 const std::vector<float>      & angle_weight,
                                                                 float                           max_shift,
                                                                 int                             i_max_offset,
                                                                 int                             j_max_offset,
                                                                 const std::vector<Surface *>  & limits,
                                                                 int                           & i_move,
                                                                 int                           & j_move,
                                                                 float                         & k_move) const;

  void                                   EstimateCor(fftw_complex * var1_c,
                                                     fftw_complex * var2_c,
                                                     fftw_complex * ccor_1_2_c,
                                                     int            cnzp) const;

  void                                   FillInCpp(const float * coeff,
                                                   int           start,
                                                   int           length,
                                                   fftw_real   * cpp_r,
                                                   int           nzp) const;

  void                                   SetLogFromVerticalTrend(const std::vector<double>                    & vertical_trend,
                                                                 std::map<std::string, std::vector<double> >  & cont_logs_seismic_resolution,
                                                                 std::vector<std::vector<double> >            & actual_synt_seismic_data,
                                                                 std::vector<std::vector<double> >            & well_synt_seismic_data,
                                                                 std::string                                    type,
                                                                 int                                            i_angle) const;

  void                                   FillInSeismic(std::vector<double> & seismic_data,
                                                       int                   start,
                                                       int                   length,
                                                       fftw_real           * seis_r,
                                                       int                   nzp,
                                                       bool                  top_value=false) const; //If true, get value at top of cell instead of center.

  void                                   SetSeismicGradient(double                            v0,
                                                            const NRLib::Grid2D<float>   &    structure_depth_grad_x,
                                                            const NRLib::Grid2D<float>   &    structure_depth_grad_y,
                                                            const NRLib::Grid2D<float>   &    ref_time_grad_x,
                                                            const NRLib::Grid2D<float>   &    ref_time_grad_y,
                                                            std::vector<double>          &    x_gradient,
                                                            std::vector<double>          &    y_gradient) const;

  void                                   SetTimeGradientSettings(float          distance,
                                                                 float          sigma_m);

  void                                   FindSeismicGradient(std::vector<SeismicStorage *>     & seismic_data,
                                                             const Simbox                      * const estimation_simbox,
                                                             int                                 n_angles,
                                                             std::vector<double>               & x_gradient,
                                                             std::vector<double>               & y_gradient,
                                                             std::vector<std::vector<double> > & sigma_gradient);

  void                                   FindContinuousPartOfData(const std::vector<bool> & hasData,
                                                                  int                       nz,
                                                                  int                     & start,
                                                                  int                     & length) const;

  int                                    FindMostProbable(const int * count,
                                                          int         n_facies,
                                                          int         block_index) const;

  void                                   WriteWell(const int                        formats,
                                                   const float                      max_hz_background,
                                                   const float                      max_hz_seismic,
                                                   const std::vector<std::string> & facies_name,
                                                   const std::vector<int>         & facies_label) const;

  void                                   GenerateSyntheticSeismic(const NRLib::Matrix    & reflection_matrix,
                                                                  std::vector<Wavelet *> & wavelet,
                                                                  int                     nz,
                                                                  int                     nzp,
                                                                  const Simbox          * timeSimbox,
                                                                  bool                    well_opt = false);

  void                                   SetLogFromGrid(FFTGrid   * grid,
                                                        int         i_angle,
                                                        int         n_angles,
                                                        std::string type);

  void                                   SetLogFromGrid(SegY         * segy,
                                                        const Simbox & simbox,
                                                        int            i_angle,
                                                        int            n_angles,
                                                        std::string    type);

  void                                   SetLogFromGrid(StormContGrid * storm,
                                                        int             i_angle,
                                                        int             n_angles,
                                                        std::string     type);

  void                                   SetSpatialFilteredLogs(std::vector<double>       & filtered_log,
                                                                int                         n_data,
                                                                std::string                 type,
                                                                const std::vector<double> & bg);

  void                                   FindMeanVsVp(const NRLib::Surface<double> & top,
                                                      const NRLib::Surface<double> & bot,
                                                      double                       & mean_vs_vp,
                                                      int                          & n_vs_vp) const;

  bool                                   VolumeFocus(const NRLib::Volume                            & volume,
                                                     std::vector<double>                            & x_pos_blocked,
                                                     std::vector<double>                            & y_pos_blocked,
                                                     std::vector<double>                            & z_pos_blocked,
                                                     std::vector<int>                               & facies_blocked,
                                                     std::vector<int>                               & s_pos,
                                                     std::vector<int>                               & i_pos,
                                                     std::vector<int>                               & j_pos,
                                                     std::vector<int>                               & k_pos,
                                                     std::map<std::string, std::vector<double> >    & continuous_logs_blocked,
                                                     std::map<std::string, std::vector<int> >       & discrete_logs_blocked,
                                                     std::map<std::string, std::vector<double> >    & cont_logs_seismic_resolution,
                                                     std::map<std::string, std::vector<double> >    & cont_logs_highcut_background,
                                                     std::map<std::string, std::vector<double> >    & cont_logs_highcut_seismic,
                                                     std::vector<std::vector<double> >              & actual_synt_seismic_data,
                                                     std::vector<std::vector<double> >              & well_synt_seismic_data) const; //Sets all observations outside volume to missing.
                                                                                                              //Returns false if none left - in that case, object is not modified.

private:

  // FUNCTIONS------------------------------------

  void                                   InterpolateTrend(const double                    * blocked_log,
                                                          double                          * trend) const;

  void                                   InterpolateTrend(const std::vector<double>       & blocked_log,
                                                          double                          * trend) const;

  void                                   InterpolateTrend(const std::vector<double>       & blocked_log,
                                                          std::vector<double>             & trend) const;

  void                                   InterpolateTrend(const std::vector<double>       & blocked_log,
                                                          std::vector<double>             & trend,
                                                          const std::vector<Surface *>    & limits) const;

  void                                   InterpolateTrend(const int                       * blocked_log,
                                                          std::vector<int>                & trend) const;

  double                                 ComputeElasticImpedance(double         vp,
                                                                 float         vs,
                                                                 float         rho,
                                                                 const float * coeff) const;

  void                                   RemoveMissingLogValues(const NRLib::Well                            * well_data,
                                                                std::vector<double>                          & x_pos_raw_logs,
                                                                std::vector<double>                          & y_pos_raw_logs,
                                                                std::vector<double>                          & z_pos_raw_logs,
                                                                std::vector<int>                             & facies_raw_logs,
                                                                std::map<std::string, std::vector<double> >  & continuous_logs_raw_logs,
                                                                std::map<std::string, std::vector<int> >     & discrete_logs_raw_logs,
                                                                const std::vector<std::string>               & cont_logs_to_be_blocked,
                                                                const std::vector<std::string>               & disc_logs_to_be_blocked,
                                                                unsigned int                                 & n_data,
                                                                bool                                         & failed,
                                                                std::string                                  & err_text) const;

  void                                   BlockWell(const Simbox                                        * estimation_simbox,
                                                   const NRLib::Well                                   * well,
                                                   const std::map<std::string, std::vector<double> >   & continuous_logs_raw_logs,
                                                   const std::map<std::string, std::vector<int> >      & discrete_logs_raw_logs,
                                                   const std::map<int, std::string>                    & facies_map,
                                                   std::map<std::string, std::vector<double> >         & continuous_logs_blocked,
                                                   std::map<std::string, std::vector<double> >         & cont_logs_highcut_seismic,
                                                   std::map<std::string, std::vector<double> >         & cont_logs_highcut_background,
                                                   std::map<std::string, std::vector<int> >            & discrete_logs_blocked,
                                                   std::vector<double>                                 & x_pos_blocked,
                                                   std::vector<double>                                 & y_pos_blocked,
                                                   std::vector<double>                                 & z_pos_blocked,
                                                   std::vector<int>                                    & facies_blocked,
                                                   unsigned int                                          n_data,
                                                   std::vector<int>                                    & i_pos,
                                                   std::vector<int>                                    & j_pos,
                                                   std::vector<int>                                    & k_pos,
                                                   int                                                 & first_M,
                                                   int                                                 & last_M,
                                                   int                                                 & first_B,
                                                   int                                                 & last_B,
                                                   unsigned int                                        & n_blocks,
                                                   std::map<std::string, int>                          & n_blocks_with_data,
                                                   int                                                 & n_blocks_with_data_tot,
                                                   bool                                                  facies_log_defined,
                                                   bool                                                  interpolate,
                                                   bool                                                  visible_only,
                                                   double                                              & dz,
                                                   bool                                                & failed,
                                                   bool                                                & is_insde,
                                                   std::string                                         & err_text) const;

  void                                   BlockWellForCorrelationEstimation(const MultiIntervalGrid                             * multiple_interval_grid,
                                                                           const NRLib::Well                                   * well,
                                                                           const std::map<std::string, std::vector<double> >   & continuous_logs_raw_logs,
                                                                           const std::map<std::string, std::vector<int> >      & discrete_raw_logs,
                                                                           const std::map<int, std::string>                    & facies_map,
                                                                           std::vector<double>                                 & x_pos_blocked,
                                                                           std::vector<double>                                 & y_pos_blocked,
                                                                           std::vector<double>                                 & z_pos_blocked,
                                                                           std::vector<int>                                    & facies_blocked,
                                                                           std::map<std::string, std::vector<double> >         & continuous_logs_blocked,
                                                                           std::map<std::string, std::vector<double> >         & cont_logs_highcut_seismic,
                                                                           std::map<std::string, std::vector<double> >         & cont_logs_highcut_background,
                                                                           std::map<std::string, std::vector<int> >            & discrete_logs_blocked,
                                                                           unsigned int                                          n_data,
                                                                           unsigned int                                        & n_blocks,
                                                                           std::map<std::string, int>                          & n_blocks_with_data,
                                                                           int                                                 & n_blocks_with_data_tot,
                                                                           std::vector<int>                                    & n_well_log_obs_in_interval,
                                                                           std::map<std::string, int>                          & n_layers_adjusted_per_interval,
                                                                           std::vector<int>                                    & i_pos,
                                                                           std::vector<int>                                    & j_pos,
                                                                           std::vector<int>                                    & k_pos,
                                                                           std::vector<int>                                    & s_pos,
                                                                           int                                                 & first_M,
                                                                           int                                                 & last_M,
                                                                           int                                                 & first_S,
                                                                           int                                                 & last_S,
                                                                           int                                                 & first_B,
                                                                           int                                                 & last_B,
                                                                           bool                                                  facies_log_defined,
                                                                           bool                                                  interpolate,
                                                                           int                                                 & n_layers,
                                                                           double                                              & dz,
                                                                           bool                                                & failed,
                                                                           std::string                                         & err_text) const;

  void    FindSizeAndBlockPointers(const MultiIntervalGrid       * multiple_interval_grid,
                                   std::vector<int>              & b_ind,
                                   int                             n_data,
                                   int                           & n_layers,
                                   int                           & first_M,
                                   int                           & last_M,
                                   int                           & first_S,
                                   int                           & last_S,
                                   unsigned int                  & n_blocks,
                                   std::map<std::string, int>    & n_layers_adjusted_per_interval,
                                   std::string                   & err_text) const;

  void    FindSizeAndBlockPointers(const Simbox                  * const estimation_simbox,
                                   bool                            visible_only,
                                   std::vector<int>              & b_ind,
                                   const int                     & n_layers,
                                   int                           & first_M,
                                   int                           & last_M,
                                   unsigned int                  & n_blocks,
                                   bool                          & is_inside) const;

  void    FindBlockIJK(const Simbox                     * estimation_simbox,
                       const std::vector<int>           & bInd,
                       const int                        & first_M,
                       const int                        & last_M,
                       int                              & first_B,
                       int                              & last_B,
                       std::vector<int>                 & i_pos,
                       std::vector<int>                 & j_pos,
                       std::vector<int>                 & k_pos,
                       double                           & dz) const;

  void    FindBlockIJK(const MultiIntervalGrid          * multiple_interval_grid,
                       const std::vector<int>           & bInd,
                       const std::vector<double>        & x_pos_raw_logs,
                       const std::vector<double>        & y_pos_raw_logs,
                       const std::vector<double>        & z_pos_raw_logs,
                       const int                        & first_M,
                       const int                        & last_M,
                       const int                        & first_S,
                       const int                        & last_S,
                       int                              & first_B,
                       int                              & last_B,
                       std::vector<int>                 & n_well_log_obs_in_interval,
                       std::vector<int>                 & i_pos,
                       std::vector<int>                 & j_pos,
                       std::vector<int>                 & k_pos,
                       std::vector<int>                 & s_pos,
                       double                           & dz) const;

  void    FindBlockIJK(const StormContGrid             & stormgrid,
                       const std::vector<int>          & b_ind,
                       const int                       & first_M,
                       const int                       & last_M,
                       int                             & first_B,
                       int                             & last_B,
                       std::vector<int>                & i_pos,
                       std::vector<int>                & j_pos,
                       std::vector<int>                & k_pos) const;

  void    BlockCoordinateLog(const std::vector<int>     &  b_ind,
                             const std::vector<double>  &  coord,
                             std::vector<double>        &  blocked_coord) const;

  void    BlockContinuousLog(const std::vector<int>     &  b_ind,
                             const std::vector<double>  &  well_log,
                             std::vector<double>        &  blocked_log) const;

  void    BlockPorosityLog(const std::vector<int>     &  b_ind,
                           const std::vector<double>  &  well_log,
                           std::vector<double>        &  blocked_log) const;

  void    BlockFaciesLog(const std::vector<int>             & b_ind,
                           const std::vector<int>           & well_log,
                           const std::map<int,std::string>  & facies_map,
                           int                                n_facies,
                           std::vector<int>                 & blocked_log) const;

  void    InterpolateContinuousLog(std::vector<double>    & blocked_log,
                                   int                      start,
                                   int                      end,
                                   int                      index,
                                   float                    rel) const;

  int   FindMostProbable(const std::vector<int>           & count,
                         int                                n_facies,
                         int                                block_index) const;

  void    SmoothTrace(std::vector<float>                  & trace) const;

  void    FindPeakTrace(std::vector<float>                & trace,
                        std::vector<double>               & z_peak,
                        std::vector<double>               & peak,
                        std::vector<double>               & b,
                        double                              dz,
                        double                              z_top) const;

  void    PeakMatch(std::vector<double>                   & zPeak,
                    std::vector<double>                   & peak,
                    std::vector<double>                   & b,
                    std::vector<double>                   & zPeakW,
                    std::vector<double>                   & peakW,
                    std::vector<double>                   & bW) const;

  double  ComputeShift(std::vector<double>                & z_peak,
                       std::vector<double>                & z_peak_w,
                       double                               z0) const;

  void    ComputeGradient(std::vector<double>             & q_epsilon,
                          std::vector<double>             & q_epsilon_data,
                          std::vector<double>             & z_shift,
                          int                               nx,
                          int                               ny,
                          double                            dx,
                          double                            dy) const;

  void    SmoothGradient(std::vector<double>               & x_gradient,
                         std::vector<double>               & y_gradient,
                         std::vector<double>               & q_epsilon,
                         std::vector<double>               & q_epsilon_data,
                         std::vector<std::vector<double> > & sigma_gradient) const;

  void    ComputePrecisionMatrix(double                     & a,
                                 double                     & b,
                                 double                     & c) const;


  float   ComputeElasticImpedance(double                      alpha,
                                  double                      beta,
                                  double                      rho,
                                  const float               * coeff) const;

  void    CountBlocksWithData(const std::vector<double>                          & x_pos_blocked,
                              const std::vector<double>                          & y_pos_blocked,
                              const std::vector<double>                          & z_pos_blocked,
                              const std::map<std::string, std::vector<double> >  & continuous_logs_blocked,
                              std::string                                          interval_name,
                              unsigned int                                         n_blocks,
                              std::map<std::string, int>                         & n_blocks_with_data,
                              int                                                & n_blocks_with_data_tot) const;

  void    CountBlocksWithDataPerInterval(const MultiIntervalGrid                            * multiple_interval_grid,
                                         const std::vector<double>                          & x_pos_blocked,
                                         const std::vector<double>                          & y_pos_blocked,
                                         const std::vector<double>                          & z_pos_blocked,
                                         const std::map<std::string, std::vector<double> >  & continuous_logs_blocked,
                                         unsigned int                                         n_blocks,
                                         std::map<std::string, int>                         & n_blocks_with_data,
                                         int                                                & n_blocks_with_data_tot) const;

  void    WriteRMSWell(const float                                max_hz_background,
                       const float                                max_hz_seismic,
                       const std::vector<std::string>           & facies_name,
                       const std::vector<int>                   & facies_label) const;

  void    WriteNorsarWell(const float                             max_hz_background,
                          const float                             max_hz_seismic) const;

  void    FindXYZForVirtualPart(const Simbox              * simbox,
                                const std::vector<int>    & i_pos,
                                const std::vector<int>    & j_pos,
                                const std::vector<int>    & k_pos,
                                const int                 & n_blocks,
                                const int                 & first_B,
                                const int                 & last_B,
                                std::vector<double>       & x_pos_blocked,
                                std::vector<double>       & y_pos_blocked,
                                std::vector<double>       & z_pos_blocked) const;

  void    FindXYZForVirtualPart(const MultiIntervalGrid   * multiple_interval_grid,
                                const std::vector<int>    & i_pos,
                                const std::vector<int>    & j_pos,
                                const std::vector<int>    & k_pos,
                                int                         n_blocks,
                                int                         first_B,
                                int                         last_B,
                                int                         first_S,
                                int                         last_S,
                                std::vector<double>       & x_pos_blocked,
                                std::vector<double>       & y_pos_blocked,
                                std::vector<double>       & z_pos_blocked) const;

  void    UpdateLog(std::vector<double>                           & data,
                    const std::vector<std::pair<size_t, size_t> > & intervals) const;

  void    UpdateLog(std::vector<int>                              & data,
                    const std::vector<std::pair<size_t, size_t> > & intervals) const;

  void    FindTrendPositions(const std::vector<int> & i_pos,
                             const std::vector<int> & j_pos,
                             const std::vector<int> & k_pos,
                             const int              & n_blocks,
                             const CravaTrend       & trend_cubes,
                             std::vector<double>    & s1,
                             std::vector<double>    & s2);

  void    AssignToFacies(const std::vector<double>         & well_log,
                         const std::vector<int>            & facies_log,
                         const std::vector<int>            & facies_numbers,
                         std::vector<std::vector<double> > & blocked_log) const;

  void    CalculateBulkShear(const int                         & n_blocks,
                             const int                         & n_facies,
                             std::vector<std::vector<double> > & bulk_modulus,
                             std::vector<std::vector<double> > & shear_modulus);


  // CLASS VARIABLES -----------------------------

  std::map<std::string, int>                  n_layers_adjusted_per_interval_;   // Number of layers per interval using dz from the first interval everywhere
  std::map<std::string, int>                  n_blocks_with_data_;               // Number of blocks with data per interval
  int                                         n_blocks_with_data_tot_;           // Total number of blocks with data
  unsigned int                                n_blocks_;                         // Number of blocks
  unsigned int                                n_data_;                           // Number of non-missing
  std::string                                 well_name_;

  std::map<int, std::string>                  facies_map_;
  bool                                        facies_log_defined_;

  // Blocked values
  std::vector<double>                         x_pos_blocked_;  // Blocked x position
  std::vector<double>                         y_pos_blocked_;  // Blocked y position
  std::vector<double>                         z_pos_blocked_;  // Blocked z position
  std::vector<int>                            facies_blocked_; // Blocked facies log

  std::vector<int>                            s_pos_;    // Simbox number for block
  std::vector<int>                            i_pos_;    // Simbox i position for block
  std::vector<int>                            j_pos_;    // Simbox j position for block
  std::vector<int>                            k_pos_;    // Simbox k position for block

  int                                         n_continuous_logs_;     // Number of continuous logs
  int                                         n_discrete_logs_;       // Number of discrete logs

  std::map<std::string, std::vector<double> > continuous_logs_blocked_;         // Map between variable name and blocked continuous log
  std::map<std::string, std::vector<int> >    discrete_logs_blocked_;           // Map between variable name and blocked discrete log

  std::map<std::string, std::vector<double> > cont_logs_seismic_resolution_;    // Continuous logs filtered to resolution of inversion result
  std::map<std::string, std::vector<double> > cont_logs_highcut_background_;    // Continuous logs high-cut filtered to background resolution (log-domain)
  std::map<std::string, std::vector<double> > cont_logs_highcut_seismic_;       // Continuous logs high-cut filtered to approx. seismic resolution (log-domain)

  std::vector<std::vector<double> >           actual_synt_seismic_data_;        ///< Forward modelled seismic data using local wavelet
  std::vector<std::vector<double> >           well_synt_seismic_data_;          ///< Forward modelled seismic data using wavelet estimated in well

  float                                       lateral_threshold_gradient_; //Minimum lateral distance where gradient lines must not cross
  float                                       sigma_m_; //Smoothing factor for the gradients

  // Logs from well, not blocked
  std::vector<double>                         x_pos_raw_logs_;
  std::vector<double>                         y_pos_raw_logs_;
  std::vector<double>                         z_pos_raw_logs_;
  std::vector<int>                            facies_raw_logs_;

  std::map<std::string, std::vector<double> > continuous_raw_logs_;  // Map between variable name and raw continuous logs
  std::map<std::string, std::vector<int> >    discrete_raw_logs_;    // Map between variable name and raw discrete logs

  //Variables needed in SetLogFromGrid and later used in WriteWell
  std::map<std::string, std::vector<double> > continuous_logs_predicted_;  ///< Map between variable name and predicted continuous log
  std::map<int, std::vector<double> >         real_seismic_data_;          ///< Map between angle and real seismic data
  std::map<int, std::vector<double> >         facies_prob_;                ///< Map between facies and facies prob

  std::vector<double>                         vp_facies_filtered_;         //Vp filtered from spatialwellfilter
  std::vector<double>                         rho_facies_filtered_;

  std::vector<int>                            interval_log_; //Log with interval number for each block, used with writing of filtered and seismic resolution logs (they are resampled for intervals, not created to output_blocked_logs)

  int                                         n_angles_;                  ///< Number of AVA stacks

  bool                                        interpolate_;              ///<

  int                                         n_layers_;                 ///< Number of layers in estimation_simbox
  double                                      dz_;                       ///< Simbox dz value for block

  int                                         first_S_;                   ///< First simbox that the well log appears in
  int                                         last_S_;                    ///< Last simbox that the well log appears in

  int                                         first_M_;                   ///< First well log entry contributing to blocked well
  int                                         last_M_;                    ///< Last well log entry contributing to blocked well

  int                                         first_B_;                   ///< First block with contribution from well log
  int                                         last_B_;                    ///< Last block with contribution from well log

  std::vector<int>                            n_well_log_obs_in_interval_;      ///< Number of well log observations in each interval

  bool                                        is_deviated_;

  //Used in logging in crava.cpp. Copied from well
  int                                         real_vs_log_;                    //Uses the indicator enum from Modelsettings
  int                                         use_for_facies_probabilities_;   //Uses the indicator enum from Modelsettings

  int                                         use_for_background_trend_;       //Uses the indicator enum from Modelsettings
  int                                         use_for_filtering_;              //Uses the indicator enum from Modelsettings
  int                                         use_for_wavelet_estimation_;     //Uses the indicator enum from Modelsettings
  int                                         use_for_rock_physics_;           //Uses the indicator enum from Modelsettings

  //Variables used for rockphysics
  std::vector<std::string>                    facies_names_;
  std::vector<std::vector<double> >           vp_for_facies_;             ///< Vector facies, vector n_blocks
  std::vector<std::vector<double> >           vs_for_facies_;             ///< Raw logs (log-domain)
  std::vector<std::vector<double> >           rho_for_facies_;            ///<
  std::vector<std::vector<double> >           porosity_for_facies_;       ///<
  std::vector<std::vector<double> >           bulk_modulus_;             ///<
  std::vector<std::vector<double> >           shear_modulus_;            ///< Logs calculated from vp_, vs_ and rho_
  std::vector<double>                         s1_;                       ///< Trend positions corresponding to the first trend cube, same for all facies
  std::vector<double>                         s2_;                       ///< Trend positions corresponding to the second trend cube, same for all facies

};
#endif
