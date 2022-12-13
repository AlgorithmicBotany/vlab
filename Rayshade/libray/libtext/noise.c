/*
 * noise.c
 *
 * Copyright (C) 1989, 1991, Robert Skinner, Craig E. Kolb
 * All rights reserved.
 *
 * This software may be freely copied, modified, and redistributed
 * provided that this copyright notice is preserved on all copies.
 *
 * You may not distribute this software, in whole or in part, as part of
 * any commercial product without the express consent of the authors.
 *
 * There is no warranty or other guarantee of fitness of this software
 * for any purpose.  It is provided solely "as is".
 *
 *
 */
#include "libcommon/common.h"

#define MINX -1000000
#define MINY MINX
#define MINZ MINX

#define SCURVE(a) ((a) * (a) * (3.0 - 2.0 * (a)))
#define REALSCALE (2.0 / 65536.0)
#define NREALSCALE (2.0 / 4096.0)
#define Hash3d(a, b, c)                                                        \
  hashTable[hashTable[hashTable[(a)&0xfff] ^ ((b)&0xfff)] ^ ((c)&0xfff)]
#define Hash(a, b, c)                                                          \
  (xtab[(xtab[(xtab[(a)&0xff] ^ (b)) & 0xff] ^ (c)) & 0xff] & 0xff)

#define INCRSUM(m, s, x, y, z)                                                 \
  ((s) * (RTable[m] * 0.5 + RTable[m + 1] * (x) + RTable[m + 2] * (y) +        \
          RTable[m + 3] * (z)))

#define MAXSIZE 267

Float RTable[MAXSIZE];
static short *hashTable;
static int R(), Crc16();

static unsigned short xtab[256] = {
    0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241, 0xc601,
    0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440, 0xcc01, 0x0cc0,
    0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40, 0x0a00, 0xcac1, 0xcb81,
    0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841, 0xd801, 0x18c0, 0x1980, 0xd941,
    0x1b00, 0xdbc1, 0xda81, 0x1a40, 0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01,
    0x1dc0, 0x1c80, 0xdc41, 0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0,
    0x1680, 0xd641, 0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081,
    0x1040, 0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
    0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441, 0x3c00,
    0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41, 0xfa01, 0x3ac0,
    0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840, 0x2800, 0xe8c1, 0xe981,
    0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41, 0xee01, 0x2ec0, 0x2f80, 0xef41,
    0x2d00, 0xedc1, 0xec81, 0x2c40, 0xe401, 0x24c0, 0x2580, 0xe541, 0x2700,
    0xe7c1, 0xe681, 0x2640, 0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0,
    0x2080, 0xe041, 0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281,
    0x6240, 0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
    0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41, 0xaa01,
    0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840, 0x7800, 0xb8c1,
    0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41, 0xbe01, 0x7ec0, 0x7f80,
    0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40, 0xb401, 0x74c0, 0x7580, 0xb541,
    0x7700, 0xb7c1, 0xb681, 0x7640, 0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101,
    0x71c0, 0x7080, 0xb041, 0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0,
    0x5280, 0x9241, 0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481,
    0x5440, 0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
    0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841, 0x8801,
    0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40, 0x4e00, 0x8ec1,
    0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41, 0x4400, 0x84c1, 0x8581,
    0x4540, 0x8701, 0x47c0, 0x4680, 0x8641, 0x8201, 0x42c0, 0x4380, 0x8341,
    0x4100, 0x81c1, 0x8081, 0x4040};

Float Chaos(), Marble();

void InitTextureTable() {
  int i, j, temp;

  seednrand(1);
  hashTable = (short *)Malloc(4096 * sizeof(short int));
  for (i = 0; i < 4096; i++)
    hashTable[i] = i;
  for (i = 4095; i > 0; i--) {
    j = (int)(nrand() * 4096);
    temp = hashTable[i];
    hashTable[i] = hashTable[j];
    hashTable[j] = temp;
  }
}

void NoiseInit() {
  int i;
  Vector rp;

  InitTextureTable();

  for (i = 0; i < MAXSIZE; i++) {
    rp.x = rp.y = rp.z = (Float)i;
    RTable[i] = R(&rp) * REALSCALE - 1.0;
  }
}

static int R(v) Vector *v;
{
  v->x *= .12345;
  v->y *= .12345;
  v->z *= .12345;

  return Crc16((char *)v, sizeof(Vector));
}

/*
 * Note that passing a Float to Crc16 and interpreting it as
 * an array of chars means that machines with different floating-point
 * representation schemes will evaluate Noise(point) differently.
 */
static int Crc16(buf, count) register char *buf;
register int count;
{
  register unsigned int crc = 0;

  while (count--)
    crc = (crc >> 8) ^ xtab[(unsigned char)(crc ^ *buf++)];

  return crc;
}

/*
 * Robert Skinner's Perlin-style "Noise" function
 */
Float Noise3(point) Vector *point;
{
  register int ix, iy, iz, jx, jy, jz;
  Float x, y, z;
  Float sx, sy, sz, tx, ty, tz;
  Float sum;
  short m;

  /* ensures the values are positive. */
  x = point->x - MINX;
  y = point->y - MINY;
  z = point->z - MINZ;

  /* its equivalent integer lattice point. */
  ix = (int)x;
  iy = (int)y;
  iz = (int)z;
  jx = ix + 1;
  jy = iy + 1;
  jz = iz + 1;

  sx = SCURVE(x - ix);
  sy = SCURVE(y - iy);
  sz = SCURVE(z - iz);

  /* the complement values of sx,sy,sz */
  tx = 1.0 - sx;
  ty = 1.0 - sy;
  tz = 1.0 - sz;

  /*
   *  interpolate!
   */
  m = Hash3d(ix, iy, iz) & 0xFF;
  sum = INCRSUM(m, (tx * ty * tz), (x - ix), (y - iy), (z - iz));

  m = Hash3d(jx, iy, iz) & 0xFF;
  sum += INCRSUM(m, (sx * ty * tz), (x - jx), (y - iy), (z - iz));

  m = Hash3d(ix, jy, iz) & 0xFF;
  sum += INCRSUM(m, (tx * sy * tz), (x - ix), (y - jy), (z - iz));

  m = Hash3d(jx, jy, iz) & 0xFF;
  sum += INCRSUM(m, (sx * sy * tz), (x - jx), (y - jy), (z - iz));

  m = Hash3d(ix, iy, jz) & 0xFF;
  sum += INCRSUM(m, (tx * ty * sz), (x - ix), (y - iy), (z - jz));

  m = Hash3d(jx, iy, jz) & 0xFF;
  sum += INCRSUM(m, (sx * ty * sz), (x - jx), (y - iy), (z - jz));

  m = Hash3d(ix, jy, jz) & 0xFF;
  sum += INCRSUM(m, (tx * sy * sz), (x - ix), (y - jy), (z - jz));

  m = Hash3d(jx, jy, jz) & 0xFF;
  sum += INCRSUM(m, (sx * sy * sz), (x - jx), (y - jy), (z - jz));

  return sum;
}

/*
 * Vector-valued "Noise"
 */
void DNoise3(point, result) Vector *point, *result;
{
  register int ix, iy, iz, jx, jy, jz;
  Float x, y, z;
  Float px, py, pz, s;
  Float sx, sy, sz, tx, ty, tz;
  short m;

  /* ensures the values are positive. */
  x = point->x - MINX;
  y = point->y - MINY;
  z = point->z - MINZ;

  /* its equivalent integer lattice point. */
  ix = (int)x;
  iy = (int)y;
  iz = (int)z;
  jx = ix + 1;
  jy = iy + 1;
  jz = iz + 1;

  sx = SCURVE(x - ix);
  sy = SCURVE(y - iy);
  sz = SCURVE(z - iz);

  /* the complement values of sx,sy,sz */
  tx = 1.0 - sx;
  ty = 1.0 - sy;
  tz = 1.0 - sz;

  /*
   *  interpolate!
   */
  m = Hash3d(ix, iy, iz) & 0xFF;
  px = x - ix;
  py = y - iy;
  pz = z - iz;
  s = tx * ty * tz;
  result->x = INCRSUM(m, s, px, py, pz);
  result->y = INCRSUM(m + 4, s, px, py, pz);
  result->z = INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(jx, iy, iz) & 0xFF;
  px = x - jx;
  s = sx * ty * tz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(jx, jy, iz) & 0xFF;
  py = y - jy;
  s = sx * sy * tz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(ix, jy, iz) & 0xFF;
  px = x - ix;
  s = tx * sy * tz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(ix, jy, jz) & 0xFF;
  pz = z - jz;
  s = tx * sy * sz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(jx, jy, jz) & 0xFF;
  px = x - jx;
  s = sx * sy * sz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(jx, iy, jz) & 0xFF;
  py = y - iy;
  s = sx * ty * sz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);

  m = Hash3d(ix, iy, jz) & 0xFF;
  px = x - ix;
  s = tx * ty * sz;
  result->x += INCRSUM(m, s, px, py, pz);
  result->y += INCRSUM(m + 4, s, px, py, pz);
  result->z += INCRSUM(m + 8, s, px, py, pz);
}
