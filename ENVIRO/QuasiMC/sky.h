/* sky.h - interface for generating rays from the sky model */

#ifndef _SKY_H
#define _SKY_H

#ifdef __cplusplus
extern "C" {
#endif
int InitSkyModel(void);
void FreeSkyModel(void);
float GetSkyDirection(float *dir, float *xvec, float *yvec);
float GetSkyIntensity(float *dir);

void SetLocation(double lati, double longi);
void SetWeather(double clear, double turb);
void SetGrowthPeriod(float period[6]);
void SetJulianDay(int jd);
void SetSkyFile(char *filename);
#ifdef __cplusplus
}
#endif

#endif
