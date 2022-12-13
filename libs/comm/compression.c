#include "compression.h"

/* packet type */
#define PACKET_RAW 0
#define PACKET_RLE 1

/************************************************************/
static int SavePacket(char type, char *src, int item_size, int count,
                      char **dest) {
  unsigned char control;
  int i;

  if (count <= 0)
    return -1;

  control = (count - 1) & 0x7f;
  if (type == PACKET_RLE) {
    control |= 0x80; /* highest bit is 1 */
    count = 1;
  }

  **dest = control;
  (*dest)++;

  for (i = 0; i < count; i++) {
    *((*dest)++) = *src;
    src += item_size;
  }

  return count;
}

/***************************************************************************/
int RleEncode(char *src, int item_size, int nitems, char *dest) {
  int x;
  char *pb_ptr;
  int pb_x, row;
  char the_same;
  unsigned short val1, val2;
  char packet_type;
  char *store_dest = dest;
  char *store_src = src;

  for (row = 0; row < item_size; row++) {
    x = 0;

    src = store_src + row;

    /* store the beginning of a packet */
    pb_ptr = src;
    pb_x = 0;

    val1 = *src;
    src += item_size;
    x++;

    the_same = 0;
    packet_type = PACKET_RAW;

    while (x < nitems) {
      /* is the end of packet reached? */
      if (x - pb_x == 128) {
        SavePacket(packet_type, pb_ptr, item_size, x - pb_x, &dest);
        pb_ptr = src;
        pb_x = x;
        the_same = 0;
        packet_type = PACKET_RAW;
        val1 = 256;
      }

      val2 = *src;
      src += item_size;
      x++;

      if (val2 == val1) {
        if (packet_type == PACKET_RAW) {
          if (the_same) {
            /* three in a row */
            SavePacket(PACKET_RAW, pb_ptr, item_size, x - 3 - pb_x, &dest);
            pb_ptr = src - 3 * item_size;
            pb_x = x - 3;
            packet_type = PACKET_RLE;
          } else
            the_same = 1;
        }

        /* nothing for RLE packet */
      } else if (packet_type == PACKET_RAW)
        the_same = 0;
      else {
        /* RLE packet */
        SavePacket(PACKET_RLE, pb_ptr, item_size, x - 1 - pb_x, &dest);
        pb_ptr = src - item_size;
        pb_x = x - 1;
        packet_type = PACKET_RAW;
        the_same = 0;
      }

      val1 = val2;
    }

    /* the end of line reached */
    SavePacket(packet_type, pb_ptr, item_size, x - pb_x, &dest);
  }

  return dest - store_dest;
}

/************************************************************/
#define min(x, y) ((x) < (y) ? (x) : (y))

int RleDecode(char *src, int item_size, int nitems, char *dest) {
  int togo, i, row;
  int count;
  char *dest_ptr, val;
  unsigned char aux;

  for (row = 0; row < item_size; row++) {
    togo = nitems;
    dest_ptr = dest + row;

    while (togo > 0) {
      /* get a packet */
      aux = *(src++);
      count = (int)(aux & 0x7f) + 1;
      togo -= count;

      if (aux & 0x80) {
        /* rle packet */
        val = *(src++);

        for (i = 0; i < count; i++) {
          *dest_ptr = val;
          dest_ptr += item_size;
        }
      } else {
        /* raw packet */
        for (i = 0; i < count; i++) {
          *dest_ptr = *(src++);
          dest_ptr += item_size;
        }
      }
    }
  }

  return nitems;
}
