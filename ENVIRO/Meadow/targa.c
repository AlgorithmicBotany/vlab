/* source of the targa documentation:
ftp://ftp.cc.monash.edu.au/pub/graphics.formats/targa.format.Z
*/

#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "targa.h"

#ifdef TEST_MALLOC
#include "test_malloc.h"
#endif

/* packet type */
#define PACKET_RAW 0
#define PACKET_RLE 1

/************************************************************/
static int savebyte(byte c, FILE *fp)
{
  return fwrite(&c, 1, 1, fp);
}

/************************************************************/
static int SavePacket(byte type, byte *row, int count, FILE *fp)
{
  byte control, aux;

  if(count<=0)
    return -1;

  control = (count-1)&0x7f;
  if(type == PACKET_RLE) {
    control |= 0x80;     /* highest bit is 1 */
    count = 1;

    /* I hope this is not a bug of out 'imconv' */
    aux = row[2];
    row[2] = row[0];
    row[0] = aux;
  }

  savebyte(control, fp);

  return fwrite(row, count, 3, fp);
}

/************************************************************/
static int RleCodeRGBRow(byte *row, int Xres, FILE *fp)
{
  int x;
  byte *pb_ptr;
  int pb_x;
  char the_same;
  unsigned short r1,r2,g1,g2,b1,b2;
  byte packet_type;

  x = 0;

  /* store the beginning of a packet */
  pb_ptr = row;
  pb_x = 0;

  b1 = *(row++);
  g1 = *(row++);
  r1 = *(row++);
  x++;
  the_same = 0;
  packet_type = PACKET_RAW;

  while(x<Xres) {
    /* is the end of packet reached? */
    if(x-pb_x == 128) {
      SavePacket(packet_type, pb_ptr, x-pb_x, fp);
      pb_ptr = row;
      pb_x = x;
      the_same = 0;
      packet_type = PACKET_RAW;
      b1=g1=r1=256;
    }

    b2 = *(row++);
    g2 = *(row++);
    r2 = *(row++);
    x++;

    if((b2==b1)&&(g2==g1)&&(r2==r1)) {
      if(packet_type == PACKET_RAW) {
	if(the_same) {
	  /* three in a row */
	  SavePacket(PACKET_RAW, pb_ptr, x-3-pb_x, fp);
	  pb_ptr = row - 3*3;
	  pb_x = x-3;
	  packet_type = PACKET_RLE;
	}
	else
	  the_same = 1;
      }
      
      /* nothing for RLE packet */
    }
    else 
      if(packet_type == PACKET_RAW)
	the_same = 0;
      else {
	/* RLE packet */
	SavePacket(PACKET_RLE, pb_ptr, x-1-pb_x, fp);
	pb_ptr = row - 3*1;
	pb_x = x-1; 
	packet_type = PACKET_RAW;
	the_same = 0;
      }
    
    b1 = b2;
    g1 = g2;
    r1 = r2;
  }
  
  /* the end of line reached */
  SavePacket(packet_type, pb_ptr, x-pb_x, fp);

  return Xres;
}

/************************************************************/
#define min(x,y) ((x)<(y)?(x):(y))

static int RleDecodeRGBRow(byte *row, targa_params_type *spec)
{
  int num, togo, i;
  byte aux;

  togo = spec->Xres;

  while(togo > 0) {
    if(spec->items_left > 0) {
      /* finish a packet */
      num = min(togo, spec->items_left);
      spec->items_left -= num;
      togo -= num;

      switch(spec->packet_type) {
      case PACKET_RLE:
	for(i=0;i<num;i++) {
	  *(row++) = spec->packet_color[2]; 
	  *(row++) = spec->packet_color[1];  /* green */
	  *(row++) = spec->packet_color[0];
	}
	break;

      case PACKET_RAW:
	fread(row, num, 3, spec->fp);
	row += num*3;
	break;
      }
    }
    else {
      /* start a packet */
      fread(&aux,1,1,spec->fp);
      spec->items_left = (int)(aux&0x7f) + 1;

      if(aux & 0x80) {
	spec->packet_type = PACKET_RLE;
	fread(spec->packet_color, 1, 3, spec->fp);
      }
      else
	spec->packet_type = PACKET_RAW;
    }
  }

  return 1;
}

/************************************************************/
int saveTGAfinish(targa_params_type *spec)
{
  fclose(spec->fp);
}

/************************************************************/
int saveTGArow(targa_params_type *spec, int y, byte *row)
{
  if(spec->current_row != y) {
    fprintf(stderr, 
	    "Targa error - row number %d doesn't correspond to the "
	    "current row %d!\n", y, spec->current_row);
    return -1;
  }

  if((spec->current_row<0)||(spec->current_row >= spec->Yres)) {
    fprintf(stderr,"Targa error - row is out of the given Yres!\n");
    return -1;
  }

  spec->current_row += spec->orientation;

  switch(spec->type) {
  case TGA_TRUECOLOR:
    return fwrite(row, spec->Xres, 3, spec->fp);
    break;

  case TGA_TRUECOLOR_RLE:
    return RleCodeRGBRow(row, spec->Xres, spec->fp);
    break;

  case TGA_MAPPED:
    return fwrite(row, spec->Xres, 1, spec->fp);
    break;
  }

  fprintf(stderr, "Targa - saving of a row not supported for type %d!\n",
	  (int)(spec->type+'0'));
  
  return -1;
}

/************************************************************/
int saveTGAhead(targa_params_type *spec)
{
  int i;

  if(spec->fp == NULL) {
    fprintf(stderr,"Targa error - file not opened!\n");
    return 0;
  }

  if(spec->id_field == NULL)
    savebyte(0,spec->fp);  /* Number of Characters in Identification Field */
  else
    savebyte(strlen(spec->id_field),spec->fp);
  
  switch(spec->type) {
  case TGA_MAPPED:
  case TGA_MAPPED_RLE:
    if(spec->colormap == NULL) {
      fprintf(stderr,"Targa error -  colormap not specified!\n");
      return 0;
    }
    savebyte(1,spec->fp);  /* colormap */
    break;
  case TGA_GRAY:      
  case TGA_TRUECOLOR: 
  case TGA_TRUECOLOR_RLE: 
    savebyte(0,spec->fp);  /* no colormap */
    break;
  default:
    fprintf(stderr,"Targa type %d not supported!\n", (int)('0'+spec->type));
    return 0;
  }

  savebyte(spec->type, spec->fp);  /* image type */

  /* five bytes of colormap specification */
  switch(spec->type) {
  case TGA_MAPPED:    
  case TGA_MAPPED_RLE:
    /* (lo-hi) first colormap entry */
    savebyte(spec->first_cmap_entry&0xff, spec->fp);
    savebyte(spec->first_cmap_entry>>8, spec->fp);
    /* (lo-hi) colormap entries */
    savebyte(spec->num_cmap_entries&0xff, spec->fp);
    savebyte(spec->num_cmap_entries>>8, spec->fp);
    
    savebyte(24,spec->fp);          /* size of a colormap entry (16/24/32) */
    break;

  case TGA_GRAY:      
  case TGA_TRUECOLOR: 
  case TGA_TRUECOLOR_RLE: 
    for(i=0;i<5;i++)
      savebyte(0,spec->fp);
    break;
  }

  /* 2 bytes (lo-hi) X origin */
  savebyte(spec->Xorigin&0xff, spec->fp);
  savebyte(spec->Xorigin>>8, spec->fp);

  /* 2 bytes (lo-hi) Y origin */
  savebyte(spec->Yorigin&0xff, spec->fp);
  savebyte(spec->Yorigin>>8, spec->fp);

  /* 2 bytes (lo-hi) X resolution */
  savebyte(spec->Xres&0xff, spec->fp);
  savebyte(spec->Xres>>8, spec->fp);

  /* 2 bytes (lo-hi) Y resolution */
  savebyte(spec->Yres&0xff, spec->fp);
  savebyte(spec->Yres>>8, spec->fp);

  /* image pixel size */
  switch(spec->type) {
  case TGA_TRUECOLOR:
  case TGA_TRUECOLOR_RLE:
    savebyte(24, spec->fp);
    break;
  case TGA_GRAY:      
  case TGA_MAPPED: 
  case TGA_MAPPED_RLE:
    savebyte(8,spec->fp);
    break;
  }

  /*  Image Descriptor Byte.
      Bits 3-0 - number of attribute bits associated with each pixel.
                 For the Targa 16, this would be 0 or 1.
		 For the Targa 24, it should be 0.
		 For  Targa 32, it should be 8. 
      Bit 4    - reserved.  Must be set to 0.
      Bit 5    - screen origin bit.
                 0 = Origin in lower left-hand corner.
		 1 = Origin in upper left-hand corner.
		 Must be 0 for Truevision images.
      Bits 7-6 - Data storage interleaving flag.
                 00 = non-interleaved.
		 01 = two-way (even/odd) interleaving.
		 10 = four way interleaving.
		 11 = reserved.
		 */
  switch(spec->type) {
  case TGA_MAPPED:
  case TGA_MAPPED_RLE:
  case TGA_GRAY:      
  case TGA_TRUECOLOR: 
  case TGA_TRUECOLOR_RLE: 
    savebyte(0x20, spec->fp); /* origin in upper left hand corner */
    break;
  }
   
  /* no identification field */

  /* for mapped images, colormap should come here */
  switch(spec->type) {
  case TGA_MAPPED:
  case TGA_MAPPED_RLE:
    for(i=0;i<spec->num_cmap_entries;i++){
      savebyte(spec->colormap[i*3+2],spec->fp);  /* blue */
      savebyte(spec->colormap[i*3+1],spec->fp);  /* green */
      savebyte(spec->colormap[i*3+0],spec->fp);  /* red */
    }
    break;
  }
  
  /* set internal parameters */
  spec->current_row = spec->Yres-1;
  spec->orientation = -1;

  return 1;
}



/************************************************************/

/************************************************************/
int loadTGAhead(char *filename, targa_params_type *spec)
{
  byte pom, pom2, skip;
  byte tpom, tpom2;
  byte cmap;
  int i;

  if((spec->fp = fopen(filename,"r"))==NULL) {
    fprintf(stderr,"Targa error - cannot open file %s!\n", filename);
    return 0;
  }

  fread(&skip,1,1,spec->fp); /* Number of Characters in Identification Field */
  
  fread(&cmap,1,1,spec->fp);
  fread(&spec->type,1,1,spec->fp);

  switch(spec->type) {
  case TGA_MAPPED:    
  case TGA_MAPPED_RLE:    
  case TGA_GRAY:      
  case TGA_TRUECOLOR:
  case TGA_TRUECOLOR_RLE:
    break;

  default:
    fprintf(stderr,"Targa error - probably not targa image!\n");
    return 0;
  }

  /* Read in colormap specifications */
   
  /* (lo-hi) first colormap entry */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->first_cmap_entry = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* (lo-hi) colormap entries */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->num_cmap_entries = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* size of a colormap entry (16/24/32) */
  fread(&pom,1,1,spec->fp);
  if(cmap && (pom!=24)) {
    fprintf(stderr,
	    "Targa error - only colormaps with items of size 24 supported!\n");
    return 0;
  }

  /* 2 bytes (lo-hi) X origin */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->Xorigin = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) Y origin */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->Yorigin = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) X resolution */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->Xres = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* 2 bytes (lo-hi) Y resolution */
  fread(&pom,1,1,spec->fp);
  fread(&pom2,1,1,spec->fp);
  spec->Yres = (((unsigned int)pom2)<<8) + (unsigned int)pom;

  /* image pixel size */
  fread(&pom,1,1,spec->fp);

  switch(spec->type) {
  case TGA_TRUECOLOR:
  case TGA_TRUECOLOR_RLE:
    if(pom!=24) {
      fprintf(stderr,"Targa error - image pixel size 24 expected (is %d)!\n",
	      (int)pom);
      return 0;
    }
    break;
  case TGA_GRAY:      
  case TGA_MAPPED: 
  case TGA_MAPPED_RLE:
    if(pom!=8) {
      fprintf(stderr,"Targa error - image pixel size 8 expected (is %d)!\n",
	      (int)pom);
      return 0;
    }
    break;
  }

  /*  Image Descriptor Byte.
      Bits 3-0 - number of attribute bits associated with each pixel.
                 For the Targa 16, this would be 0 or 1.
		 For the Targa 24, it should be 0.
		 For  Targa 32, it should be 8. 
      Bit 4    - reserved.  Must be set to 0.
      Bit 5    - screen origin bit.
                 0 = Origin in lower left-hand corner.
		 1 = Origin in upper left-hand corner.
		 Must be 0 for Truevision images.
      Bits 7-6 - Data storage interleaving flag.
                 00 = non-interleaved.
		 01 = two-way (even/odd) interleaving.
		 10 = four way interleaving.
		 11 = reserved.
		 */
  fread(&pom,1,1,spec->fp);

  switch(spec->type) {
  case TGA_MAPPED:
  case TGA_MAPPED_RLE:
  case TGA_GRAY:      
  case TGA_TRUECOLOR: 
  case TGA_TRUECOLOR_RLE:
    if(pom!=0x20) {
      fprintf(stderr,"Targa error - image destriptor 0x20 expected (is %x)\n"
	      "           (Targa 24, origin up, non-interleaved).\n",
	      (int)pom);
      return 0;
    }
    break;
  }

  if(skip==0)
    spec->id_field = NULL;
  else {
#ifndef TEST_MALLOC
    if((spec->id_field = malloc(skip+1))!=NULL) {
#else
    if((spec->id_field = test_malloc(skip+1,"spec->id_field in loadTGAhead"))!=NULL) {
#endif
      /* read the identification field */
      for(i=0;i<skip;i++)
	fread(&spec->id_field[i],1,1,spec->fp);
      spec->id_field[skip] = 0;
    }
    else
      /* skip possible identification field */
      for(i=0;i<skip;i++)
	fread(&pom,1,1,spec->fp);
  }

  /* read or skip the colormap */
  if(!cmap) 
    spec->colormap = NULL;
  else {
    /* only skip for now */
    fprintf(stderr,"Targa warning - colormap is ignored.\n");

    for(i=0;i<spec->num_cmap_entries;i++)
      fread(&pom,1,3,spec->fp);
  }

  /* set internal parameters */
  spec->current_row = spec->Yres-1;
  spec->orientation = -1;
  spec->items_left = 0;

  return 1;
}

/************************************************************/
int loadTGAfinish(targa_params_type *spec)
{
  if(spec->id_field != NULL)
#ifndef TEST_MALLOC
    free(spec->id_field);
#else
    test_free(spec->id_field, "spec->id_field in loadTGAfinish");
#endif

  if(spec->colormap != NULL)
#ifndef TEST_MALLOC
    free(spec->colormap);
#else
    test_free(spec->colormap, "spec->colormap in loadTGAfinish");
#endif

  fclose(spec->fp);
}

/************************************************************/
int loadTGArow(targa_params_type *spec, int y, byte *row)
{
  if(spec->current_row != y) {
    fprintf(stderr, 
	    "Targa error - row number %d doesn't correspond to the "
	    "current row %d!\n", y, spec->current_row);
    return -1;
  }

  if((spec->current_row<0)||(spec->current_row >= spec->Yres)) {
    fprintf(stderr,"Targa error - row is out of the given Yres!\n");
    return -1;
  }

  spec->current_row += spec->orientation;

  switch(spec->type) {
  case TGA_TRUECOLOR:
    return fread(row, spec->Xres, 3, spec->fp);
    break;

  case TGA_TRUECOLOR_RLE:
    return RleDecodeRGBRow(row, spec);
    break;
  }

  fprintf(stderr, "Targa - saving of a row not supported for type %d!\n",
	  (int)(spec->type+'0'));
  
  return -1;
}

