#include <math.h>
#include <algorithm>

#ifndef LIGHT_HPP
#define LIGHT_HPP

/*********************************************************************************************
=	LightModel Environment Class			Kipp Horel, Feb 5th 2009
=
=	This class holds a 3 Dimensional voxelized model of approximate Light
conditions, represented as floats between 0 and 1.
=
=	Constructor takes 2 arguments: LightModel(# of Divisions (int) , Span
(float) ); =	the represented space will be centered along X and Z axes, so
unless an ODD number of Divisions is given, =	the origin will sit on the fence
between 2 voxels.
=
=	void initExposureMap( )			- initializes each voxel with a
random value between 0.9 and 1, which adds stochasticity and creates non-zero
gradients. =	Float getContinuousExposure( V3f )	- returns exposure value
in the voxel which contains or is closest to the given world position.
=	void shadow3D( V3f, bool )		- when bool=true, reduces light
exposure in voxels near the given world position. if bool=false, it reverses
this effect. =	V3f getContinuousEnvDir( V3f pos, float rad )
=						- returns 3D vector representing
direction and strength of the relative gradient at the given world position.
=
*********************************************************************************************/

class LightModel {
  int VOXEL_DENSITY; // # of voxels in each dimension, total # of voxels =
                     // DENSITY^3.
  float SPAN; // Amount of world space to be represented by the voxel set. size
              // of an individual voxel = SPAN / DENSITY
  float WORLDtoVOX;  // this a conversion factor for turning world coordinates
                     // into voxel indexes.
  float ***exposure; // Private pointer which will give this class access to
                     // it's dynamically created set of voxels.
  float SPREAD; // A higher spread value will produce a wider shadow, and lower
                // = narrower. Must be > 0.
  int DEPTH;    // max propagation depth in voxels
  float C1,
      C2; // propagation constants, C1/(C2+distance) is the shadowing factor.
  float RAN, VERT; // Constants controlling the random factor and Vertical bias
                   // during light field initialization.

public:
  LightModel(int d, float s) {
    VOXEL_DENSITY = d;
    SPAN = s;
    WORLDtoVOX = ((float)(VOXEL_DENSITY - 1) /
                  (SPAN * 2)); // this a conversion factor for turning world
                               // coordinates into voxel indexes.

    exposure = new float *
        *[d]; // link main ptr to array of ptr-ptr's.	(one axis down, 2 to go)
    for (int i = 0; i < d; i++) {
      *(exposure + i) =
          new float *[d]; // link each ptr-ptr to an array of ptr's. (now we
                          // have a 2D grid of pointers)
      for (int j = 0; j < d; j++)
        exposure[i][j] = new float[d]; // link each ptr to an array of floats.
    } // (now we have a dynamically allocated 3D array of floats)
  }

  ~LightModel() // MC Feb 2015 - added memory deallocation
  {
    for (int i = 0; i < VOXEL_DENSITY; i++) {
      for (int j = 0; j < VOXEL_DENSITY; j++)
        delete[] exposure[i][j];
      delete[] exposure[i];
    }
    delete[] exposure;
  }

  void setParam(float sp, int dp, float ca, float cb, float r, float v) {
    SPREAD = sp; // setting the most important model parameters is done with
                 // this function.
    DEPTH = dp;  // these parameters are described in more depth above.
    C1 = ca;
    C2 = cb;
    RAN = r;
    VERT = v;
  }

  void initExposureMap() {
    //		Printf("Initializing exposure map\n");
    for (int k = 0; k < VOXEL_DENSITY;
         k++) // voxels can be initialized with varying levels of randomization
              // and depth based bias.
    {
      for (int i = 0; i < VOXEL_DENSITY; i++) {
        for (int j = 1; j + 1 < VOXEL_DENSITY; j++) {
          exposure[i][j][k] = (1.0f - (RAN + VERT)) + ran(RAN) +
                              (VERT * (float)j) / (float)VOXEL_DENSITY;
        }
        // MC Feb 2015 - why would the lower and upper boundary be completely
        // shaded if there is nothing in the exposure map? These should be
        // parameters and not hard coded, so I've set the values to be not
        // shaded
        exposure[i][0][k] =
            1.f; // the "floor", or lower boundary has lower light quality.
        exposure[i][VOXEL_DENSITY - 1][k] =
            1.0f; // the "roof", or upper boundary is completely shaded.
      }
    }
  }

  void getVoxelIndex(V3f pos, int &x, int &y,
                     int &z) // returns the index of the closest exposure voxel.
                             // starting point for the shadow algorithm.
  {
    y = (pos.y * WORLDtoVOX); // y is indexed only in the positive domain.
    x = (pos.x * WORLDtoVOX + ((float)VOXEL_DENSITY) / 2.0f); //
    z = (pos.z * WORLDtoVOX +
         ((float)VOXEL_DENSITY) /
             2.0f); // the X and Z dimensions both need an offset, to center the
                    // space around the origin.
    x = std::max(std::min(x, VOXEL_DENSITY - 1), 0);
    y = std::max(std::min(y, VOXEL_DENSITY - 1),
                 0); // Since the indexes are restricted in each dimension to
                     // fall within the acceptable range
    z = std::max(std::min(z, VOXEL_DENSITY - 1),
                 0); // this process will return the closest valid voxel even if
                     // the world position is out of range.
  }

  void getCornerVoxelIndex(V3f pos, int &x, int &y,
                           int &z) // when interpolating, a 2x2x2 cubic set of
                                   // neighboring voxels is considered.
  { // this function is much the same as above, except modifed to return indexes
    y = (pos.y * WORLDtoVOX); // to the top-left-front member of this set. by
                              // adding 1 to each dimension in combination,
    x = (pos.x * WORLDtoVOX + ((float)VOXEL_DENSITY - 1) /
                                  2.0f); // all 8 exposure values will combined
                                         // to find a weighted average.
    z = (pos.z * WORLDtoVOX + ((float)VOXEL_DENSITY - 1) / 2.0f);
    x = std::max(std::min(x, VOXEL_DENSITY - 2), 0);
    y = std::max(std::min(y, VOXEL_DENSITY - 2), 0);
    z = std::max(std::min(z, VOXEL_DENSITY - 2), 0);
  }

  float
  getContinuousExposure(V3f pos) // returns exposure value @ world coordinates.
  {
    int x, y, z;
    getCornerVoxelIndex(
        pos, x, y,
        z); // this conversion from world coordinates to voxel indexes enforces
            // array boundaries, and points us to the set of 8 surrounding data
            // points for exposure interpolation.
    float U(pos.x * WORLDtoVOX);
    U -= floor(U); // we convert to voxel space, and remove the Integer portion
                   // in each dimension, leaving only a remainder between 0-1.
    float V(pos.y * WORLDtoVOX); // this gives a set of U V W weightings which
                                 // in combination describe the proximity to
                                 // each of the 8 voxels.
    V -= floor(V);
    float W(pos.z * WORLDtoVOX);
    W -= floor(W);

    float interpolated = 0; // INTERPOLATION:
                            // if U=1.0, V=1.0, W=1.0,  1.0x1.0x1.0, adds
                            // 0.000*exposure[x][y][z] (far corner no effect)
    interpolated +=
        (1.0f - U) * (1.0f - V) * (1.0f - W) *
        exposure[x][y][z]; // if U=0.1, V=0.1, W=0.1,  0.9x0.9x0.9, adds
                           // 0.729*exposure[x][y][z] (very close full effect)
    interpolated += (U) * (1.0f - V) * (1.0f - W) * exposure[x + 1][y][z];
    interpolated += (1.0f - U) * (V) * (1.0f - W) * exposure[x][y + 1][z];
    interpolated += (U) * (V) * (1.0f - W) * exposure[x + 1][y + 1][z];
    // since these 8 combined weights sum to 1.0,
    interpolated +=
        (1.0f - U) * (1.0f - V) *
        (W)*exposure[x][y]
                    [z + 1]; // by weighted combinination of all 8 members,
    interpolated += (U) * (1.0f - V) * (W)*exposure[x + 1][y][z + 1];
    interpolated += (1.0f - U) * (V) * (W)*exposure[x][y + 1][z + 1];
    interpolated += (U) * (V) * (W)*exposure[x + 1][y + 1][z + 1];

    return interpolated; // we can interpolate smoothly across the 2x2x2 set.
  }

  V3f getContinuousEnvDir(
      V3f pos, float rad) // Measuring the gradient of the interpolated field
  {
    V3f result;

    pos.x = std::max(std::min(pos.x, SPAN * 0.9999f),
                     SPAN * -0.9999f); // enforce boundary conditions
    pos.y = std::max(std::min(pos.y, SPAN * 1.9999f), 0.0001f);
    pos.z = std::max(std::min(pos.z, SPAN * 0.9999f), SPAN * -0.9999f);
    float xa(std::max(pos.x - rad, SPAN * -0.9999f)),
        xb(std::min(pos.x + rad,
                    SPAN * 0.9999f)); // Each dimension is probed a given
                                      // distance in each direction.
    float ya(std::max(pos.y - rad, 0.0001f)),
        yb(std::min(
            pos.y + rad,
            SPAN * 1.9999f)); // the locations for sampling of the environment
    float za(std::max(pos.z - rad, SPAN * -0.9999f)),
        zb(std::min(pos.z + rad,
                    SPAN * 0.9999f)); // are determined here, and boundary
                                      // conditions are enforced.

    result.x += getContinuousExposure(
        V3f(xb, pos.y, pos.z)); // the gradient approximation is calculated
    result.x -= getContinuousExposure(V3f(xa, pos.y, pos.z));
    result.y += getContinuousExposure(V3f(
        pos.x, yb,
        pos.z)); // by balancing two opposing measurements against each other
    result.y -= getContinuousExposure(V3f(pos.x, ya, pos.z));
    result.z +=
        getContinuousExposure(V3f(pos.x, pos.y, zb)); // along each axis.
    result.z -= getContinuousExposure(V3f(pos.x, pos.y, za));

    return result;
  }

  void shadow3D(
      V3f pos,
      bool sub) // This function adds or subtracts shadow from the environment.
  {
    float dist;
    int x, y, z, xb, yb, zb;
    getVoxelIndex(pos, x, y, z); // get our voxel indexes
    int max(DEPTH), min(0);

    if (sub)
      for (float i = min; i < max;
           i++) // if sub=true, we will be DECREASING light values.
      {
        int n(i * SPREAD);
        for (float j = -n; j <= n; j++)
          for (float k = -n; k <= n; k++) {

            dist = sqrt(i * i + j * j + k * k);
            xb = x + j;
            yb = y - i;
            zb =
                z +
                k; // xb, yb, zb are now the indexes of the voxel being updated;
            if (xb >= 0 && xb < VOXEL_DENSITY && yb >= 0 &&
                yb < VOXEL_DENSITY && zb >= 0 &&
                zb < VOXEL_DENSITY) // if it is within boundaries,
            {
              // MC Feb 2015 - added check for negative values of C1 *
              // pow(C2,-dist)
              float s = C1 * pow(C2, -dist);
              if (s > 1.0)
                s = 1.0;
              exposure[xb][yb][zb] *=
                  (1.0 - s); // modify the exposure value by a factor determined
                             // by distance from x,y,z
            }
          }
      }
    else
      for (float i = min; i < max;
           i++) // this alternative loop is almost identical,
      { // except that it INCREASES light values, negating the effect of the
        // above process.
        int n(i * SPREAD);
        for (float j = -n; j <= n; j++)
          for (float k = -n; k <= n; k++) {
            dist = sqrt(i * i + j * j + k * k);
            xb = x + j;
            yb = y - i;
            zb = z + k;
            if (xb >= 0 && xb < VOXEL_DENSITY && yb >= 0 &&
                yb < VOXEL_DENSITY && zb >= 0 && zb < VOXEL_DENSITY) {
              float s = C1 * pow(C2, -dist);
              if (s > 1.0)
                s = 1.0;
              exposure[xb][yb][zb] /= (1.0 - s);
            }
          }
      }
  }
};

#endif // LIGHT_HPP
