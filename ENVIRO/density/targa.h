
/* TYPE OF TARGA FILES */
#define TGA_NOIMAGE 0        /* no image data included */
#define TGA_MAPPED 1         /* uncompressed, color-mapped images */
#define TGA_TRUECOLOR 2      /* uncompressed, RGB images  */
#define TGA_GRAY 3           /* uncompressed, black and white images */
#define TGA_MAPPED_RLE 9     /* runlength encoded color-mapped images */
#define TGA_TRUECOLOR_RLE 10 /* runlength encoded RGB images  */

typedef unsigned char byte;

struct targa_params_type {
  FILE *fp;
  byte type;
  int Xorigin, Yorigin;
  int Xres, Yres;
  char *id_field;
  char *colormap; /* pointer to a field of byte triplets RGB */
  int first_cmap_entry, num_cmap_entries;

  /* internal */
  int current_row;
  signed char orientation; /* plus minus 1 */
  char packet_type;
  int items_left;
  byte packet_color[3];
};
typedef struct targa_params_type targa_params_type;

/*** prototypes ***/
int saveTGAhead(targa_params_type *spec);
/* row contains Xres triples of bytes of blue, green, and red */
int saveTGArow(targa_params_type *spec, int y, byte *row);
int saveTGAfinish(targa_params_type *spec);

int loadTGAhead(char *filename, targa_params_type *spec);
/* row contains Xres triples of bytes of blue, green, and red */
int loadTGArow(targa_params_type *spec, int y, byte *row);
int loadTGAfinish(targa_params_type *spec);
