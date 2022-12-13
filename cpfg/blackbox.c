/********************************************************************/
/*								    */
/* MODULE:	blackbox.c					    */
/* AUTHOR:	Jim Hanan					    */
/* STARTED ON:    April 20, 1988	       			    */
/* PURPOSE:    This module is the "black box" which holds all	    */
/*	    functions specified using the @ followed by a	    */
/*	    single or multi character function identifier.	    */
/*	    The mainline calls init_blackbox for initialization.    */
/*	    Appropriate calls should be added there.		    */
/*	    Function calls should be added to the		    */
/*	    call_blackbox routine, which is called from the	    */
/*	    @ case in interpret.				    */
/*								    */
/* MODIFIED: August 91 by Jim at Apple for rendering dispatcher     */
/*	     March-June 94 by Radek			      	    */
/*               ansi standard                                      */
/*								    */
/********************************************************************/
/*								    */
/* FUNCTION	DATE	DESCRIPTION				    */
/*    o	    April 88    2d circle				    */
/*    c	    July 91	3d circle				    */
/*    O	    August 89    3d sphere				    */
/*		 January 91    modified for sphere library	    */
/*    v	    April 90    Roll turtle to vertical plane		    */
/* Movements:                                                       */
/*    Ti    Nov 94      increase elasticity                         */
/*    Td    May 94      decrease elasticity                         */
/*    Tp    Nov 94      adjust U and L according to the last segment*/
/*                      to prevent twist                            */
/*    Tf    Nov 94      force the twist                             */
/* Generalized cylinders:                                           */
/*    Gs    Dec 94      start spline                                */
/*    Gc    Dec 94      continue spline                             */
/*    Ge    Dec 94      end spline                                  */
/* Textures:                                                        */
/*    Tx    Dec 94      set segment texture (cylinders or splines)  */
/*                                                                  */
/*    R     June 95     set heading vector                          */
/*    M     Sept 95     set position                                */
/* Miscelaneous:                                                    */
/*    mo     Dec  95    new object                                  */
/*                      starts new object (mainly for rayshade      */
/*                      when a new grid is started (3 parameters    */
/*                      specify the grid size)                      */
/*    mc    Dec  95     conditional cut                             */
/*                      if the parameter is 1, it acts as %         */
/********************************************************************/

#ifdef WIN32
#include "warningset.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "control.h"
#include "generate.h"
#include "interpret.h"
#include "utility.h"
#include "environment.h"
#include "quaternions.h"
#include "rayshade.h"
#include "viewVol.h"

#include "blackbox.h"
#include "patch.h"
#include "general.h"
#include "textures.h"
#include "movements.h"
#include "log.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#define EPr environmentparam

/*extern void adjust_up_and_left(TURTLE *turtle); */
/* from tropisms.c */

/********************************************************************/
/* init_blackbox is called from the TurtleDraw routine.		    */
/* Calls for initialization of each function are issued.	    */
/********************************************************************/
void init_blackbox(void) {
  VERBOSE("init blackbox\n");
  /* calls of the form init_whateverfunction() should be added here */
  InitSurfacePatches(); /* found in patch.c */
}

/********************************************************************/
/* call_blackbox is called from interpret.c when a @ is encountered.*/
/* The function indicated by the leading symbol(s) of str is	    */
/* performed. Rendering is done by the dispatch routines	    */
/********************************************************************/
void call_blackbox(char *(*str), /* the string being interpreted */
                   struct TURTLE *turtle, DRAWPARAM *dr, VIEWPARAM *vw) {
  extern int nextGlobalString; /* maximum allowed global string index - JH1 */

  char function, subfunction;
  static char subrequired[] = "PTGmDb"; /* symbols of functions requiring
   subfunction symbols */
  StringModule module;     /* Structure to hold symbol and parameters */
  StringModule subModule;  /* Structure to hold subsymbol and parameters */
  double parameter1;       /* Value of first parameter of function symbol */
  double parameter2 = 0.0; /* Value of first parameter of subfunction symbol */
  int parm1;               /* Flag indicating presence of parameter */
  int parm2 = 0;           /* Flag indicating presence of parameter */

  /* main line declarations for the v function */
  static double yvec[] = {0.0, 1.0, 0.0}; /* vertical axis */
  int size[3];

  /* main line declarations for the T functions */
  double aux;
  short index;
  char *store_str; /* to remember position in a string if the function is */
                   /* not a valid blackbox function */

  /* main line declarations for the P functions */
  /* none */

  /* main line declarations for the L function */
  /* none */

  /* Determine desired function */
  /* extract next symbol and first parameter if available */
  store_str = *str;
  function = NextStringModule(str, &module);

  if (module.parameters > 0) {
    parameter1 = module.actual[0].value;
    parm1 = 1;
  } else {
    parameter1 = 0.0;
    parm1 = 0;
  }

  if (function == '\0') {
    /* if end of string, reset and return for normal termination */
    (*str)--;
    Message("End of string reached before function found.\n");
    return;
  }

  subfunction = '\0';
  subModule.symbol = '\0';
  subModule.parameters = 0;
  /* If necessary, determine desired subfunction */
  if (strchr(subrequired, function) != NULL && !parm1) {
    /* extract next symbol and first parameter if available */
    subfunction = NextStringModule(str, &subModule);

    if (subModule.parameters > 0) {
      parameter2 = subModule.actual[0].value;
      parm2 = 1;
    } else {
      parameter2 = 0.0;
      parm2 = 0;
    }
    if (subfunction == '\0') {
      /* if end of string, reset and return for normal termination */
      (*str)--;
      Message("End of string reached before subfunction found.\n");
      return;
    }
  }

  /* call the appropriate function */
  /* must scale by turtle->scale_factor for geometry */
  switch (function) {
  case ',':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (!parm1)
        turtle->color_index_back -= dr->color_increment_back;
      else
        turtle->color_index_back = clp.colormap + (int)parameter1;
      if (turtle->color_index_back < clp.colormap)
        turtle->color_index_back += 256;
      (*dr->tdd->SetColour)(turtle, dr, vw);
    }
    break;

    /* ;  Assign the parameter value(s) to color if present or */
    /*    Increase the current color index by the color increment. */
  case ';':
    if ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)) {
      if (!parm1) {
        turtle->color_index_back += dr->color_increment_back;
      } else {
        turtle->color_index_back = clp.colormap + (int)parameter1;
      }
      if (turtle->color_index_back > clp.colormap + 255) {
        turtle->color_index_back -= 256;
      }
      (*dr->tdd->SetColour)(turtle, dr, vw);
    }
    break;

  case 'o': /* draw a "two dimensional" circle */
    if (!parm1) {
      parameter1 = turtle->line_width;
    } else {
      parameter1 *= turtle->scale_factor;
    }
    if ((dr->tdd != NULL) &&
        ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
      (*dr->tdd->Circle2D)(turtle, dr, vw, (float)parameter1);
    break;
  case 'c': /* draw a "three dimensional" circle */
    if (!parm1) {
      parameter1 = turtle->line_width;
    } else {
      parameter1 *= turtle->scale_factor;
    }
    if ((dr->tdd != NULL) &&
        ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
      (*dr->tdd->Circle3D)(turtle, dr, vw, (float)parameter1);
    break;
  case 'b': /* draw a circle with a hole in the middle */
    if (!parm2) {
      parameter2 = turtle->line_width;
    } else {
      parameter2 *= turtle->scale_factor;
    }
    switch (subfunction) {
    case 'o': /* draw a "two dimensional" bounded circle */
      if ((dr->tdd != NULL) && (dr->tdd->CircleB2D != NULL) &&
          ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol))) {
        (*dr->tdd->CircleB2D)(turtle, dr, vw, (float)parameter2,
                              (float)turtle->line_width);
      }
      break;
    case 'c': /* draw a "three dimensional" bounded circle */
      if ((dr->tdd != NULL) && (dr->tdd->CircleB3D != NULL) &&
          ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
        (*dr->tdd->CircleB3D)(turtle, dr, vw, (float)parameter2,
                              (float)turtle->line_width);
      break;
    }
    break;
  case 'O': /* draw a sphere */
    if (!parm1) {
      parameter1 = turtle->line_width;
    } else {
      parameter1 *= turtle->scale_factor;
    }
    if ((dr->tdd != NULL) &&
        ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
      (*dr->tdd->Sphere)(turtle, dr, vw, (float)parameter1);
    break;
  case 'v': /* Roll the turtle to vertical */
            /* calculate new left vector */
    if ((turtle->heading[1] < 1.0) && (turtle->heading[1] > -1.0)) {
      DCrossProduct(yvec, turtle->heading, turtle->left);

      DNormalize(turtle->left);

      /* and up vector */
      DCrossProduct(turtle->left, turtle->heading, turtle->up);
    }
    break;
#ifdef RESEARCH_VER
  case 'R': /* Set the heading vector to predefined one */
    if (module.parameters >= 2) {
      turtle->heading[0] = module.actual[0].value;
      turtle->heading[1] = module.actual[1].value;

      if (module.parameters == 2)
        turtle->heading[2] = 0.0;
      else
        turtle->heading[2] = module.actual[2].value;

      if (DDotProduct(turtle->heading, turtle->heading) == 0.0) {
        turtle->heading[0] = 0.0;
        turtle->heading[1] = 1.0;
        turtle->heading[2] = 0.0;
      } else
        DNormalize(turtle->heading);

      /* extract possible up vector */
      if (module.parameters >= 6) {
        turtle->up[0] = module.actual[3].value;
        turtle->up[1] = module.actual[4].value;
        turtle->up[2] = module.actual[5].value;

        /* must be nonzero and perpendicular to heading vector */
        if ((DDotProduct(turtle->up, turtle->up) == 0.0) ||
            (DDotProduct(turtle->up, turtle->heading) != 0.0))
          /* no twist assumption */
          adjust_up_and_left(turtle);
        else {
          DNormalize(turtle->up);

          DCrossProduct(turtle->heading, turtle->up, turtle->left);
        }
      } else
        adjust_up_and_left(turtle);
    }
    break;
  case 'M': /* move the turtle to a given point */
    if (module.parameters >= 2) {
      turtle->position[0] = module.actual[0].value;
      turtle->position[1] = module.actual[1].value;

      if (module.parameters == 2)
        turtle->position[2] = 0.0;
      else
        turtle->position[2] = module.actual[2].value;

      if (dr->output_type == TYPE_VIEWVOLUME)
        vvUpdate(turtle->position, vw);
    }
    break;
#endif      /* RESEARCH_VER */
  case 'P': /* L-system defined Surface patch processing */
    /* no scaling required since the turtle defined the points */
    /* check that patch id is present */
    if (!parm2) {
      Message("WARNING: Patch call P%c without ID ignored.\n", subfunction);
      break;
    }
    switch (subfunction) {
    case 'S': /* initialize patch */
      SurfacePatchInit(&subModule);
      break;
    case 'C': /* record control point */
      SurfacePatchControlPoint(&subModule, turtle);
      break;
    case 'D': /* draw patch */
      if ((dr->tdd != NULL) &&
          ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
        (*dr->tdd->LdefinedSurface)(&subModule, turtle, dr, vw);
      break;
    default:
      *str = store_str; /* restore the string just after '@' */
      Message("Unknown subfunction %c following @P.\n", subfunction);
    }
    break;

  case 'L':
    /* Print label indexed by first parameter at the current turtle position */

    if (!parm1 || parameter1 >= nextGlobalString) {
      Message("WARNING: Global string reference out of range for label. ");
      Message(" Using last string defined.\n");
      parameter1 = nextGlobalString;
    }
    /* pass character string and remaining parameters to rendering routines */
    if ((dr->tdd != NULL) &&
        ((dr->output_type != TYPE_ENVIRONMENT) || (dr->env_output_symbol)))
      (*dr->tdd->Label)(turtle, dr, vw, GlobalString(parameter1),
                        module.parameters, &module.actual[1].value);
    break;

  case 'S': /* JH1 */
    /* Pass string indexed by first parameter to csh  */

    if (!parm1 || parameter1 >= nextGlobalString) {
      Message("WARNING: Global string reference out of range for label. ");
      Message(" Using last string defined.\n");
      parameter1 = nextGlobalString;
    }
    /* pass character string and remaining parameters to shell */
    if (dr->output_type != TYPE_ENVIRONMENT)
      (void)system(GlobalString(parameter1));
    break;

  case 'T':
    if (subfunction == 'x') {
      /* set texture */
      turtle->texture = NOTEXTURE;

      if (parm2) {
        turtle->texture = (int)parameter2 - 1;

        if (!is_valid_texture_index(turtle->texture)) {
          // putting a warning here spits too much spam to the console.
          // just gracefully recover. We can warn the user closer to where
          // the problem originated.
          turtle->texture = NOTEXTURE;
        }
        return;
      }
    }
    /* any movement  */ /* Radek */
    /* scaling doesn't apply */
    if (!parm2) {
      index = 0;
    } else {
      index = (short)(parameter2 - 1);
      if ((index < 0) || (index >= EPr.number_of_movements)) {
        Message("Movement index out of range!\n");
        break;
      }
    }
    if (subModule.parameters >= 2)
      aux = subModule.actual[1].value;
    else
      aux = EPr.movements[index].elasticity_incr;

    switch (subfunction) {
    case 'i': /* increase elasticity */
      break;
    case 'd': /* decrease elasticity */
      aux = -aux;
      break;
    case 's':
      if (subModule.parameters >= 2)
        aux = subModule.actual[1].value - turtle->movements[index].elasticity;
      else /* set to 0 */
        aux = -turtle->movements[index].elasticity;
      break;

      /* following subfunctions control the no twist adjustment */
    case 'p': {
      /* adjust up and left vector to prevent twist with
              respect to the previous segment - no parameters */

      if (parm1)
        Message("Parameter for @Tp ignored.\n");

      adjust_up_and_left(turtle);
      return;
    }
    case 'f':
      /* previous twist was desirable. set prev-up to current up*/
      turtle->prev_heading[0] = turtle->heading[0];
      turtle->prev_heading[1] = turtle->heading[1];
      turtle->prev_heading[2] = turtle->heading[2];

      turtle->prev_up[0] = turtle->up[0];
      turtle->prev_up[1] = turtle->up[1];
      turtle->prev_up[2] = turtle->up[2];
      return;
    default:
      *str = store_str; /* restore the string just after '@' */
      Message("Unknown subfunction %c following @T.\n", subfunction);
    }

    if (parm2) {
      turtle->movements[index].elasticity += aux;
      update_movement(index, aux);
    }
    break;
  case 'G': {
    /* generalized cylinders  */ /* Radek */
    int rings = 0;

    if (dr->tdd != NULL) {
      switch (subfunction) {
      case 's': /* start spline */
        break;

      case 'c': /* continue spline */
      case 'e': /* end spline */
        if (subModule.parameters >= 1) {
          rings = subModule.actual[0].value;
          if (rings < 1)
            rings = 1;
        }
        break;
      case 't': /* tangent parameter */
        if (subModule.parameters >= 1) {
          turtle->tangent_parameter[0] = subModule.actual[0].value;

          if (subModule.parameters > 1)
            turtle->tangent_parameter[1] = subModule.actual[1].value;
          else
            turtle->tangent_parameter[1] = turtle->tangent_parameter[0];
        }
        break;
      case 'r': /* radius tangents */
        if (subModule.parameters == 1) {
          turtle->scale_radius_tangents = subModule.actual[0].value;
          break;
        }
        if (subModule.parameters >= 2) {
          /* both tangents the same if the second not specified */
          turtle->radius_tangent[0][0] = turtle->radius_tangent[1][0] =
              cos(subModule.actual[0].value / 180.0 * M_PI) *
              subModule.actual[1].value;
          turtle->radius_tangent[0][1] = turtle->radius_tangent[1][1] =
              sin(subModule.actual[0].value / 180.0 * M_PI) *
              subModule.actual[1].value;

          if (subModule.parameters >= 4) {
            turtle->radius_tangent[1][0] =
                cos(subModule.actual[2].value / 180.0 * M_PI) *
                subModule.actual[3].value;
            turtle->radius_tangent[1][1] =
                sin(subModule.actual[2].value / 180.0 * M_PI) *
                subModule.actual[3].value;
          }
        }
        break;
      }

      switch (subfunction) {
      case 's': /* start spline */
        start_spline(turtle, dr);
        break;
      case 'c': /* continue spline */
        continue_spline(turtle, dr, rings);
        break;
      case 'e': /* end spline */
        finish_spline(turtle, dr, rings);
        break;
      case 't':
      case 'r':
        /* do nothing */
        break;
      default:
        *str = store_str; /* restore the string just after '@' */
        Message("Unknown subfunction %c following @G.\n", subfunction);
      }
    }
    break;
  }
#ifdef RESEARCH_VER
  case 'm': {
    /* miscelaneous  */ /* Radek */
    if (dr->tdd != NULL) {

      switch (subfunction) {
      case 'o': /* new object - so far just in rayshade */
        if (dr->output_type == TYPE_RAYSHADE) {

          /* set the grid size */
          for (index = 0; index < 3; index++)
            if (subModule.parameters >= index + 1)
              size[index] = subModule.actual[index].value;
            else
              size[index] = 1;

          rsStartNewGrid(turtle, dr, vw, size);
        }
        break;
      case 'c': /* conditional cut */
        if (subModule.parameters >= 1)
          if (subModule.actual[0].value == 1)
            SkipCutPart(str);
        break;
      default:
        *str = store_str; /* restore the string just after '@' */
        Message("Unknown subfunction %c following @m.\n", subfunction);
      }
    }
    break;
  }
#endif      /* RESEARCH_VER */
  case '#': /* change the current contour */
    turtle->contour = parm1 ? get_contour_index((int)parameter1) : DEF_CONTOUR;
    break;

  case '!': /* change the current cylinder_sides */
    turtle->cylinder_sides = parm1 ? (int)parameter1 : 8;
    if (turtle->cylinder_sides < 3)
      turtle->cylinder_sides = 3;
    break;

  case 'I': /* include a rayshade object */
    if (dr->output_type == TYPE_RAYSHADE) {

      if (!parm1 || parameter1 >= nextGlobalString) {
        Message("WARNING: Global string reference out of range for @I. ");
        Message(" Using last string defined.\n");
        parameter1 = nextGlobalString;
      }
      /* pass character string and remaining parameters */
      rsInsertObject(GlobalString(parameter1),
                     module.parameters >= 2 ? module.actual[1].value : 1,
                     turtle, vw, dr);
      break;
    }
    break;

  case 'J': /* SEE ALSO @mo! new object - so far just in rayshade */
    if (dr->tdd != NULL)
      if (dr->output_type == TYPE_RAYSHADE) {

        /* set the grid size */
        for (index = 0; index < 3; index++)
          if (subModule.parameters >= index + 1)
            size[index] = subModule.actual[index].value;
          else
            size[index] = 1;

        rsStartNewGrid(turtle, dr, vw, size);
      }
    break;

  case 'D': { /* dimensions - modifies turtle scale */
    if (dr->tdd != NULL) {

      switch (subfunction) {
      case '\0': /* no subfunction - set the scale */
        if (parm1)
          turtle->scale_factor = module.actual[0].value;
        else
          Message("Parameter of module @D missing.\n");
        break;

      case 'i': /* increment - multiply scale by a factor */
        if (parm2) {
          if (subModule.actual[0].value != 0)
            turtle->scale_factor *= subModule.actual[0].value;
          else
            Message("The first parameter of module @Di must be non-zero.\n");
        } else
          turtle->scale_factor *= dr->scale_multiplier;
        break;

      case 'd': /* decrement - divide scale by a factor */
        if (parm2) {
          if (subModule.actual[0].value != 0)
            turtle->scale_factor /= subModule.actual[0].value;
          else
            Message("The first parameter of module @Dd must be non-zero.\n");
        } else
          turtle->scale_factor /= dr->scale_multiplier;
        break;

      default:
        *str = store_str; /* restore the string just after '@' */
        Message("Unknown subfunction %c following @D.\n", subfunction);
      }
    }
    break;
  }
  default:
    *str = store_str; /* restore the string just after '@' */
    /* pass to rendering routine in case it knows what to do */

    if (function != 'Z' && function != 'Y') /* these two are used by 'ilsa' */
      Message("Unknown function %c following @.\n", function);
  }
}
