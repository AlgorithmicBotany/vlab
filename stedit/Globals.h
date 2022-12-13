/* ********************************************************************
 *
 * Copyright (c) 2019 Biological Modeling and Visualization Laboratory,
 *
 * University of Calgary. All rights reserved.
 *
 * ********************************************************************/

#pragma once

/* The maximum size of the undo stack. If the stack becomes larger, the oldest
 * states will be thrown out*/
#define MAX_UNDOS 200

/* A mathematical constant whose value is the ratio of any circular pastry
 * desert's circumference to its diameter in Euclidean space */
#define PI                                                                     \
  3.1415926535897932384626433832795028841971693993751058209749445923078164062862089986280348253421170679

/* The size inside the window used to draw the outer border and constrain the
 * placement of texture control points */
#define WIN_SIZE 0.98

/* Default display size of control points for the bezier and texture editors */
#define DEFAULT_POINT_SIZE 6

/* Default display size of the contact point for the bezier editor */
#define DEFAULT_CONTACT_POINT_SIZE 10

/* Default display width of lines for the bezier and texture editors */
#define DEFAULT_LINE_WIDTH 1

/* Default display width of wireframe lines for the bezier editor */
#define DEFAULT_WIREFRAME_WIDTH 1

/* Default subdivision samples for bezier patches in the bezier editor */
#define DEFAULT_SUBDIVISION_SAMPLES 20

enum SavingMode { CONTINUOUS, TRIGGERED, OFF };
