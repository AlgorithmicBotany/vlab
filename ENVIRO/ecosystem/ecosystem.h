#define MAX_LINE_LEN ((255 * 5 + 3) * 2) /* max lenght of received substring   \
                                          */
#define MAX_PARAM 12 /* max number of parameters of comm. symbol */

/* queries to be answered */
struct item_type {
  float position[3];
  float radius;
  float vigor;
  char removed;
  int range[3][2]; /* range in voxels (min,max) for each coordinate */
  int index;
  unsigned long dist;
  int master;
};
typedef struct item_type item_type;
