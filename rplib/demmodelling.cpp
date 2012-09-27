#include "rplib/demmodelling.h"

#include "rplib/dem.h"
#include "rplib/mineral.h"
#include "rplib/brine.h"
#include "rplib/co2.h"
#include "rplib/solidmixed.h"
#include "rplib/fluidmixed.h"
#include "rplib/rockinclusion.h"
#include "rplib/distributionsmineral.h"
#include "rplib/distributionsbrine.h"
#include "rplib/distributionsco2.h"
#include "rplib/distributionsrockinclusion.h"
#include "rplib/distributionssolidmix.h"
#include "rplib/distributionsfluidmix.h"
#include "rplib/distributionsbrineevolution.h"
#include "rplib/distributionsco2evolution.h"
#include "rplib/distributionsfluidmixevolution.h"
#include "rplib/distributionsrockinclusionevolution.h"
#include "rplib/distributionwithtrend.h"
#include "rplib/deltadistributionwithtrend.h"

#include "nrlib/exception/exception.hpp"
#include "nrlib/random/delta.hpp"
#include "nrlib/random/normal.hpp"
#include "nrlib/trend/trend.hpp"

#include <cmath>

double
DEMTools::CalcBulkModulusOfBrineFromTPS(double temperature,
                                        double pressure,
                                        double salinity) {


  double vb   = CalcVelocityOfBrineFromTPS(temperature,
                                           pressure,
                                           salinity);

  double rhob = CalcDensityOfBrineFromTPS(temperature,
                                          pressure,
                                          salinity);

  return rhob*(vb*vb)/1E3;
}

double
DEMTools::CalcDensityOfBrineFromTPS(double temperature,
                                    double pressure,
                                    double salinity) {

  return CalcDensityOfWaterFromTP(temperature,pressure) +
        salinity*(0.668 + 0.44*salinity + 1E-6*(300*pressure - 2400*pressure*salinity +
        temperature*(80 + 3*temperature - 3300*salinity - 13*pressure + 47*pressure*salinity)));

}

void
DEMTools::CalcCo2Prop(double& bulk_modulus,
                      double& density,
                      double ti,
                      double pi) {

  double ttmp[] = {1.7000000e+001,  2.7000000e+001,  3.7000000e+001,  4.7000000e+001,  5.7000000e+001,  6.7000000e+001,  7.7000000e+001,  8.7000000e+001,  9.7000000e+001,  1.0700000e+002,  1.1700000e+002,  1.2700000e+002};
  size_t nt     = 12;

  double pmpatmp[] = {1.0005000e-001,  1.0005000e+000,  4.0020000e+000,  7.0035000e+000,  1.0005000e+001,  1.4007000e+001,  2.0010000e+001,  2.5012500e+001,  3.0015000e+001,  4.0020000e+001};
  size_t np        = 10;

  std::vector<double> t(ttmp, ttmp + nt);
  std::vector<double> pmpa(pmpatmp, pmpatmp + np);

  static std::vector< std::vector<double> > co2_bulk(np, std::vector<double>(nt, 0.0));
  /*static std::vector< std::vector<double> > co2_velocity(np, std::vector<double>(nt, 0.0));*/ //Velocity interpolation commented out
  static std::vector< std::vector<double> > co2_density(np, std::vector<double>(nt, 0.0));

  //{ // local scope co2 velocity
  //  double tmp0[] = {2.6400000e+002,  2.6800000e+002,  2.7200000e+002,  2.7600000e+002,  2.8000000e+002,  2.8400000e+002,  2.8800000e+002,  2.9100000e+002,  2.9500000e+002,  2.9900000e+002,  3.0200000e+002,  3.0600000e+002};
  //  co2_velocity[0] = std::vector<double>(tmp0, tmp0 + nt);

  //  double tmp1[] = {2.5600000e+002,  2.6100000e+002,  2.6600000e+002,  2.7000000e+002,  2.7500000e+002,  2.7900000e+002,  2.8400000e+002,  2.8800000e+002,  2.9200000e+002,  2.9600000e+002,  3.0000000e+002,  3.0400000e+002};
  //  co2_velocity[1] = std::vector<double>(tmp1, tmp1 + nt);

  //  double tmp2[] = {2.2000000e+002,  2.3100000e+002,  2.4100000e+002,  2.5100000e+002,  2.5900000e+002,  2.6400000e+002,  2.7100000e+002,  2.7600000e+002,  2.8100000e+002,  2.8700000e+002,  2.9200000e+002,  2.9600000e+002};
  //  co2_velocity[2] = std::vector<double>(tmp2, tmp2 + nt);

  //  double tmp3[] = {3.7900000e+002,  1.8500000e+002,  2.0900000e+002,  2.1900000e+002,  2.3700000e+002,  2.5200000e+002,  2.6300000e+002,  2.7000000e+002,  2.7600000e+002,  2.8200000e+002,  2.8500000e+002,  2.9000000e+002};
  //  co2_velocity[3] = std::vector<double>(tmp3, tmp3 + nt);

  //  double tmp4[] = {4.8300000e+002,  4.1100000e+002,  1.4200000e+002,  1.8300000e+002,  2.1200000e+002,  2.3800000e+002,  2.4900000e+002,  2.5700000e+002,  2.6500000e+002,  2.7200000e+002, 2.7800000e+002,  2.8600000e+002};
  //  co2_velocity[4] = std::vector<double>(tmp4, tmp4 + nt);

  //  double tmp5[] = {5.5400000e+002,  4.9600000e+002,  3.6600000e+002,  2.6900000e+002,  2.4700000e+002,  2.4800000e+002,  2.5500000e+002,  2.6000000e+002,  2.6900000e+002,  2.7800000e+002,  2.8500000e+002,  2.9100000e+002};
  //  co2_velocity[5] = std::vector<double>(tmp5, tmp5 + nt);

  //  double tmp6[] = {6.3700000e+002,  5.8600000e+002, 5.0200000e+002,  4.4100000e+002,  3.8200000e+002,  3.5800000e+002,  3.5300000e+002,  3.4500000e+002,  3.3700000e+002,  3.2800000e+002,  3.2000000e+002,  3.1400000e+002};
  //  co2_velocity[6] = std::vector<double>(tmp6, tmp6 + nt);

  //  double tmp7[] = { 6.8800000e+002,  6.4000000e+002,  5.9100000e+002,  5.3800000e+002,  4.8800000e+002,  4.6000000e+002,  4.3100000e+002,  4.1300000e+002,  3.9700000e+002,  3.8200000e+002,  3.6600000e+002,  3.5200000e+002};
  //  co2_velocity[7] = std::vector<double>(tmp7, tmp7 + nt);

  //  double tmp8[] = {7.3700000e+002,  6.8700000e+002,  6.4100000e+002,  5.9600000e+002,  5.5200000e+002,  5.2100000e+002,  4.9400000e+002,  4.7000000e+002,  4.5000000e+002,  4.2900000e+002,  4.1200000e+002,  3.9700000e+002};
  //  co2_velocity[8] = std::vector<double>(tmp8, tmp8 + nt);

  //  double tmp9[] = { 8.1400000e+002,  7.6600000e+002,  7.1400000e+002,  6.7600000e+002,  6.3900000e+002,  6.1500000e+002,  5.9200000e+002,  5.6400000e+002,  5.4500000e+002,  5.2600000e+002,  5.0400000e+002,  4.8700000e+002};
  //  co2_velocity[9] = std::vector<double>(tmp9, tmp9 + nt);

  //} // local scope end co2 velocity

  { // local scope co2 density
    double tmp0[] = {1.8600000e-003,  1.8000000e-003,  1.7400000e-003,  1.6800000e-003,  1.6300000e-003,  1.5800000e-003,  1.5400000e-003,  1.4900000e-003,  1.4500000e-003,  1.4100000e-003,  1.3800000e-003,  1.3400000e-003};
    co2_density[0] = std::vector<double>(tmp0, tmp0 + nt);

    double tmp1[] = {1.9630000e-002,  1.8840000e-002,  1.8130000e-002,  1.7480000e-002,  1.6880000e-002,  1.6320000e-002,  1.5800000e-002,  1.5310000e-002,  1.4860000e-002,  1.4440000e-002,  1.4040000e-002,  1.3660000e-002};
    co2_density[1] = std::vector<double>(tmp1, tmp1 + nt);

    double tmp2[] = {1.2900000e-001,  9.3950000e-002,  8.7090000e-002,  8.1690000e-002,  7.7240000e-002,  7.3450000e-002,  7.0130000e-002,  6.7190000e-002,  6.4540000e-002,  6.2150000e-002,  6.0000000e-002,  5.7970000e-002};
    co2_density[2] = std::vector<double>(tmp2, tmp2 + nt);

    double tmp3[] = {8.3000000e-001,  6.8000000e-001,  2.0072000e-001,  1.8283000e-001,  1.6362000e-001,  1.4960000e-001,  1.3928000e-001,  1.3094000e-001,  1.2391000e-001,  1.1786000e-001,  1.1275000e-001,  1.0794000e-001};
    co2_density[3] = std::vector<double>(tmp3, tmp3 + nt);

    double tmp4[] = {9.1700000e-001,  8.0500000e-001,  6.8300000e-001,  4.4973000e-001,  3.2761000e-001,  2.6715000e-001,  2.3550000e-001,  2.1381000e-001,  1.9732000e-001,  1.8422000e-001,  1.7409000e-001,  1.6443000e-001};
    co2_density[4] = std::vector<double>(tmp4, tmp4 + nt);

    double tmp5[] = {9.3000000e-001,  8.6000000e-001,  7.8000000e-001,  6.9000000e-001,  5.7000000e-001,  4.8000000e-001,  3.9000000e-001,  3.3500000e-001,  3.0000000e-001,  2.7500000e-001,  2.5200000e-001,  2.3500000e-001};
    co2_density[5] = std::vector<double>(tmp5, tmp5 + nt);

    double tmp6[] = { 9.6000000e-001,  9.1000000e-001,  8.6000000e-001,  8.0000000e-001,  7.4000000e-001,  6.8000000e-001,  6.2000000e-001,  5.6000000e-001,  5.1000000e-001,  4.7000000e-001,  4.2000000e-001,  3.9000000e-001};
    co2_density[6] = std::vector<double>(tmp6, tmp6 + nt);

    double tmp7[] = {9.9000000e-001,  9.5000000e-001,  9.0000000e-001,  8.5000000e-001,  7.9000000e-001,  7.5000000e-001,  7.0000000e-001,  6.4000000e-001,  5.9000000e-001,  5.5000000e-001,  5.1000000e-001,  4.8000000e-001};
    co2_density[7] = std::vector<double>(tmp7, tmp7 + nt);

    double tmp8[] = {1.0080000e+000,  9.7000000e-001,  9.3000000e-001,  8.9000000e-001,  8.5000000e-001,  8.1000000e-001,  7.7000000e-001,  7.2000000e-001,  6.8000000e-001,  6.3000000e-001,  6.0000000e-001,  5.7000000e-001};
    co2_density[8] = std::vector<double>(tmp8, tmp8 + nt);

    double tmp9[] = {1.0400000e+000,  1.0000000e+000,  9.7000000e-001,  9.4000000e-001,  9.0000000e-001,  8.7000000e-001,  8.4000000e-001,  8.0000000e-001,  7.7000000e-001,  7.3000000e-001,  7.0000000e-001,  6.7000000e-001};
    co2_density[9] = std::vector<double>(tmp9, tmp9 + nt);

  } // local scope end co2 density

  { // local scope for co2 bulk
    double tmp0[] = {1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004,  1.3000000e-004 };
    co2_bulk[0] = std::vector<double>(tmp0, tmp0 + nt);

    double tmp1[] = {1.2900000e-003,  1.2800000e-003,  1.2800000e-003,  1.2800000e-003,  1.2800000e-003,  1.2700000e-003,  1.2700000e-003,  1.2700000e-003,  1.2600000e-003,  1.2600000e-003,  1.2600000e-003,  1.2600000e-003};
    co2_bulk[1] = std::vector<double>(tmp1, tmp1 + nt);

    double tmp2[] = {6.2400000e-003,  5.0100000e-003,  5.0400000e-003,  5.1300000e-003,  5.1800000e-003,  5.1400000e-003,  5.1700000e-003,  5.1000000e-003,  5.1000000e-003,  5.1100000e-003,  5.1100000e-003,  5.0700000e-003};
    co2_bulk[2] = std::vector<double>(tmp2, tmp2 + nt);

    double tmp3[] = {1.1922000e-001,  2.3270000e-002,  8.7300000e-003,  8.7900000e-003,  9.2300000e-003,  9.5100000e-003,  9.6500000e-003,  9.5500000e-003,  9.4100000e-003,  9.3700000e-003,  9.1600000e-003,  9.1000000e-003};
    co2_bulk[3] = std::vector<double>(tmp3, tmp3 + nt);

    double tmp4[] = {2.1393000e-001,  1.3598000e-001,  1.5990000e-002,  1.2820000e-002,  1.4690000e-002,  1.5200000e-002,  1.4600000e-002,  1.4120000e-002,  1.3860000e-002,  1.3630000e-002,  1.3460000e-002,  1.3410000e-002};
    co2_bulk[4] = std::vector<double>(tmp4, tmp4 + nt);

    double tmp5[] = {2.8543000e-001,  2.1157000e-001,  1.0449000e-001,  4.9930000e-002,  3.4780000e-002,  2.9520000e-002,  2.5360000e-002,  2.2650000e-002,  2.1710000e-002,  2.1250000e-002,  2.0470000e-002,  1.9900000e-002};
    co2_bulk[5] = std::vector<double>(tmp5, tmp5 + nt);

    double tmp6[] = {3.8954000e-001,  3.1249000e-001,  2.1672000e-001,  1.5558000e-001,  1.0798000e-001,  8.7150000e-002,  7.7260000e-002,  6.6650000e-002,  5.7920000e-002,  5.0560000e-002,  4.3010000e-002,  3.8450000e-002};
    co2_bulk[6] = std::vector<double>(tmp6, tmp6 + nt);

    double tmp7[] = {4.6861000e-001,  3.8912000e-001,  3.1435000e-001,  2.4603000e-001,  1.8813000e-001,  1.5870000e-001,  1.3003000e-001,  1.0916000e-001,  9.2990000e-002,  8.0260000e-002,  6.8320000e-002,  5.9470000e-002};
    co2_bulk[7] = std::vector<double>(tmp7, tmp7 + nt);

    double tmp8[] = {5.4751000e-001,  4.5781000e-001,  3.8212000e-001,  3.1614000e-001,  2.5900000e-001,  2.1987000e-001,  1.8791000e-001,  1.5905000e-001,  1.3770000e-001,  1.1595000e-001,  1.0185000e-001,  8.9840000e-002};
    co2_bulk[8] = std::vector<double>(tmp8, tmp8 + nt);

    double tmp9[] = {6.8910000e-001,  5.8676000e-001,  4.9450000e-001,  4.2956000e-001,  3.6749000e-001,  3.2906000e-001,  2.9439000e-001,  2.5448000e-001,  2.2871000e-001,  2.0197000e-001,  1.7781000e-001,  1.5890000e-001};
    co2_bulk[9] = std::vector<double>(tmp9, tmp9 + nt);

  } // local scope end


  if (ti < t.front() || ti > t.back() || pi < pmpa.front() || pi > pmpa.back())
    throw NRLib::Exception("Temperature or pressure outside valid range.");
  size_t i1 = 0;
  while (i1 < t.size() && ti >= t[i1])
    i1++;

  if (i1 >= 1)
    i1--;

  if (i1 > t.size() - 2)
    i1 = t.size() - 2;

  double t1 = (ti - t[i1])/(t[i1+1] - t[i1]);

  size_t j1 = 0;
  while (j1 < pmpa.size() && pi >= pmpa[j1])
    j1++;

  if (j1 >= 1)
    j1--;

  if (j1 > pmpa.size() - 2)
    j1 = pmpa.size() - 2;

  double t2 = (pi - pmpa[j1])/(pmpa[j1+1] - pmpa[j1]);

  double val_s1_1 = t1*co2_bulk[j1][i1+1] + (1.0 - t1)*co2_bulk[j1][i1];
  double val_s1_2 = t1*co2_bulk[j1+1][i1+1] + (1.0 - t1)*co2_bulk[j1+1][i1];
  bulk_modulus    = t2*val_s1_2 + (1-t2)*val_s1_1;

  val_s1_1 = t1*co2_density[j1][i1+1] + (1.0 - t1)*co2_density[j1][i1];
  val_s1_2 = t1*co2_density[j1+1][i1+1] + (1.0 - t1)*co2_density[j1+1][i1];
  density  = t2*val_s1_2 + (1-t2)*val_s1_1;

  /*val_s1_1 = t1*co2_velocity[j1][i1+1] + (1.0 - t1)*co2_velocity[j1][i1];
  val_s1_2 = t1*co2_velocity[j1+1][i1+1] + (1.0 - t1)*co2_velocity[j1+1][i1];
  velocity = t2*val_s1_2 + (1-t2)*val_s1_1;*/

}


double
DEMTools::CalcEffectiveElasticModuliUsingHill(const std::vector<double> & prop,
                                              const std::vector<double> & volumefraction) {
  return 0.5*(CalcEffectiveElasticModuliUsingReuss(prop, volumefraction) +
              CalcEffectiveElasticModuliUsingVoigt(prop, volumefraction));
}

double
DEMTools::CalcEffectiveElasticModuliUsingReuss(const std::vector<double> & prop,
                                               const std::vector<double> & volumefraction) {

  assert (prop.size() == volumefraction.size());
  assert (std::accumulate(volumefraction.begin(), volumefraction.end(), 0.0) == 1.0);

  bool found_zero = false;
  for (size_t i = 0; i < prop.size(); ++i) {
    if (prop[i] == 0.0)
      found_zero = true;
  }
  if (found_zero)
    throw NRLib::Exception("Invalid arguments: One of the properties is zero.");

  double eff_prop = 0.0;
  for (size_t i = 0; i < volumefraction.size(); ++i)
    eff_prop += volumefraction[i] / prop[i];
  eff_prop = 1.0 / eff_prop;

  return eff_prop;
}

double
DEMTools::CalcEffectiveElasticModuliUsingVoigt(const std::vector<double> & prop,
                                               const std::vector<double> & volumefraction) {

  assert (prop.size() == volumefraction.size());
  assert (std::accumulate(volumefraction.begin(), volumefraction.end(), 0.0) == 1.0);

  double eff_prop = 0.0;
  for (size_t i = 0; i < volumefraction.size(); ++i)
    eff_prop += volumefraction[i] * prop[i];

  return eff_prop;

}

double
DEMTools::CalcEffectiveDensity(const std::vector<double> & rho,
                               const std::vector<double> & volumefraction) {

  assert (rho.size() == volumefraction.size());
  assert (std::accumulate(volumefraction.begin(), volumefraction.end(), 0.0) == 1.0);

  double eff_rho = 0.0;
  for (size_t i = 0; i < volumefraction.size(); ++i)
    eff_rho += volumefraction[i] * rho[i];

  return eff_rho;
}


void
DEMTools::CalcEffectiveBulkAndShearModulus(const std::vector<double>&       bulk_modulus,
                                           const std::vector<double>&       shear_modulus,
                                           const std::vector<double>&       aspect_ratio,
                                           std::vector<double>&             concentration,
                                           double                           bulk_modulus_bg,
                                           double                           shear_modulus_bg,
                                           double&                    effective_bulk_modulus,
                                           double&                    effective_shear_modulus) {

  DEM dem(bulk_modulus,
          shear_modulus,
          aspect_ratio,
          concentration,
          bulk_modulus_bg,
          shear_modulus_bg);

  effective_bulk_modulus = effective_shear_modulus = 0;
  dem.CalcEffectiveModulus(effective_bulk_modulus, effective_shear_modulus);

}


double
DEMTools::CalcVelocityOfBrineFromTPS(double temperature,
                                     double pressure,
                                     double salinity) {

  return CalcVelocityOfWaterFromTP(temperature, pressure) +
         salinity*(1170.0 - 9.6*temperature + 0.055*(temperature*temperature) - 8.5E-5*(temperature*temperature*temperature) +
         2.6*pressure - 0.0029*temperature*pressure - 0.0476*(pressure*pressure)) +
         pow(salinity, 1.5)*(780.0 - 10.0*pressure + 0.16*(pressure*pressure)) - 1820*(salinity*salinity);

}

//calculate acoustic velocity of water
double
DEMTools::CalcVelocityOfWaterFromTP(double temperature,
                                    double pressure) {
  std::vector< std::vector<double> > omega(5, std::vector<double>(4, 0.0));

  omega[0][0] = 1402.85;
  omega[0][1] = 1.524;
  omega[0][2] = 3.437E-3;
  omega[0][3] = -1.197E-5;

  omega[1][0] = 4.871;
  omega[1][1] = -0.0111;
  omega[1][2] = 1.739E-4;
  omega[1][3] = -1.628E-6;

  omega[2][0] = -0.04783;
  omega[2][1] = 2.747E-4;
  omega[2][2] = -2.135E-6;
  omega[2][3] = 1.237E-8;

  omega[3][0] = 1.487E-4;
  omega[3][1] = -6.503E-7;
  omega[3][2] = -1.455E-8;
  omega[3][3] = 1.327E-10;

  omega[4][0] =  -2.197E-7;
  omega[4][1] = 7.987E-10;
  omega[4][2] = 5.230E-11;
  omega[4][3] = -4.614E-13;

  double ww = 0.0;

  for (int i = 0; i < 5; i++)
    for (int j = 0; j < 4; j++)
      ww += omega[i][j]*pow(temperature, i)*pow(pressure, j);

  return ww;
}


double
DEMTools::CalcDensityOfWaterFromTP(double temperature,
                                   double pressure) {

  return  1.0 + 1E-6*(-80*temperature -3.3*(temperature*temperature) + 0.00175*(temperature*temperature*temperature) +
                      489*pressure - 2*temperature*pressure + 0.016*(temperature*temperature)*pressure -
                      1.3E-5*(temperature*temperature*temperature)*pressure -
                      0.333*(pressure*pressure) - 0.002*temperature*(pressure*pressure));

}

void
DEMTools::CalcSeismicParamsFromElasticParams(const double bulk_modulus,
                                             const double shear_modulus,
                                             const double density,
                                             double & vp,
                                             double & vs) {
  assert(density != 0.0);
  vp = (bulk_modulus + 4.0 * shear_modulus / 3.0) / density;
  assert(vp > 0.0);
  vp = sqrt(vp);
  vs = shear_modulus / density;
  assert(vs > 0.0);
  vs = sqrt(vs);

}

void
DEMTools::DebugTestCalcEffectiveModulus(double& effective_bulk_modulus,
                                        double& effective_shear_modulus,
                                        double& effective_density) {

  // Script for rock physics modelling using differential effective medium theory (DEM).
  // Specifications
  // Mineral properties
  // (According to Table A.4.1, p458-459, RP Handbook, Mavko et al. 2009).
  double quartz_k                = 37;   // gpa
  double quartz_mu               = 44;   // gpa
  double quartz_rho              = 2.65; // g/ccm

  double clay_k                  = 21;   // gpa
  double clay_mu                 = 7;    // gpa
  double clay_rho                = 2.6;  // g/ccm

  // Fluid properties
  // (Brine properties according to Batzle, M. and Wang, Z. 1992, and
  //  CO2 properties according to Z. Wang's compilation and measurements, RPH Tools)
  double porepressure            = 20;   // mpa
  double temperature             = 50;   // �c

  double brine_salinity          = 0.05; // 100*//
  double brine_k                 = CalcBulkModulusOfBrineFromTPS(temperature, porepressure, brine_salinity)*0.001 /*gpa*/;

  double brine_rho               = CalcDensityOfBrineFromTPS(temperature, porepressure, brine_salinity); // g/ccm

  double co2_k;
  double co2_rho;

  CalcCo2Prop(co2_k, co2_rho, temperature, porepressure);



  ////
  enum GeometryType {Spherical, Mixed};
  GeometryType my_geo_type = Mixed; // pore geometry, this enum chooses the one to use.
  // example 1: spherical
  double inclusiongeometry_spectrum          = 1.0;
  double inclusiongeometry_concentration     = 1.0;
  // example 2: mixed (oblate)
  double aspect_ratio2_tmp []          = {1, 0.5000, 0.1000, 0.0100, 1.0000e-003, 1.0000e-004};
  double concentration2_tmp []     = {0.6419, 0.3205, 0.0321, 0.0050, 5.0000e-004, 5.0000e-005};
  std::vector<double> aspect_ratio2(aspect_ratio2_tmp, aspect_ratio2_tmp + 6);
  std::vector<double> concentration2(concentration2_tmp, concentration2_tmp + 6);

  //// plf
  ////
  double porosity = 0.2;
  double lithology = 0.15;     // volume fraction of clay
  double saturation = 0.8;      // volume fraction of brine
  ////

  //// mixing
  // effective solid properties
  ////
  std::vector<double> v_mineral_k;
  v_mineral_k.push_back(clay_k);
  v_mineral_k.push_back(quartz_k);
  std::vector<double> v_mineral_mu;
  v_mineral_mu.push_back(clay_mu);
  v_mineral_mu.push_back(quartz_mu);
  std::vector<double> v_mineral_rho;
  v_mineral_rho.push_back(clay_rho);
  v_mineral_rho.push_back(quartz_rho);
  std::vector<double> v_lith;
  v_lith.push_back(lithology);
  v_lith.push_back(1.0 - lithology);
  double solid_k                 = CalcEffectiveElasticModuliUsingHill(v_mineral_k, v_lith);
  double solid_mu                = CalcEffectiveElasticModuliUsingHill(v_mineral_mu, v_lith);
  double solid_rho               = CalcEffectiveDensity(v_mineral_rho, v_lith);


  ////
  // effective fluid properties
  std::vector<double> v_fluid_k;
  v_fluid_k.push_back(brine_k);
  v_fluid_k.push_back(co2_k);
  std::vector<double> v_fluid_rho;
  v_fluid_rho.push_back(brine_rho);
  v_fluid_rho.push_back(co2_rho);
  std::vector<double> v_sat;
  v_sat.push_back(saturation);
  v_sat.push_back(1.0 - saturation);
  double fluid_k                 = CalcEffectiveElasticModuliUsingReuss(v_fluid_k, v_sat);     // homogeneous
  // fluid.k                 = geqhill(brine.k, saturation, co2.k);      // patchy
  double fluid_rho               = CalcEffectiveDensity(v_fluid_rho, v_sat);
  ////

  //// effective rock physics properties
  //
  ////
  std::vector<double> v_rock_rho;
  v_rock_rho.push_back(fluid_rho);
  v_rock_rho.push_back(solid_rho);
  std::vector<double> v_poro;
  v_poro.push_back(porosity);
  v_poro.push_back(1.0 - porosity);
 effective_density                = CalcEffectiveDensity(v_rock_rho, v_poro);
  // without gassmann
  /*[rock.k rock.mu]        = geqdem(   [solid.k fluid.k*ones(1, nrofinclgeo)], ...
                                      [solid.mu 0*ones(1, nrofinclgeo)], ...
                                      inclusiongeometry.spectrum, ...
                                      porosity*inclusiongeometry.concentration);*/

 std::vector<double> bulk_modulus;
 std::vector<double> shear_modulus;
 std::vector<double> aspect_ratio;
 std::vector<double> concentration;

 if (my_geo_type == Spherical) {
   bulk_modulus =  std::vector<double>(1, fluid_k);
   shear_modulus = std::vector<double>(1, 0.0);
   aspect_ratio = std::vector<double>(1, inclusiongeometry_spectrum);
   concentration = std::vector<double>(1, inclusiongeometry_concentration*porosity);
 }
 else if (my_geo_type == Mixed) {
   bulk_modulus =  std::vector<double>(aspect_ratio2.size(), fluid_k);
   shear_modulus = std::vector<double>(aspect_ratio2.size(), 0.0);
   aspect_ratio  = aspect_ratio2;
   for (size_t i = 0; i < concentration2.size(); i++)
    concentration2[i] *= porosity;
   concentration = concentration2;
 }

  CalcEffectiveBulkAndShearModulus(bulk_modulus,
                                   shear_modulus,
                                   aspect_ratio,
                                   concentration,
                                   solid_k,
                                   solid_mu,
                                   effective_bulk_modulus,
                                   effective_shear_modulus);


}


void
DEMTools::DebugTestCalcEffectiveModulus2(double& effective_bulk_modulus,
                                         double& effective_shear_modulus,
                                         double& effective_density)
{
  std::vector<double> dummy_u(3);
  for(int i=0; i<3; i++)
    dummy_u[i] = 0;

  // Script for rock physics modelling using differential effective medium theory (DEM).
  // Specifications
  // Mineral properties
  // (According to Table A.4.1, p458-459, RP Handbook, Mavko et al. 2009).
  double quartz_k                = 37;   // gpa
  double quartz_mu               = 44;   // gpa
  double quartz_rho              = 2.65; // g/ccm

  Mineral quartz(quartz_k, quartz_mu, quartz_rho, dummy_u);

  double clay_k                  = 21;   // gpa
  double clay_mu                 = 7;    // gpa
  double clay_rho                = 2.6;  // g/ccm

  Mineral clay(clay_k, clay_mu, clay_rho, dummy_u);

  // Fluid properties
  // (Brine properties according to Batzle, M. and Wang, Z. 1992, and
  //  CO2 properties according to Z. Wang's compilation and measurements, RPH Tools)
  double porepressure            = 20;   // mpa
  double temperature             = 50;   // �c

  double brine_salinity          = 0.05; // 100*//

  Brine brine(brine_salinity, temperature, porepressure);

  CO2 co2(temperature, porepressure);
  ////
  //// plf
  ////
  double porosity = 0.2;
  double lithology = 0.15;     // volume fraction of clay
  double saturation = 0.8;      // volume fraction of brine
  ////

  //// mixing
  // effective solid properties
  ////
  std::vector<double> volume_fraction(2);

  volume_fraction[0] = lithology;
  volume_fraction[1] = 1.0 - volume_fraction[0];

  std::vector<Solid*> mineral(2);
  mineral[0] = &clay;
  mineral[1] = &quartz;

  SolidMixed solidmixed(mineral, volume_fraction, DEMTools::Hill);

  double brine_k;
  double brine_rho;
  brine.GetElasticParams(brine_k, brine_rho);

  double co2_k;
  double co2_rho;
  co2.GetElasticParams(co2_k, co2_rho);

  std::vector<double> volume_fraction2(2);
  volume_fraction2[0] = saturation;
  volume_fraction2[1] = 1.0 - volume_fraction2[0];

  std::vector<Fluid*> fluid(2);
  fluid[0] = &brine;
  fluid[1] = &co2;


  FluidMixed fluid_mix(fluid, volume_fraction2, DEMTools::Reuss);
  ////


  //// effective rock physics properties
  // without gassmann
  /*[rock.k rock.mu]        = geqdem(   [solid.k fluid.k*ones(1, nrofinclgeo)], ...
                                      [solid.mu 0*ones(1, nrofinclgeo)], ...
                                      inclusiongeometry.spectrum, ...
                                      porosity*inclusiongeometry.concentration);*/
  enum GeometryType {Spherical, Mixed};
  GeometryType my_geo_type = Mixed; // pore geometry, this enum chooses the one to use.

  std::vector<double> inclusion_spectrum;
  std::vector<double> inclusion_concentration;

  if (my_geo_type == Spherical) {
    inclusion_spectrum.push_back(1.0);
    inclusion_concentration.push_back(1.0);
  }
  else if (my_geo_type == Mixed) {
    inclusion_spectrum.push_back(1.0);
    inclusion_spectrum.push_back(0.5000);
    inclusion_spectrum.push_back(0.1000);
    inclusion_spectrum.push_back(0.0100);
    inclusion_spectrum.push_back(1.0000e-003);
    inclusion_spectrum.push_back(1.0000e-004);
    inclusion_concentration.push_back(0.6419);
    inclusion_concentration.push_back(0.3205);
    inclusion_concentration.push_back(0.0321);
    inclusion_concentration.push_back(0.0050);
    inclusion_concentration.push_back(5.0000e-004);
    inclusion_concentration.push_back(5.0000e-005);
  }

  RockInclusion rock_inclusion(&solidmixed, &fluid_mix, inclusion_spectrum, inclusion_concentration, porosity);
  rock_inclusion.GetElasticParams(effective_bulk_modulus, effective_shear_modulus, effective_density);

}


//void
//DEMTools::DebugTestCalcEffectiveModulus3(double& effective_bulk_modulus,
//                                         double& effective_shear_modulus,
//                                         double& effective_density) {
//
//  //// Script for rock physics modelling using differential effective medium theory (DEM).
//
//  //// Mineral properties, distribution functions.
//  //// (According to Table A.4.1, p458-459, RP Handbook, Mavko et al. 2009).
//  NRLib::Delta distr_quartz_k(37.0);
//  NRLib::Delta distr_quartz_mu(44.0);
//  NRLib::Delta distr_quartz_rho(2.65);
//  DistributionsMineralEvolution * distr_quartz_evolution = NULL;
//  DistributionsSolid * distr_quartz = new DistributionsMineral(&distr_quartz_k, &distr_quartz_mu, &distr_quartz_rho, distr_quartz_evolution);
//  //Solid * quartz = distr_quartz->GenerateSample();
//
//  NRLib::Delta distr_clay_k(21.0);
//  NRLib::Delta distr_clay_mu(7.0);
//  NRLib::Delta distr_clay_rho(2.6);
//  DistributionsMineralEvolution * distr_clay_evolution = NULL;
//  DistributionsSolid * distr_clay = new DistributionsMineral(&distr_clay_k, &distr_clay_mu, &distr_clay_rho, distr_clay_evolution);
//  //Solid * clay = distr_clay->GenerateSample();
//
//  //// Mixing, effective solid properties. Distribution functions.
//  std::vector< DistributionsSolid * > distr_solid;
//  distr_solid.push_back(distr_clay);
//  distr_solid.push_back(distr_quartz);
//  std::vector< NRLib::Distribution<double> * > distr_vol_frac;
//  distr_vol_frac.push_back(new NRLib::Delta(0.15));
//  distr_vol_frac.push_back(new NRLib::Delta(1.0 - 0.15));
//  DistributionsSolidMixEvolution * distr_solidmix_evolution = NULL;
//  DistributionsSolid * distr_solid_mixed = new DistributionsSolidMix(distr_solid, distr_vol_frac, DEMTools::Hill, distr_solidmix_evolution);
//  //Solid * solid_mixed = distr_solid_mixed->GenerateSample();
//
//  //// Fluid properties
//  //// (Brine properties according to Batzle, M. and Wang, Z. 1992, and
//  //// CO2 properties according to Z. Wang's compilation and measurements, RPH Tools)
//  NRLib::Delta distr_pore_pressure(20.0);
//  NRLib::Delta distr_temperature(50.0);
//  NRLib::Delta distr_salinity(0.05);
//  DistributionsBrineEvolution * distr_brine_evolution = NULL;
//  DistributionsCO2Evolution   * distr_co2_evolution   = NULL;
//  DistributionsFluid * distr_brine  = new DistributionsBrine(&distr_temperature, &distr_pore_pressure, &distr_salinity, distr_brine_evolution);
//  DistributionsFluid * distr_co2    = new DistributionsCO2(&distr_temperature, &distr_pore_pressure, distr_co2_evolution);
//  //Fluid * brine = distr_brine->GenerateSample();
//  //Fluid * co2   = distr_co2->GenerateSample();
//
//  //// Mixing, effective fluid properties. Distribution functions.
//  std::vector< DistributionsFluid * > distr_fluid;
//  distr_fluid.push_back(distr_brine);
//  distr_fluid.push_back(distr_co2);
//  std::vector< NRLib::Distribution<double> * > distr_vol_frac2;
//  distr_vol_frac2.push_back(new NRLib::Delta(0.8));
//  distr_vol_frac2.push_back(new NRLib::Delta(1.0 - 0.8));
//  DistributionsFluidMixEvolution * distr_fluidmix_evolution  = NULL;
//  DistributionsFluid * distr_fluid_mixed = new DistributionsFluidMix(distr_fluid, distr_vol_frac2, DEMTools::Reuss, distr_fluidmix_evolution);
//  //Fluid * fluid_mixed = distr_fluid_mixed->GenerateSample();
//
//
//  //// Geometry specification
//  enum GeometryType {Spherical, Mixed};
//  GeometryType my_geo_type = Mixed; // pore geometry, this enum chooses the one to use.
//
//  std::vector< NRLib::Distribution<double> * > distr_incl_spectrum;
//  std::vector< NRLib::Distribution<double> * > distr_incl_concentration;
//
//  if (my_geo_type == Spherical) {
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0));
//    distr_incl_concentration.push_back( new NRLib::Delta(1.0));
//  }
//  else if (my_geo_type == Mixed) {
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.5000));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.1000));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.0100));
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0000e-003));
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0000e-004));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.6419));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.3205));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.0321));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.0050));
//    distr_incl_concentration.push_back( new NRLib::Delta(5.0000e-004));
//    distr_incl_concentration.push_back( new NRLib::Delta(5.0000e-005));
//  }
//  NRLib::Delta distr_porosity(0.2);
//
//
//  //// Rock, distribution functions.
//  DistributionsRockInclusionEvolution * distr_evolution = NULL;
//  DistributionsRock * distr_rock_incl  = new DistributionsRockInclusion(distr_solid_mixed,
//                                                                        distr_fluid_mixed,
//                                                                        distr_incl_spectrum,
//                                                                        distr_incl_concentration,
//                                                                        &distr_porosity,
//                                                                        distr_evolution);
//
//
//  //// Generating a sample of the rock.
//  //// This function call generates the whole tree of solid and fluid objects from which this rock is composed.
//  std::vector<double> dummy(1, 0.0);
//  Rock * rock_incl = distr_rock_incl->GenerateSample(dummy);
//
//  //// Getting the elastic parameters.
//  RockInclusion * rock_incl_casted = dynamic_cast<RockInclusion*>(rock_incl);
//  rock_incl_casted->GetElasticParams(effective_bulk_modulus, effective_shear_modulus, effective_density);
//
//
//  //// Deleting dynamically allocated memory.
//  delete rock_incl;
//  delete distr_rock_incl;
//  size_t n_incl = distr_incl_spectrum.size();
//  for (size_t i = 0; i < n_incl; ++i) {
//    delete distr_incl_spectrum[i];
//    delete distr_incl_concentration[i];
//  }
//
//  delete distr_fluid_mixed;
//  size_t n_vol2 = distr_vol_frac2.size();
//  for (size_t i = 0; i < n_vol2; ++i)
//    delete distr_vol_frac2[i];
//
//  delete distr_co2;
//  delete distr_brine;
//
//  delete distr_solid_mixed;
//  size_t n_vol = distr_vol_frac.size();
//  for (size_t i = 0; i < n_vol; ++i)
//    delete distr_vol_frac[i];
//
//  delete distr_clay;
//  delete distr_quartz;
//}


void
DEMTools::DebugTestCalcEffectiveModulus4(double& effective_bulk_modulus,
                                         double& effective_shear_modulus,
                                         double& effective_density) {

  //// Script for rock physics modelling using differential effective medium theory (DEM).

  //// Mineral properties, distribution functions.
  //// (According to Table A.4.1, p458-459, RP Handbook, Mavko et al. 2009).

  NRLib::Trend * trend_quartz_k          = new NRLib::TrendConstant(37.0);
  DistributionWithTrend * distr_quartz_k = new DeltaDistributionWithTrend(trend_quartz_k, false);

  NRLib::Trend * trend_quartz_mu          = new NRLib::TrendConstant(44.0);
  DistributionWithTrend * distr_quartz_mu = new DeltaDistributionWithTrend(trend_quartz_mu, false);

  NRLib::Trend * trend_quartz_rho          = new NRLib::TrendConstant(2.65);
  DistributionWithTrend * distr_quartz_rho = new DeltaDistributionWithTrend(trend_quartz_rho, false);

  DistributionsMineralEvolution * distr_quartz_evolution = NULL;
  DistributionsSolid * distr_quartz = new DistributionsMineral(distr_quartz_k, distr_quartz_mu, distr_quartz_rho, 0, 0, 0, distr_quartz_evolution);

  NRLib::Trend * trend_clay_k          = new NRLib::TrendConstant(21.0);
  DistributionWithTrend * distr_clay_k = new DeltaDistributionWithTrend(trend_clay_k, false);

  NRLib::Trend * trend_clay_mu          = new NRLib::TrendConstant(7.0);
  DistributionWithTrend * distr_clay_mu = new DeltaDistributionWithTrend(trend_clay_mu, false);

  NRLib::Trend * trend_clay_rho          = new NRLib::TrendConstant(2.6);
  DistributionWithTrend * distr_clay_rho = new DeltaDistributionWithTrend(trend_clay_rho, false);

  DistributionsMineralEvolution * distr_clay_evolution = NULL;
  DistributionsSolid * distr_clay = new DistributionsMineral(distr_clay_k, distr_clay_mu, distr_clay_rho, 0, 0, 0, distr_clay_evolution);

  //// Mixing, effective solid properties. Distribution functions.
  std::vector< DistributionsSolid * > distr_solid;
  distr_solid.push_back(distr_clay);
  distr_solid.push_back(distr_quartz);

  std::vector< DistributionWithTrend * > distr_vol_frac;

  NRLib::Trend * trend_vol_frac    = new NRLib::TrendConstant(0.15);
  NRLib::Trend * trend_vol_fracbg  = new NRLib::TrendConstant(1.0-0.15);

  distr_vol_frac.push_back(new DeltaDistributionWithTrend(trend_vol_frac, false));
  distr_vol_frac.push_back(new DeltaDistributionWithTrend(trend_vol_fracbg, false));

  DistributionsSolidMixEvolution * distr_solidmix_evolution = NULL;
  DistributionsSolid * distr_solid_mixed = new DistributionsSolidMix(distr_solid, distr_vol_frac, DEMTools::Hill, distr_solidmix_evolution);
  //Solid * solid_mixed = distr_solid_mixed->GenerateSample();

  //// Fluid properties
  //// (Brine properties according to Batzle, M. and Wang, Z. 1992, and
  //// CO2 properties according to Z. Wang's compilation and measurements, RPH Tools)
  NRLib::Trend * trend_pore                   = new NRLib::TrendConstant(20.0);
  DistributionWithTrend * distr_pore_pressure = new DeltaDistributionWithTrend(trend_pore, false);

  NRLib::Trend * trend_temperature          = new NRLib::TrendConstant(50.0);
  DistributionWithTrend * distr_temperature = new DeltaDistributionWithTrend(trend_temperature, false);

  NRLib::Trend * trend_salinity          = new NRLib::TrendConstant(0.05);
  DistributionWithTrend * distr_salinity = new DeltaDistributionWithTrend(trend_salinity, false);

  DistributionsBrineEvolution * distr_brine_evolution = NULL;
  DistributionsCO2Evolution   * distr_co2_evolution   = NULL;
  DistributionsFluid * distr_brine  = new DistributionsBrine(distr_temperature, distr_pore_pressure, distr_salinity, distr_brine_evolution);
  DistributionsFluid * distr_co2    = new DistributionsCO2(distr_temperature, distr_pore_pressure, distr_co2_evolution);
  //Fluid * brine = distr_brine->GenerateSample();
  //Fluid * co2   = distr_co2->GenerateSample();

  //// Mixing, effective fluid properties. Distribution functions.
  std::vector< DistributionsFluid * > distr_fluid;
  distr_fluid.push_back(distr_brine);
  distr_fluid.push_back(distr_co2);

  std::vector< DistributionWithTrend * > distr_vol_frac2f;
  distr_vol_frac2f.push_back(new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.8), false));
  distr_vol_frac2f.push_back(new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0 - 0.8), false));

  DistributionsFluidMixEvolution * distr_fluidmix_evolution  = NULL;
  DistributionsFluid * distr_fluid_mixed = new DistributionsFluidMix(distr_fluid, distr_vol_frac2f, DEMTools::Reuss, distr_fluidmix_evolution);
  //Fluid * fluid_mixed = distr_fluid_mixed->GenerateSample();


  //// Geometry specification
  enum GeometryType {Spherical, Mixed};
  GeometryType my_geo_type = Mixed; // pore geometry, this enum chooses the one to use.

  std::vector< DistributionWithTrend * > distr_incl_spectrum;
  std::vector< DistributionWithTrend * > distr_incl_concentration;

  if (my_geo_type == Spherical) {
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0), false));
  }
  else if (my_geo_type == Mixed) {
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0), false));
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.5000), false));
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.1000), false));
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.0100), false));
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0000e-003), false));
    distr_incl_spectrum.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(1.0000e-004), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.6419), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.3205), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.0321), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(0.0050), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(5.0000e-004), false));
    distr_incl_concentration.push_back( new DeltaDistributionWithTrend(new NRLib::TrendConstant(5.0000e-005), false));
  }

  NRLib::Trend * trend_porosity          = new NRLib::TrendConstant(0.2);
  DistributionWithTrend * distr_porosity = new DeltaDistributionWithTrend(trend_porosity, false);


  //// Rock, distribution functions.
  DistributionsRockInclusionEvolution * distr_evolution = NULL;
  DistributionsRock * distr_rock_incl  = new DistributionsRockInclusion(distr_solid_mixed,
                                                                        distr_fluid_mixed,
                                                                        distr_incl_spectrum,
                                                                        distr_incl_concentration,
                                                                        distr_porosity,
                                                                        distr_evolution);


  //// Generating a sample of the rock.
  //// This function call generates the whole tree of solid and fluid objects from which this rock is composed.
  std::vector<double> dummy(2, 0.0);
  Rock * rock_incl = distr_rock_incl->GenerateSample(dummy);

  //// Getting the elastic parameters.
  RockInclusion * rock_incl_casted = dynamic_cast<RockInclusion*>(rock_incl);
  rock_incl_casted->GetElasticParams(effective_bulk_modulus, effective_shear_modulus, effective_density);


  //// Deleting dynamically allocated memory.
  delete rock_incl;
  delete distr_rock_incl;
  size_t n_incl = distr_incl_spectrum.size();
  for (size_t i = 0; i < n_incl; ++i) {
    delete distr_incl_spectrum[i];
    delete distr_incl_concentration[i];
  }

  delete distr_fluid_mixed;
  size_t n_vol2 = distr_vol_frac2f.size();
  for (size_t i = 0; i < n_vol2; ++i)
    delete distr_vol_frac2f[i];

  delete distr_co2;
  delete distr_brine;

  delete distr_solid_mixed;
  size_t n_vol = distr_vol_frac.size();
  for (size_t i = 0; i < n_vol; ++i)
    delete distr_vol_frac[i];

  delete distr_clay;
  delete distr_quartz;

  delete distr_quartz_k;
  delete distr_quartz_mu;
  delete distr_quartz_rho;

  delete distr_clay_k;
  delete distr_clay_mu;
  delete distr_clay_rho;

  delete distr_pore_pressure;
  delete distr_temperature;

  delete distr_salinity;

  delete distr_porosity;
}

//void
//DEMTools::DebugTestDeletionAndCopying() {
//
//  //// For testing the RP design's memory allocation and freeing of
//  //// memory, using the differential effective medium theory (DEM).
//  //// Will be further developed.
//
//  ////SETTING UP DISTRIBUTIONS************************************
//  //// Mineral properties, distribution functions.
//  //// (According to Table A.4.1, p458-459, RP Handbook, Mavko et al. 2009).
//  NRLib::Delta distr_quartz_k(37.0);
//  NRLib::Delta distr_quartz_mu(44.0);
//  NRLib::Delta distr_quartz_rho(2.65);
//  DistributionsMineralEvolution * distr_quartz_evolution = NULL;
//  DistributionsSolid * distr_quartz = new DistributionsMineral(&distr_quartz_k, &distr_quartz_mu, &distr_quartz_rho, distr_quartz_evolution);
//
//  NRLib::Delta distr_clay_k(21.0);
//  NRLib::Delta distr_clay_mu(7.0);
//  NRLib::Delta distr_clay_rho(2.6);
//  DistributionsMineralEvolution * distr_clay_evolution = NULL;
//  DistributionsSolid * distr_clay = new DistributionsMineral(&distr_clay_k, &distr_clay_mu, &distr_clay_rho, distr_clay_evolution);
//
//  //// Mixing, effective solid properties. Distribution functions.
//  std::vector< DistributionsSolid * > distr_solid;
//  distr_solid.push_back(distr_clay);
//  distr_solid.push_back(distr_quartz);
//  std::vector< NRLib::Distribution<double> * > distr_vol_frac;
//  distr_vol_frac.push_back(new NRLib::Delta(0.15));
//  distr_vol_frac.push_back(new NRLib::Delta(1.0 - 0.15));
//  DistributionsSolidMixEvolution * distr_solidmix_evolution = NULL;
//  DistributionsSolid * distr_solid_mixed = new DistributionsSolidMix(distr_solid, distr_vol_frac, DEMTools::Hill, distr_solidmix_evolution);
//
//  //// Fluid properties
//  //// (Brine properties according to Batzle, M. and Wang, Z. 1992, and
//  //// CO2 properties according to Z. Wang's compilation and measurements, RPH Tools)
//  NRLib::Delta distr_pore_pressure(20.0);
//  NRLib::Delta distr_temperature(50.0);
//  NRLib::Delta distr_salinity(0.05);
//  DistributionsBrineEvolution * distr_brine_evolution = NULL;
//  DistributionsCO2Evolution   * distr_co2_evolution   = NULL;
//  DistributionsFluid * distr_brine  = new DistributionsBrine(&distr_temperature, &distr_pore_pressure, &distr_salinity, distr_brine_evolution);
//  DistributionsFluid * distr_co2    = new DistributionsCO2(&distr_temperature, &distr_pore_pressure, distr_co2_evolution);
//
//  //// Mixing, effective fluid properties. Distribution functions.
//  std::vector< DistributionsFluid * > distr_fluid;
//  distr_fluid.push_back(distr_brine);
//  distr_fluid.push_back(distr_co2);
//  std::vector< NRLib::Distribution<double> * > distr_vol_frac2;
//  distr_vol_frac2.push_back(new NRLib::Delta(0.8));
//  distr_vol_frac2.push_back(new NRLib::Delta(1.0 - 0.8));
//  DistributionsFluidMixEvolution * distr_fluidmix_evolution  = NULL;
//  DistributionsFluid * distr_fluid_mixed = new DistributionsFluidMix(distr_fluid, distr_vol_frac2, DEMTools::Reuss, distr_fluidmix_evolution);
//
//  //// Geometry specification
//  enum GeometryType {Spherical, Mixed};
//  GeometryType my_geo_type = Mixed; // pore geometry, this enum chooses the one to use.
//
//  std::vector< NRLib::Distribution<double> * > distr_incl_spectrum;
//  std::vector< NRLib::Distribution<double> * > distr_incl_concentration;
//
//  if (my_geo_type == Spherical) {
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0));
//    distr_incl_concentration.push_back( new NRLib::Delta(1.0));
//  }
//  else if (my_geo_type == Mixed) {
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.5000));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.1000));
//    distr_incl_spectrum.push_back( new NRLib::Delta(0.0100));
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0000e-003));
//    distr_incl_spectrum.push_back( new NRLib::Delta(1.0000e-004));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.6419));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.3205));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.0321));
//    distr_incl_concentration.push_back( new NRLib::Delta(0.0050));
//    distr_incl_concentration.push_back( new NRLib::Delta(5.0000e-004));
//    distr_incl_concentration.push_back( new NRLib::Delta(5.0000e-005));
//  }
//  NRLib::Delta distr_porosity(0.2);
//
//
//  //// Rock, distribution functions.
//  DistributionsRockInclusionEvolution * distr_evolution = NULL;
//  DistributionsRock * distr_rock_incl  = new DistributionsRockInclusion(distr_solid_mixed,
//                                                                        distr_fluid_mixed,
//                                                                        distr_incl_spectrum,
//                                                                        distr_incl_concentration,
//                                                                        &distr_porosity,
//                                                                        distr_evolution);
//
//
//  ////TESTING************************************
//  //// Generating a sample of the rock.
//  //// This function call generates the whole tree of solid and fluid objects from which this rock is composed.
//  std::vector<double> dummy(1, 0.0);
//  std::vector<double>  mean_m = distr_rock_incl->GetExpectation(dummy);
//  NRLib::Grid2D<double> cov_m = distr_rock_incl->GetCovariance(dummy);
//  Rock * rock_incl = distr_rock_incl->GenerateSample(dummy);
//
//  //// Evolve Rock --- Testing
//  std::vector<Rock *> rock_tmp;
//  std::vector<int> delta_time2(1,6);
//  Rock * rock_incl_new1 = rock_incl->Evolve(delta_time2, rock_tmp);
//  rock_tmp.push_back(rock_incl_new1);
//  delta_time2.push_back(32);
//  Rock * rock_incl_new2 = rock_incl->Evolve(delta_time2, rock_tmp);
//
//  //// COPYING
//  Rock * rock_incl_new3 = rock_incl->Evolve(delta_time2, rock_tmp);
//  Rock * rock_incl_base = rock_incl_new3->Clone(); // Copy!
//
//  const Solid * s3 = (dynamic_cast<RockInclusion*>(rock_incl_new3))->GetSolid();
//  const Fluid * f3 = (dynamic_cast<RockInclusion*>(rock_incl_new3))->GetFluid();
//  assert(s3 != NULL && f3 != NULL);
//  delete rock_incl_new3;
//  // Now s3 and f3 shoud be pointing to nothing.
//
//  // test that rock_incl_base is intact after rock_incl_new3 is deleted.
//  double vp, vs, rho;
//  rock_incl_base->ComputeSeismicParams(vp, vs, rho);
//  const Solid * s = (dynamic_cast<RockInclusion*>(rock_incl_base))->GetSolid();
//  const Fluid * f = (dynamic_cast<RockInclusion*>(rock_incl_base))->GetFluid();
//  assert(s != NULL && f != NULL); // Now s and f shoud be meaningful.
//  // A note on object deletion: There is no compiler errors or warnings
//  // if we try to invoke
//  // delete s;
//  // delete f;
//  // I.e. the const in the return values of functions GetSolid() and
//  // getFluid() does not prevent this. But when we later come to
//  // delete rock_incl_base;
//  // there is a run-time error since we try to free memory that has
//  // already been freed.
//  delete rock_incl_base;
//
//  {
//    // ASSIGNMENT ROCK
//    std::vector<double> incl_spec(1, 1.0);
//    std::vector<double> incl_conc(1, 1.0);
//    Solid * q = distr_quartz->GenerateSample(dummy);
//    Fluid * b = distr_brine->GenerateSample(dummy);
//    RockInclusion rock_incl2(q, b, incl_spec, incl_conc, 0.3);
//    rock_incl2 = *(dynamic_cast<RockInclusion*>(rock_incl)); // Assignment
//    /*const Solid * s4 = (dynamic_cast<RockInclusion*>(rock_incl))->GetSolid();
//    const Fluid * f4 = (dynamic_cast<RockInclusion*>(rock_incl))->GetFluid();
//    const Solid * s_incl = rock_incl2.GetSolid();
//    const Fluid * f_incl = rock_incl2.GetFluid();*/
//    delete q;
//    delete b;
//  } //Here rock_incl2, s4, f4, s_incl, and f_incl go out of scope. Destruction should work!
//  delete rock_incl; // Should destroy s4 and f4, but not s_incl and f_incl.
//
//  {
//    Solid * quartz = distr_quartz->GenerateSample(dummy);
//    Solid * clay = distr_clay->GenerateSample(dummy);
//    Fluid * brine = distr_brine->GenerateSample(dummy);
//    Fluid * co2   = distr_co2->GenerateSample(dummy);
//
//    Solid * solid_mixed = distr_solid_mixed->GenerateSample(dummy);
//    std::vector<Solid *> s_ass;
//    s_ass.push_back(clay);
//    s_ass.push_back(quartz);
//    std::vector<double>  v_ass;
//    v_ass.push_back(0.3);
//    v_ass.push_back(0.7);
//    // ASSIGNMENT SOLIDMIXED
//    SolidMixed sol_mixed_ass(s_ass, v_ass, DEMTools::Hill);
//    sol_mixed_ass = *(dynamic_cast<SolidMixed*>(solid_mixed)); // Assignment
//    double k_mix, mu_mix, rho_mix;
//    solid_mixed->ComputeElasticParams(k_mix, mu_mix, rho_mix);
//
//    delete solid_mixed;
//    double k_ass, mu_ass, rho_ass;
//    sol_mixed_ass.ComputeElasticParams(k_ass, mu_ass, rho_ass); // Should work.
//    //solid_mixed->ComputeElasticParams(k_mix, mu_mix, rho_mix); //Should not work.
//
//    Fluid * fluid_mixed = distr_fluid_mixed->GenerateSample(dummy);
//    std::vector<Fluid *> f_ass;
//    f_ass.push_back(co2);
//    f_ass.push_back(brine);
//    std::vector<double>  vf_ass;
//    vf_ass.push_back(0.47);
//    vf_ass.push_back(0.53);
//    // ASSIGNMENT FLUIDMIXED
//    FluidMixed flu_mixed_ass(f_ass, vf_ass, DEMTools::Reuss);
//    flu_mixed_ass = *(dynamic_cast<FluidMixed*>(fluid_mixed)); // Assignment
//    double fk_mix, frho_mix;
//    fluid_mixed->GetElasticParams(fk_mix, frho_mix);
//
//    delete fluid_mixed;
//    double fk_ass, frho_ass;
//    flu_mixed_ass.GetElasticParams(fk_ass, frho_ass); // Should work.
//    //fluid_mixed->GetElasticParams(fk_mix, frho_mix); //Should not work.
//
//
//    delete clay;
//    delete quartz;
//    delete brine;
//    delete co2;
//  }  //Here sol_mixed_ass and flu_mixed_ass go out of scope. Destruction should work!
//
//
//  delete rock_incl_new2;
//  delete rock_incl_new1;
//  //delete rock_incl;
//
//  //// Deleting dynamically allocated memory for distribution functions.
//  delete distr_rock_incl;
//  size_t n_incl = distr_incl_spectrum.size();
//  for (size_t i = 0; i < n_incl; ++i) {
//    delete distr_incl_spectrum[i];
//    delete distr_incl_concentration[i];
//  }
//
//  delete distr_fluid_mixed;
//  size_t n_vol2 = distr_vol_frac2.size();
//  for (size_t i = 0; i < n_vol2; ++i)
//    delete distr_vol_frac2[i];
//
//  delete distr_co2;
//  delete distr_brine;
//
//  delete distr_solid_mixed;
//  size_t n_vol = distr_vol_frac.size();
//  for (size_t i = 0; i < n_vol; ++i)
//    delete distr_vol_frac[i];
//
//  delete distr_clay;
//  delete distr_quartz;
//
//}
