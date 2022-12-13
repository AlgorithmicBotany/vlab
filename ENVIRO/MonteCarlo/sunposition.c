/* ++++++++++++++++++++++++++++++++++++++++++++++++++ */
/* sunposition.c */
/* To get the apparent course of the sun in the sky */
/* ++++++++++++++++++++++++++++++++++++++++++++++++++ */

/* following the working paper No. 162 of B. K. P. Horn (March 1978) */
/*
Note : Conversion from Dayligth Saving Time to Central European Time
       (if necessary) must be provided by the user,
       since time zones are not following meridians but are irregularly
       drawn, and thus reliable conversion cannot be computed by routine.
*/
/* Original java code by Rafael, March 1994
   WWW http://kogs-www.informatik.uni-hamburg.de/~wiemker/
   e-mail: wiemker@informatik.uni-hamburg.de */
/* Modified by Radomir Mech, October 96
   Example (Calgary Oct 7,96)
   sunposition 51.05 -114.05 96 10 7 7:28:44 +7
*/

#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/***************************************************************************/
/* Computes sun position (as altitude in deg above horizon and azimuth in
   degrees north-to-east) for a given langitude "lan" (in degrees + east,
   - west), longitude "lon", year, day, and time (CET Central European time).
*/
void GetSunPosition(double lat, double lon, int year, int month, int day,
                    double time, double *altitude, double *azimuth) {
  double gamma, mean_anomaly, ecc, ecc_anomaly, lambda, true_anomaly;
  double r, dec, ra, gha, gha_mod, a, e, lat_sun, lon_sun;
  double f, l, jd, t, west, south;
  int i;

  /* constants */
  double deg_to_rad = M_PI / 180.; /* conversion deg / radians */

  lat = lat * deg_to_rad; /* convert */
  lon = lon * deg_to_rad;

  f = -floor((14.0 - (double)month) / 12.0);
  l = f + year + 4800;
  jd = floor((367.0 * ((double)month - 2.0 - f * 12.)) / 12.);

  jd = jd - floor(3. * (1. + floor(l / 100.)) / 4.);
  jd = jd + floor(1461. * l / 4.) + (int)day - 32075;

  /* JD  is verified with IDL JULDAY routine */

  jd = jd - 2442414; /* counted from January 1, 1975 */

  /* t: UT in units of day */
  t = time / 3600.0 / 24.0 - jd - 1.0 / 24.0;
  /*  course starts at midnight: 00 h 00' , */
  /*  and  UT is minus 1 hour*/

  /* mean longitude of sun's perigee : */
  gamma = (282.5105 + 0.00004709 * t) * deg_to_rad; /* almost constant */

  mean_anomaly = (357.5166 + 0.98560026 * t) * deg_to_rad;
  /* ca. one degree per day */
  ecc = 0.016720 - 0.0000000011 * t; /* almost constant */

  ecc_anomaly =
      2. * atan(sqrt((1. + ecc) / (1. - ecc)) * tan(mean_anomaly / 2.));
  /* is a good starting value */
  /* now iterate to solve transcendental equation */
  for (i = 1; i < 10; i++) {
    ecc_anomaly = mean_anomaly + ecc * sin(ecc_anomaly);
  }

  true_anomaly =
      2. * atan(sqrt((1. + ecc) / (1. - ecc)) * tan(ecc_anomaly / 2.));
  lambda = gamma + true_anomaly; /* true */

  dec = asin(sin(lambda) *
             sin(23.44 * deg_to_rad)); /* earth axis assumed fixed */
  ra = atan(tan(lambda) *
            cos(23.44 * deg_to_rad)); /* luni solar precession neglected */
  /* Quadrant Ambiguity of tangens */
  /* Math.cos(lambda) has sign of Math.cos(a) */
  /* if necessary push a to second branch of tangens  */
  if (cos(lambda) < 0.)
    ra += M_PI;

  gha = (100.0215 + 360.98564734 * t) *
        deg_to_rad; /* ca. one revolution per day */

  lat_sun = dec;
  lon_sun = ra - gha;

  e = asin(sin(lat) * sin(lat_sun) +
           cos(lat) * cos(lat_sun) * cos(lon_sun - lon));
  west = cos(lat_sun) * sin(lon_sun - lon);
  south =
      (-cos(lat) * sin(lat_sun) + sin(lat) * cos(lat_sun) * cos(lon_sun - lon));

  a = atan(west / south);

  /* Quadrant Ambiguity of tangens : south or north ? */
  /* if necessary push a to second branch of tangens */
  if (south >= 0.)
    a = M_PI - a;
  if (south < 0.)
    a = -a;
  /* avoid negative azimuth, count [0..360]  */
  if (a < 0.)
    a += 2 * M_PI;

  /* t holds UT in units of day */

  *altitude = e / deg_to_rad;
  *azimuth = a / deg_to_rad;
}

/****************************************************************************/
#ifdef STANDALONE

/* to compile as a separate program:
   cc -o sunposition -DSTANDALONE sunposition.c -lm
   */
#include <string.h>
#include <stdio.h>

void main(int argc, char **argv) {
  double lon, lat, altitude, azimuth;
  int year, month, day, time, timediff;
  char *ptr, *ptr2;

  if (argc < 7) {
    fprintf(stderr,
            "Usage: sunposition latitude longitude year month day time"
            " timediff\n"
            "     latitude and longitude are in degrees (+ east, - west)\n"
            "     year is given either as YY or YYYY\n"
            "     time is specified as hh:mm:ss or just in seconds\n"
            "     timediff is difference in hours from CET (Central European "
            "time)\n");
    exit(-1);
  }

  lat = atof(argv[1]);
  lon = atof(argv[2]);
  year = atoi(argv[3]);

  if (year < 100)
    year += 1900;

  month = atoi(argv[4]);
  day = atoi(argv[5]);

  time = 0;
  if ((ptr = strchr(argv[6], ':')) != NULL) {
    *ptr = 0;
    time = atoi(argv[6]);

    if ((ptr2 = strchr(++ptr, ':')) != NULL) {
      *ptr2 = 0;

      time = 3600 * time + atoi(ptr) * 60 + atoi(ptr2 + 1);
    } else
      /* just minutes and seconds */
      time = 60 * time + atoi(ptr);
  } else
    time = atoi(argv[6]);

  /* difference from CET */
  if (argc > 7)
    timediff = atoi(argv[7]);

  GetSunPosition(lat, lon, year, month, day, time + timediff * 3600, &altitude,
                 &azimuth);

  fprintf(stderr,
          "Sun postion at %g degrees latitude and %g degrees of longitude\n"
          "at %d:%d:%d (CET+%dh) on %4d/%02d/%02d is\n"
          "%.2f degrees altitude and %.2f degrees azimuth\n",
          lat, lon, (int)(time / 3600.0), (int)((time % 3600) / 60.0),
          time % 60, timediff, year, month, day, altitude, azimuth);
}

#endif
