#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "animparam.h"
#include "comlineparam.h"
#include "log.h"

static const unsigned int apDoubleBuffer = 1 << 0;
static const unsigned int apClearBetweenFrames = 1 << 1;
static const unsigned int apScaleBetweenFrames = 1 << 2;
static const unsigned int apNewViewBetweenFrames = 1 << 3;
static const unsigned int apCenterBetweenFrames = 1 << 4;
static const unsigned int apHCenterBetweenFrames = 1 << 5;

void InitializeAnimparam(ANIMPARAM *pAnimParameters) {
  pAnimParameters->flags = 0;
  SetDoubleBuffer(pAnimParameters, 1);
  SetClearBetweenFrames(pAnimParameters, 1);
  SetScaleBetweenFrames(pAnimParameters, 0);
  SetNewViewBetweenFrames(pAnimParameters, 0);
  SetCenterBetweenFrames(pAnimParameters, 0);
  SetHCenterBetweenFrames(pAnimParameters, 0);
  pAnimParameters->swap_interval = 1;
  pAnimParameters->first = 1;
  pAnimParameters->last = 1;
  pAnimParameters->step = 1;
  pAnimParameters->intervals[0].from = -1;
}

int DoubleBuffer(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apDoubleBuffer);
}

void SetDoubleBuffer(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apDoubleBuffer;
  else
    pAP->flags &= ~apDoubleBuffer;
}

int NewViewBetweenFrames(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apNewViewBetweenFrames);
}

void SetNewViewBetweenFrames(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apNewViewBetweenFrames;
  else
    pAP->flags &= ~apNewViewBetweenFrames;
}

int ScaleBetweenFrames(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apScaleBetweenFrames);
}

void SetScaleBetweenFrames(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apScaleBetweenFrames;
  else
    pAP->flags &= ~apScaleBetweenFrames;
}

int ClearBetweenFrames(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apClearBetweenFrames);
}

void SetClearBetweenFrames(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apClearBetweenFrames;
  else
    pAP->flags &= ~apClearBetweenFrames;
}

int CenterBetweenFrames(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apCenterBetweenFrames);
}

void SetCenterBetweenFrames(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apCenterBetweenFrames;
  else
    pAP->flags &= ~apCenterBetweenFrames;
}

int HCenterBetweenFrames(const ANIMPARAM *pAP) {
  return 0 != (pAP->flags & apHCenterBetweenFrames);
}

void SetHCenterBetweenFrames(ANIMPARAM *pAP, int set) {
  if (set)
    pAP->flags |= apHCenterBetweenFrames;
  else
    pAP->flags &= ~apHCenterBetweenFrames;
}

void ReadAnimData(const char *animfilename, ANIMPARAM *pAnimparam) {
  FILE *fp;
  static char on[] = "on";
  char buff[1024], *token = NULL, *token2 = NULL;
  int ind, dummy;

  InitializeAnimparam(pAnimparam);

  if (animfilename == NULL)
    return;

  if ((fp = fopen(animfilename, "r")) == NULL) {
    if (animfilename != NULL) {
      Message("Can't open animate file %s.\n", animfilename);
      Message("Using defaults.\n");
    }
  } else {
    while (!feof(fp)) {
      if (!fgets(buff, sizeof(buff), fp))
        break;
      if ((token = strtok(buff, ":\n")) == NULL)
        continue;
      if (strcmp(token, "frame intervals")) {
        if ((token2 = strtok(NULL, " ,\t:\n")) == NULL) {
          Message("Animate file: no parameter with command %s\n", token);
          continue;
        }
      }
      if (!strcmp(token, "double buffer")) {
        if (strncmp(token2, on, sizeof(on)) == 0)
          SetDoubleBuffer(pAnimparam, 1);
        else
          SetDoubleBuffer(pAnimparam, 0);
        continue;
      }

      if (!strcmp(token, "clear between frames")) {
        if (strncmp(token2, on, sizeof(on)) == 0)
          SetClearBetweenFrames(pAnimparam, 1);
        else{
          SetClearBetweenFrames(pAnimparam, 0);
	}
        continue;
      }

      if (!strcmp(token, "scale between frames")) {
       if (strncmp(token2, on, sizeof(on)) == 0)
          SetScaleBetweenFrames(pAnimparam, 1);
        else
          SetScaleBetweenFrames(pAnimparam, 0);
        continue;
      }

      if (!strcmp(token, "new view between frames")) {
        if (strncmp(token2, on, sizeof(on)) == 0)
          SetNewViewBetweenFrames(pAnimparam, 1);
        else
          SetNewViewBetweenFrames(pAnimparam, 0);
        continue;
      }

      if (!strcmp(token, "center between frames")) {
        if (strncmp(token2, on, sizeof(on)) == 0)
          SetCenterBetweenFrames(pAnimparam, 1);
        else
          SetCenterBetweenFrames(pAnimparam, 0);
        continue;
      }
      if (!strcmp(token, "hcenter between frames")) {
        if (strncmp(token2, on, sizeof(on)) == 0)
          SetHCenterBetweenFrames(pAnimparam, 1);
        else
          SetHCenterBetweenFrames(pAnimparam, 0);
        continue;
      }

      if (!strcmp(token, "swap interval")) {
        pAnimparam->swap_interval = atoi(token2);
        continue;
      }

      if (!strcmp(token, "first frame")) {
        pAnimparam->first = atoi(token2);
        if (pAnimparam->first < 0) {
          pAnimparam->first = 0;
        }
        continue;
      }

      if (!strcmp(token, "last frame")) {
        pAnimparam->last = atoi(token2);
        if (pAnimparam->last >= 0 && pAnimparam->last < pAnimparam->first) {
          pAnimparam->last = pAnimparam->first;
        }
        continue;
      }

      if (!strcmp(token, "step")) {
        pAnimparam->step = atoi(token2);
        if (pAnimparam->step <= 0) {
          pAnimparam->step = 1;
        }
        continue;
      }

      if (!strcmp(token, "frame intervals")) {
        ind = 0;

        for (;;) {
          if ((token = strtok(NULL, ",\n")) == NULL)
            break;

          pAnimparam->intervals[ind].from = -1;
          pAnimparam->intervals[ind].to = -1;
          pAnimparam->intervals[ind].step = -1;
          pAnimparam->intervals[ind].function = 0;

          sscanf(token, "%d-%d step %d", &pAnimparam->intervals[ind].from,
                 &pAnimparam->intervals[ind].to,
                 &pAnimparam->intervals[ind].step);

          if (sscanf(token, "%d-%d step %d rotate %f %f %f", &dummy, &dummy,
                     &dummy, &pAnimparam->intervals[ind].data[0],
                     &pAnimparam->intervals[ind].data[1],
                     &pAnimparam->intervals[ind].data[2]) == 6 ||
              sscanf(token, "%d-%d rotate %f %f %f", &dummy, &dummy,
                     &pAnimparam->intervals[ind].data[0],
                     &pAnimparam->intervals[ind].data[1],
                     &pAnimparam->intervals[ind].data[2]) == 5) {
            pAnimparam->intervals[ind].function = 1;
          }

          if (sscanf(token, "%d-%d step %d  scale %f %f %f", &dummy, &dummy,
                     &dummy, &pAnimparam->intervals[ind].data[0],
                     &pAnimparam->intervals[ind].data[1],
                     &pAnimparam->intervals[ind].data[2]) == 6 ||
              sscanf(token, "%d-%d  scale %f %f %f", &dummy, &dummy,
                     &pAnimparam->intervals[ind].data[0],
                     &pAnimparam->intervals[ind].data[1],
                     &pAnimparam->intervals[ind].data[2]) == 5) {
            pAnimparam->intervals[ind].function = 2;
          }

          if (pAnimparam->intervals[ind].to < 0)
            pAnimparam->intervals[ind].to = pAnimparam->intervals[ind].from;

          if (++ind >= MAXANIMINTERVALS) {
            Message("Warning: too many frame intervals, the rest ignored.\n");
            break;
          }
        }

        if (pAnimparam->intervals[0].from >= 0) {
          pAnimparam->first = pAnimparam->intervals[0].from;
          pAnimparam->last = pAnimparam->intervals[--ind].to;
        }

        continue;
      }
    }
    fclose(fp);

    if (clp.verbose) {
      if (DoubleBuffer(pAnimparam))
        Message("double buffer: on\n");
      else
        Message("double buffer: off\n");

      if (ClearBetweenFrames(pAnimparam))
        Message("clear between frames: on\n");
      else
        Message("clear between frames: off\n");

      if (ScaleBetweenFrames(pAnimparam))
        Message("scale between frames: on\n");
      else
        Message("scale between frames: off\n");

      if (NewViewBetweenFrames(pAnimparam))
        Message("new view between frames: on\n");
      else
        Message("new view between frames: off\n");

      if (CenterBetweenFrames(pAnimparam))
        Message("center between frames: on\n");
      else
        Message("center between frames: off\n");

      Message("swap interval: %d\n", pAnimparam->swap_interval);
      Message("first frame: %d\n", pAnimparam->first);
      Message("last frame: %d\n", pAnimparam->last);
      Message("step: %d\n", pAnimparam->step);
      if (pAnimparam->intervals[0].from > 0) {
        ind = 0;
        Message("frame intervals: ");

        while (pAnimparam->intervals[ind].from > 0) {
          Message("%d", pAnimparam->intervals[ind].from);
          if (pAnimparam->intervals[ind].to > pAnimparam->intervals[ind].from) {
            Message(" - %d step ", pAnimparam->intervals[ind].to);
            if (pAnimparam->intervals[ind].step > 0)
              Message("%d", pAnimparam->intervals[ind].step);
            else
              Message("%d", pAnimparam->step);
          }
          Message(", ");
          if (++ind >= MAXANIMINTERVALS)
            break;
        }
        Message("\n");
      }

      Message("FINISHED READING %s\n", animfilename);
    }
  }
}
