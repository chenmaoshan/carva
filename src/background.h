#ifndef BACKGROUND_H
#define BACKGROUND_H

#include <stdio.h>

class Vario;
class Simbox;
class FFTGrid;
class CovGrid2D;
class WellData;
class GridMapping;
class KrigingData3D;
class KrigingData2D;
class ModelSettings;

//Special note on the use of Background:
//All pointers used here are also used externally, so no deletion happens. 

class Background
{
public:
  Background(FFTGrid       ** grids,
             WellData      ** wells,
             FFTGrid       *& velocity,
             Simbox         * timeSimbox,
             Simbox         * timeBGSimbox,
             ModelSettings  * modelSettings);
  Background(FFTGrid ** grids);
  ~Background(void);

  FFTGrid    * getAlpha(void) { return backModel_[0]; }
  FFTGrid    * getBeta(void)  { return backModel_[1]; }
  FFTGrid    * getRho(void)   { return backModel_[2]; }
  double       getMeanVsVp()  { return vsvp_;}
  
  void         setClassicVsVp(); //For debugging purposes.

  void         writeBackgrounds(Simbox                  * simbox, 
                                GridMapping             * depthMapping, 
                                GridMapping             * timeMapping,
                                const bool                isFile, 
                                const TraceHeaderFormat & thf = TraceHeaderFormat(TraceHeaderFormat::SEISWORKS)) const;

  void         releaseGrids(); //backModel grids are now taken care of by other classes.
  
private:
  void         generateBackgroundModel(FFTGrid      *& bgAlpha,
                                       FFTGrid      *& bgBeta,
                                       FFTGrid      *& bgRho,
                                       FFTGrid      *& velocity,
                                       WellData     ** wells,
                                       Simbox        * simbox,
                                       ModelSettings * modelSettings);
  void         resampleBackgroundModel(FFTGrid      *& bgAlpha,
                                       FFTGrid      *& bgBeta,
                                       FFTGrid      *& bgRho,
                                       Simbox        * timeBGsimbox,
                                       Simbox        * timeSimbox,
                                       ModelSettings * modelSettings);
  void         padAndSetBackgroundModel(FFTGrid * bgAlpha,
                                        FFTGrid * bgBeta,
                                        FFTGrid * bgRho);
  void         createPaddedParameter(FFTGrid *& pNew,
                                     FFTGrid  * pOld);
  void         calculateBackgroundTrend(float             * trend,
                                        float             * avgDev,
                                        WellData         ** wells,
                                        Simbox            * simbox,
                                        float               logMin, 
                                        float               logMax,
                                        float               maxHz, 
                                        bool                write1D,
                                        bool                write3D,
                                        int                 nWells, 
                                        bool                hasVelocityTrend,
                                        const std::string & name,
                                        bool                isFile);
  const CovGrid2D & makeCovGrid2D(Simbox * simbox,
                                  Vario  * vario, 
                                  int      debugFlag);
  void         setupKrigingData2D(std::vector<KrigingData2D> & krigingDataAlpha,
                                  std::vector<KrigingData2D> & krigingDataBeta,
                                  std::vector<KrigingData2D> & krigingDataRho,
                                  float                      * trendAlpha,
                                  float                      * trendBeta, 
                                  float                      * trendRho ,
                                  int                          debugFlag,
                                  Simbox                     * simbox,
                                  WellData                  ** wells,
                                  const int                    nWells);
  void         makeKrigedBackground(const std::vector<KrigingData2D> & krigingData,
                                    FFTGrid                         *& bgGrid,
                                    float                            * trend,
                                    Simbox                           * simbox,
                                    const CovGrid2D                  & covGrid2D,
                                    const std::string                & type,
                                    bool                               isFile);
  void         calculateVelocityDeviations(FFTGrid   * velocity,
                                           WellData ** wells,
                                           Simbox    * simbox,                               
                                           float    *& trendVel,
                                           float    *& avgDevVel,
                                           float     * avgDevAlpha,
                                           int         outputFlag,
                                           int         nWells);
  void         resampleParameter(FFTGrid *& parameterNew,
                                 FFTGrid  * parameterOld,
                                 Simbox   * simboxNew,
                                 Simbox   * simboxOld,
                                 bool       isFile);
  void         calculateVerticalTrend(WellData         ** wells,
                                      float             * trend, 
                                      float               logMin,
                                      float               logMax,
                                      float               maxHz,
                                      int                 nWells,
                                      int                 nz,
                                      float               dz,
                                      const std::string & name);
  void         writeVerticalTrend(float      * trend, 
                                  float        dz,
                                  int          nz,
                                  std::string  name);
  void         calculateDeviationFromVerticalTrend(WellData    ** wells,
                                                   const float  * trend, 
                                                   float        * avg_dev,
                                                   int            nWells,
                                                   int            nd,
                                                   std::string    name);
  void         writeDeviationsFromVerticalTrend(const float *  avg_dev_alpha,
                                                const float *  avg_dev_beta,
                                                const float *  avg_dev_rho,
                                                const float *  trend_alpha, 
                                                const float *  trend_beta, 
                                                const float *  trend_rho,
                                                WellData    ** wells,
                                                const int      nWells,
                                                const int      nz);
  void         smoothTrendWithLocalLinearRegression(float      * trend, 
                                                    int        * count,
                                                    int          nWells,
                                                    int          nz,
                                                    float        dz,
                                                    float        min_value, 
                                                    float        max_value,
                                                    std::string  parName);
  void         fillInVerticalTrend(FFTGrid     * bgTrend, 
                                   const float * trend);
  void         findMeanVsVp(FFTGrid * Vp,
                            FFTGrid * Vs);
  FFTGrid    * copyFFTGrid(FFTGrid   * origGrid, 
                           const bool  expTrans, 
                           const bool  fileGrid) const;

  FFTGrid    * backModel_[3];       // Cubes for background model files.
  int          DataTarget_;         // Number of data requested in a kriging block 
  double       vsvp_;               // Average ratio between vs and vp.
};

#endif
