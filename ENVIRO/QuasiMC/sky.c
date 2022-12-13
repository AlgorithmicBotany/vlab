/* sky.c - implementaion of the sky model interfaced by sky.h */
/*
 * March, 2018
 * The formulae for the clear sky and overcast sky were modified to account for
 * a new zenith luminace equation. Neil White identified a problem with the old
 * sky model when simulating lighting conditions at latitudes from -10 to -44.
 * He was getting strange spikes in radiant flux at around the 354 day of the
 * year. I found that the previous zenith luminance equation was returning such
 * large spikes for these parameters. I could not identify the problem with the
 * zenith luminance equation, so I modified the sky model based on the following
 * paper: A. J. Preetham, Peter Shirley, and Brian Smits. 1999. A practical
 * analytic model for daylight. In Proceedings of the 26th annual conference on
 * Computer graphics and interactive techniques (SIGGRAPH '99). ACM
 * Press/Addison-Wesley Publishing Co., New York, NY, USA, 91-100.
 * DOI=http://dx.doi.org/10.1145/311535.311545
 * The sky model with equations from this paper does not produce spikes in
 * radiant flux. Equation numbers from the paper:
 * (1) - clear sky model
 * (2) - overcast model
 * (A.2) - zenith luminance
 * (A.6) - sun position
 * */

#include "quasiMC.h"
#include "randquasimc.h"

#define RADIANS(x) ((x) / 180.0 * M_PI)

#ifdef _MSC_VER
#define round(x) floor(x + 0.5)
#endif

double ZenithLuminace(double sun_alt);
double ClearSky(double theta, double phi, double alt, double azi);
double OverCastSky(double theta, double alt);
void ComputeIntensity(void);
void ComputeDistribution(void);

// parameters set by args.c
static double latitude, longitude;
static double clear_days, turbidity;
static double start_time,
    end_time;                  // growth period from 0 to 24 (in decimal hours)
static double julian_day;      // day of the year in 1 to 365
static char sky_file_name[64]; // name of sky file to use

float max_sky_intensity;
double intensity[AZIMUTH_SAMPLES][ALTITUDE_SAMPLES];
static double intensity_dist[AZIMUTH_SAMPLES][ALTITUDE_SAMPLES];

static int loc_set = 0, weather_set = 0, period_set = 0, julian_day_set = 0,
           sky_file_set = 0;

/* -------------------------------------------------------------------------- */

int InitSkyModel(void) {
  /* set default parameters */
  if (!loc_set) {
    latitude = 0.0;
    longitude = 0.0;
  }

  if (!weather_set) {
    clear_days = 1.0;
    turbidity = 2.45;
  }

  if (!period_set) {
    start_time = 0.0;
    end_time = 24.0;
  }

  if (!julian_day_set) {
    julian_day = 172.0;
  }

  max_sky_intensity = 0.0;
  ComputeIntensity();
  ComputeDistribution();

  return (1);
}

/* -------------------------------------------------------------------------- */

void SetLocation(double lati, double longi) {
  latitude = lati;
  longitude = longi;
  loc_set = 1;
  return;
}

/* -------------------------------------------------------------------------- */

void SetWeather(double clear, double turb) {
  clear_days = clear;
  turbidity = turb;
  weather_set = 1;
  return;
}

/* -------------------------------------------------------------------------- */

void SetJulianDay(int jd) {
  julian_day = (double)jd;
  julian_day_set = 1;
  return;
}

/* -------------------------------------------------------------------------- */

void SetGrowthPeriod(float period[6]) {
  // growth period specified in decimal hours over one day (0-24)
  start_time = period[0];
  end_time = period[1];
  period_set = 1;
  return;
}

void SetSkyFile(char *filename) {
  strcpy(sky_file_name, filename);
  sky_file_set = 1;
  return;
}

/* -------------------------------------------------------------------------- */

void FreeSkyModel(void) {
  loc_set = 0;
  weather_set = 0;
  period_set = 0;
  sky_file_set = 0;
  return;
}

/* -------------------------------------------------------------------------- */

float GetSkyIntensity(float *dir)
// NOTE: this actually returns the 'radiance' (W m^-2 sr^-1) of the light in
// direction 'dir' not the intensity in general, 'intensity' is not a good term
// to use because it is often incorrectly used
{
  double azimuth, altitude, len;
  int azi_index, alt_index;

  // check if direction is below the horizon
  if (dir[Y] < 0.0)
    return (0.0);

  // angle from zenith
  len = sqrt(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
  if (len < EPSILON)
    altitude = 0.0;
  else
    altitude = acos(dir[1] / len);

  // azimuth angle
  len = sqrt(dir[0] * dir[0] + dir[2] * dir[2]);
  if (len < EPSILON)
    azimuth = 0.0;
  else
    azimuth = acos(dir[0] / len);

  if (dir[2] < 0.0)
    azimuth = -azimuth;

#if NEW_SKY_MODEL == 0
  azi_index = (int)((azimuth + M_PI) / (2.0 * M_PI) * (double)AZIMUTH_SAMPLES);
  alt_index = (int)(altitude / (0.5 * M_PI) * (double)ALTITUDE_SAMPLES);
#else
  azi_index = (int)((azimuth + M_PI) / (2.0 * M_PI) * (double)AZIMUTH_SAMPLES);
  alt_index =
      (int)-((altitude - M_PI * 0.5) / (0.5 * M_PI) * (double)ALTITUDE_SAMPLES);
#endif

  if (azi_index < 0)
    azi_index = 0;
  if (azi_index >= AZIMUTH_SAMPLES)
    azi_index = AZIMUTH_SAMPLES - 1;
  if (alt_index >= ALTITUDE_SAMPLES)
    alt_index = ALTITUDE_SAMPLES - 1;

  return ((float)intensity[azi_index][alt_index]);
}

/* -------------------------------------------------------------------------- */

float GetSkyDirection(float *dir, float *xvec, float *yvec)
/* returns a light ray sampled from the "intensity" distribution, with 'dir'
 * being the direction of the ray, 'yvec' and 'xvec' forming the basis */
{
  double azi, alt, x, y;
  double azi_x, alt_y;
  double sinalt, sinazi, cosazi;
  int x_index, y_index;

  if (intensity_dist[AZIMUTH_SAMPLES - 1][ALTITUDE_SAMPLES - 1] <= 0.00001) {
    dir[0] = dir[1] = dir[2] = 0.f;
    xvec[0] = xvec[1] = xvec[2] = 0.f;
    yvec[0] = yvec[1] = yvec[2] = 0.f;
    return (0.f);
  }

  // use x to pick a random azimuth angle. x is randomly choosen from the CDF
  x = RandU01(RANDQMC_START) *
      intensity_dist[AZIMUTH_SAMPLES - 1][ALTITUDE_SAMPLES - 1];

  // find the index of x in the CDF
  x_index = 0;
  while (x > intensity_dist[x_index][ALTITUDE_SAMPLES - 1] &&
         x_index < AZIMUTH_SAMPLES)
    ++x_index;

  if (x_index >= AZIMUTH_SAMPLES) {
    fprintf(stderr,
            "quasiMC - floating point precision error in sky azimuth sample\n");
    x_index = AZIMUTH_SAMPLES - 1;
  }

  // if the x index is zero we have a special case because (x_index-1) is
  // used when x_index != 0, which will cause seg faults if x_index = 0.
  if (x_index == 0) {
    // find the interpolating value for the azimuth
    // with the zeros: azi_x = (x-0.0) / (distF[0][PHI_SAMPLES-1] - 0.0)
    azi_x = x / intensity_dist[0][ALTITUDE_SAMPLES - 1];

    // use y to pick a random altitude angle knowing that x_index = 0
    y = RandU01(RANDQMC_START) * intensity_dist[0][ALTITUDE_SAMPLES - 1];
  } else {
    // find the interpolation value for the azimuth angle
    azi_x = (x - intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1]) /
            (intensity_dist[x_index][ALTITUDE_SAMPLES - 1] -
             intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1]);

    // use y to pick a random altitude knowing that x_index != 0
    y = intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1] +
        RandU01(RANDQMC_START) *
            (intensity_dist[x_index][ALTITUDE_SAMPLES - 1] -
             intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1]);
  }

  // find the index of y in the CDF
  y_index = 0;
  while (y > intensity_dist[x_index][y_index] && y_index < ALTITUDE_SAMPLES)
    ++y_index;

  if (y_index >= ALTITUDE_SAMPLES) {
    fprintf(
        stderr,
        "quasiMC - floating point precision error in sky altitude sample\n");
    y_index = ALTITUDE_SAMPLES - 1;
  }

  // find the interpolating value for phi
  if (y_index != 0)
    alt_y = (y - intensity_dist[x_index][y_index - 1]) /
            (intensity_dist[x_index][y_index] -
             intensity_dist[x_index][y_index - 1]);
  else if (x_index != 0)
    alt_y = (y - intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1]) /
            (intensity_dist[x_index][y_index] -
             intensity_dist[x_index - 1][ALTITUDE_SAMPLES - 1]);
  else
    // alt_y = (y - 0.0)/(intensity_dist[x_index][y_index] - 0.0);
    alt_y = y / intensity_dist[x_index][y_index];

  // using x_index and y_index find the value of the azimuth and altitude angles
  azi = (((double)x_index + azi_x) / (double)(AZIMUTH_SAMPLES)) * 2.0 * M_PI -
        M_PI;
  alt = (((double)y_index + alt_y) / (double)(ALTITUDE_SAMPLES)) * 0.5 * M_PI;

  if (alt < 0.0 || alt > 0.5 * M_PI)
    fprintf(stderr, "sky - altitude angle, %g, is out of range.\n", alt);

  if (azi < -M_PI || azi > M_PI)
    fprintf(stderr, "sky - azimuth angle, %g, is out of range.\n", azi);

  // determine ray direction dir
  alt = 0.5 * M_PI - alt; // added this, as dir was incorret otherwise (sun on
                          // horizon -> ray dir to zenith!)

#if NEW_SKY_MODEL == 0
  dir[Y] = (float)cos(alt);
  sinalt = sin(alt);

  cosazi = cos(azi);
  sinazi = sin(azi);
  dir[X] = (float)(sinalt * cosazi);
  dir[Z] = (float)(sinalt * sinazi);

  // get two vectors perpendicular to dir, yvec is pointing "up"
  yvec[Y] = (float)sinalt;
  yvec[X] = (float)(-dir[Y] * cosazi);
  yvec[Z] = (float)(-dir[Y] * sinazi);

  // xvec is perpendicular both to dir and yvec
  xvec[Y] = 0.0f;
  xvec[X] = (float)-sinazi;
  xvec[Z] = (float)cosazi;
#else

  dir[Y] = (float)cos(alt);
  sinalt = sin(alt);

  cosazi = cos(azi);
  sinazi = sin(azi);
  // compared to first sky model, the new model has different coordinate system
  // in the first one: +X was South, +Z was West
  // in the new one: +X is East, +Z is South
  // so rotate around Y-axis by 90 degrees counter clockwise
  dir[X] = (float)(sinalt * sinazi);
  dir[Z] = (float)-(sinalt * cosazi);

  // get two vectors perpendicular to dir, yvec is pointing "up"
  yvec[Y] = (float)sinalt;
  yvec[X] = (float)(-dir[Y] * sinazi);
  yvec[Z] = (float)(dir[Y] * cosazi);

  // xvec is perpendicular both to dir and yvec
  xvec[Y] = 0.0f;
  xvec[X] = (float)cosazi;
  xvec[Z] = (float)sinazi;

#endif

  // return intensity of light
  return ((float)intensity[x_index][y_index]);
}

/* -------------------------------------------------------------------------- */

void ComputeDistribution(void)
// computes the cumulative distribution function
{
  int i, j;

  // The CDF is computed in two steps.
  // 1) Sum up the columns of the PDF, i.e., the ALTITUDE_SAMPLES
  // 2) Add the last entry of each column into every entry of the next column.
  for (i = 0; i < AZIMUTH_SAMPLES; i++) {
    intensity_dist[i][0] = intensity[i][0];
    for (j = 1; j < ALTITUDE_SAMPLES; j++)
      intensity_dist[i][j] = intensity[i][j] + intensity_dist[i][j - 1];
  }

  for (i = 1; i < AZIMUTH_SAMPLES; i++)
    for (j = 0; j < ALTITUDE_SAMPLES; j++)
      intensity_dist[i][j] =
          intensity_dist[i][j] + intensity_dist[i - 1][ALTITUDE_SAMPLES - 1];

  return;
}

/* -------------------------------------------------------------------------- */

void ComputeIntensity(void) {
  double standard_time; // time of day in decimal hours
  double rad_meridian;  // standard meridian for time zone in radians
  double rad_longitude; // longitude of locatin in radians (-pi, pi)
  double rad_latitude;  // in radians, zenith = 0, horizon = pi/2
  double sin_latitude, cos_latitude;
  double solar_time, solar_time_adjust;
  double solar_declination, sin_declination, cos_declination;
  double sun_altitude, sun_azimuth;
  double alt_sample, azi_sample;
  double sum_intensity, max_intensity;
  int i, j;
  FILE *skyfile;

  if (sky_file_set) {
    // set variables dealing with light intensity from the sky
    memset(intensity, 0, sizeof(double) * AZIMUTH_SAMPLES * ALTITUDE_SAMPLES);
    sum_intensity = 0.0;

    if ((skyfile = fopen(sky_file_name, "r")) != NULL) {
      fscanf(skyfile, "%d %d", &i, &j);
      if (i != AZIMUTH_SAMPLES || j != ALTITUDE_SAMPLES) {
        fprintf(stderr,
                "QuasiMC - Error! The number of sky input samples must be %d "
                "by %d\n",
                AZIMUTH_SAMPLES, ALTITUDE_SAMPLES);
        fclose(skyfile);
        return;
      }

      // determine the light intensity coming from the sky
      for (i = 0; i < AZIMUTH_SAMPLES; i++) {
        for (j = 0; j < ALTITUDE_SAMPLES; j++) {
          if (fscanf(skyfile, "%lf %lf %lf", &azi_sample, &alt_sample,
                     &max_intensity) != 3)
            fprintf(stderr,
                    "QuasiMC - Error! The sky input file has missing values\n");

          intensity[i][j] = max_intensity > 0.0 ? max_intensity : 0.0;
          sum_intensity += max_intensity > 0.0 ? max_intensity : 0.0;
        }
      }

      fclose(skyfile);
    } else {
      fprintf(stderr, "QuasiMC - Error! Cannot open sky input file: %s\n",
              sky_file_name);
    }
  } else {
    // set variables to determine sun position
    rad_meridian = RADIANS(round(longitude / 12.0) *
                           12.0); // round longitude (there are 12 times zones)
    rad_longitude = RADIANS(longitude);
    rad_latitude = RADIANS(latitude);

    if (fabs(rad_meridian - rad_longitude) > RADIANS(45.0))
      fprintf(stderr,
              "QuasiMC - warning %g hours between standard meridian and "
              "longitude\n",
              (rad_longitude - rad_meridian) * 12.0 / M_PI);

    // set variables dealing with light intensity from the sky
    memset(intensity, 0, sizeof(double) * AZIMUTH_SAMPLES * ALTITUDE_SAMPLES);
    sum_intensity = 0.0;
    solar_declination = 0.4093 * sin(2.0 * M_PI * (julian_day - 81.0) / 368.0);
    sin_declination = sin(solar_declination);
    cos_declination = cos(solar_declination);
    solar_time_adjust = +0.17 * sin(4.0 * M_PI * (julian_day - 80.0) / 373.0) -
                        0.129 * sin(2.0 * M_PI * (julian_day - 8.0) / 355.0) +
                        12.0 * (rad_meridian - rad_longitude) / M_PI;
    sin_latitude = sin(rad_latitude);
    cos_latitude = cos(rad_latitude);
    for (standard_time = start_time; standard_time <= end_time;
         standard_time += 0.1) {
      // determine the position of the sun in the sky
      solar_time = standard_time + solar_time_adjust;
      //+ 0.17*sin(4.0*M_PI*(julian_day-80.0)/373.0)
      //- 0.129*sin(2.0*M_PI*(julian_day-8.0)/355.0)
      //+ 12.0*(rad_meridian - rad_longitude)/M_PI;

      // solar_declination = 0.4093*sin(2.0*M_PI*(julian_day-81.0)/368.0);

#if NEW_SKY_MODEL == 0
      // old equations used in old formula for Lz (luminance at zenith)
      sun_altitude =
          asin(sin_latitude * sin_declination -
               cos_latitude * cos_declination * cos(M_PI * solar_time / 12.0));

      sun_azimuth = -atan2(cos_declination * sin(M_PI * solar_time / 12.0),
                           -cos_latitude * sin_declination -
                               sin_latitude * cos_declination *
                                   cos(M_PI * solar_time / 12.0));
#else
      sun_altitude = M_PI * 0.5 - asin(sin_latitude * sin_declination -
                                       cos_latitude * cos_declination *
                                           cos(M_PI * solar_time / 12.0));

      sun_azimuth = atan2(
          (-cos_declination * sin(M_PI * solar_time / 12.0)),
          (cos_latitude * sin_declination -
           sin_latitude * cos_declination * cos(M_PI * solar_time / 12.0)));
#endif

      // determine the light intensity coming from the sky
      for (i = 0; i < AZIMUTH_SAMPLES; i++) {
        azi_sample =
            (((double)i) * 2.0 * M_PI) / ((double)(AZIMUTH_SAMPLES)) - M_PI;
        for (j = 0; j < ALTITUDE_SAMPLES; j++) {
#if NEW_SKY_MODEL == 0
          alt_sample = ((double)j) * M_PI * 0.5 / ((double)(ALTITUDE_SAMPLES));
#else
          // adjust for new zenith luminace:
          alt_sample = M_PI * 0.5 -
                       ((double)j) * M_PI * 0.5 / ((double)(ALTITUDE_SAMPLES));
#endif

          // check if it is a clear day or overcast day
          if (clear_days > 0.0)
            max_intensity =
                ClearSky(alt_sample, azi_sample, sun_altitude, sun_azimuth);
          else
            max_intensity = OverCastSky(alt_sample, sun_altitude);

          intensity[i][j] += max_intensity > 0.0 ? max_intensity : 0.0;
          sum_intensity += max_intensity > 0.0 ? max_intensity : 0.0;
        }
      }
    }
  }

  // normalize the luminance distribution of the sky and save max luminance for
  // glDisplay
  max_intensity = 0.0;
  if (sum_intensity > 0.0)
    for (i = 0; i < AZIMUTH_SAMPLES; i++)
      for (j = 0; j < ALTITUDE_SAMPLES; j++) {
        intensity[i][j] = intensity[i][j] / sum_intensity;
        if (intensity[i][j] > max_intensity)
          max_intensity = intensity[i][j];
      }
  max_sky_intensity = (float)max_intensity;

  // FOR COMPARISON, output a sky file
  // skyfile = fopen("sky.dat", "w");
  // fprintf (skyfile, "%d %d\n", AZIMUTH_SAMPLES, ALTITUDE_SAMPLES);
  // for (i = 0; i < AZIMUTH_SAMPLES; i++)
  //{
  //  azi_sample = (((double) i) * 2.0*M_PI)/((double)(AZIMUTH_SAMPLES)) - M_PI;
  //  // QQQ: generate Turtle sky // max_intensity = 1.0;
  //  for (j = 0; j < ALTITUDE_SAMPLES; j++)
  //  {
  //    alt_sample = ((double) j) * M_PI*0.5/((double)(ALTITUDE_SAMPLES));
  //    fprintf (skyfile, "%g %g %g\n", azi_sample, alt_sample,
  //    intensity[i][j]);
  //    // QQQ: generate Turtle sky // fprintf (skyfile, "%g %g %g\n",
  //    azi_sample, alt_sample, max_intensity);
  //    // QQQ: generate Turtle sky // max_intensity -= 1.0/52.0;
  //  }
  //  fprintf (skyfile, "\n");
  //}
  // fclose(skyfile);

  return;
}

#if NEW_SKY_MODEL == 1
/* -------------------------------------------------------------------------- */

double ZenithLuminace(double sun_alt)
// return zenith luminance value in K cd m^{-2}
{
  double chi = (4.0 / 9.0 - turbidity / 120.0) * (M_PI - 2.0 * sun_alt);
  double Lz =
      (4.0453 * turbidity - 4.9710) * tan(chi) - 0.2155 * turbidity + 2.4192;
  return Lz;
}
/* -------------------------------------------------------------------------- */

double ClearSky(double alt, double azi, double sun_alt, double sun_azi)
/* alt, azi are the altitude and azimuth of direction V in sky dome
 * sun_alt, sun_azi are the altitude and azimuth of sun's direction */
{
  double Lz, L, ex, normfactor;
  double gamma, cosgamma, cos_alt, cos_sun_alt;

  Lz = ZenithLuminace(sun_alt);

  // calculation of cosgamma uses dot product of the two vectors
  // for sun position and direction V on sky dome
  // then it is simplified using product-to-sum trig identities
  // double sundir_x = sin(sun_alt) * cos(sun_azi);
  // double sundir_y = sin(sun_alt) * sin(sun_azi);
  // double sundir_z = cos(sun_alt);
  // double Dx = sin(alt)*cos(azi);
  // double Dy = sin(alt)*sin(azi);
  // double Dz = cos(alt);
  // cosgamma = Dx*sundir_x + Dy*sundir_y + Dz*sundir_z;

  cos_sun_alt = cos(sun_alt);
  cos_alt = cos(alt);
  cosgamma =
      sin(alt) * sin(sun_alt) * sin(azi - sun_azi) + cos_alt * cos_sun_alt;
  gamma = acos(cosgamma);

  // alt in [0,pi/2], so cos_alt is in [1,0]
  if (cos_alt > 0.01)
    ex = 1.0 - exp(-0.32 / cos_alt);
  else
    ex = 1.0; // 1.0 - exp(-32) = 1.0 - 1.3e-14

  normfactor = 0.273850963 * (0.91 + 10.0 * exp(-3.0 * sun_alt) +
                              0.45 * cos_sun_alt * cos_sun_alt);

  L = Lz * (0.91 + 10.0 * exp(-3.0 * gamma) + 0.45 * cosgamma * cosgamma) *
      (ex / normfactor);

  return (L);
}

/* -------------------------------------------------------------------------- */

double OverCastSky(double alt, double sun_alt) {
  return (ZenithLuminace(sun_alt) * (1.0 + 2.0 * cos(alt)) / 3.0);
}

/* -------------------------------------------------------------------------- */
#else

// Old ClearSky function that used a poor formula for Lz, giving sharp spikes
// of luminace values at the zenith. in particular, when latitute was in
// [-23,23] As a patch, I've added a clamp for sun altitude close to pi/2
double ClearSky(double alt, double azi, double sun_alt, double sun_azi)
/* alt, azi are the altitude and azimuth of direction V in sky dome
 * sun_alt, sun_azi are the altitude and azimuth of sun's direction */
{
  const double MAX_SUN_ALT = M_PI * 0.5 * 0.99;
  const double TAN_MAX_SUN_ALT = tan(MAX_SUN_ALT);
  double Lz, L, ex, normfactor;
  double gamma, cosgamma, sin_alt, sin_sun_alt;

  sin_sun_alt = sin(sun_alt);
  sin_alt = sin(alt);

  // avoid large values of tan(sun_alt) when sun_alt approaches pi/2
  if (sun_alt < MAX_SUN_ALT)
    Lz = ((1.376 * turbidity - 1.81) * tan(sun_alt) + 0.38) * 1000.0 / 203.0;
  else
    Lz = ((1.376 * turbidity - 1.81) * TAN_MAX_SUN_ALT + 0.38) * 1000.0 / 203.0;

  /* calculation of cosgamma is exactly same as using dot product of the two
   * vectors for sun position and direction V on sky dome (simplified using
   * product-to-sum trig identities) */
  /* CODE for dot product of vectors:
    double sundir_x = -sin(sun_azi) * cos(sun_alt);
    double sundir_y = -cos(sun_azi) * cos(sun_alt);
    double sundir_z = sin(sun_alt);

    double Dx = -sin(azi)*cos(alt);
    double Dy = -cos(azi)*cos(alt);
    double Dz = sin(alt);

    cosgamma = Dx*sundir_x + Dy*sundir_y + Dz*sundir_z;
  */
  cosgamma =
      cos(alt) * cos(sun_alt) * cos(azi - sun_azi) + sin_alt * sin_sun_alt;

  cosgamma = cosgamma < 0.0 ? 0.0 : cosgamma;
  cosgamma = cosgamma > 1.0 ? 1.0 : cosgamma;
  gamma = acos(cosgamma);

  if (sin_alt > 0.01)
    ex = 1.0 - exp(-0.32 / sin_alt);
  else
    ex = 1.0; // 1.0 - exp(-32) = 1.0 - 1.3e-14

  normfactor = 0.274 * (0.91 + 10.0 * exp(-3.0 * (M_PI * 0.5 - sun_alt)) +
                        0.45 * sin_sun_alt * sin_sun_alt);
  L = Lz * (0.91 + 10.0 * exp(-3.0 * gamma) + 0.45 * cosgamma * cosgamma) *
      (ex / normfactor);

  return (L);
}

/* -------------------------------------------------------------------------- */

double OverCastSky(double alt, double sun_alt) {
  double zenith_overcast = (8.6 * sin(sun_alt) + 0.123) * 1000.0 / 203.0;
  return (zenith_overcast * (1.0 + 2.0 * sin(alt)) / 3.0);
}
#endif
