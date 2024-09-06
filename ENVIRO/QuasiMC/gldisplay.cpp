#define GL_SILENCE_DEPRECATION
#include <QtGui>
#include <QtOpenGL>

#include "gldisplay.h"
#include "quasiMC.h"

#ifdef __cplusplus
extern "C" {
#endif
extern int return_type;
extern SCENE scene;
extern QUERIES queries;
extern int use_sky_model;
extern int rays_from_objects;

extern VISUALIZATION visualization;

extern float max_sky_intensity;
extern double intensity[AZIMUTH_SAMPLES][ALTITUDE_SAMPLES];

#ifdef __cplusplus
}
#endif

extern QMutex scene_mutex;

#define MAX_RAYS 512
static float rays[MAX_RAYS][6];
static int num_rays = 0;
static float sky_pos[AZIMUTH_SAMPLES][ALTITUDE_SAMPLES][3];
static float sky_radius;

static void DrawGrid(void);
static void DrawSphere(void);

/* -- implementation of GLDisplay class -----------------------  */

GLDisplay::GLDisplay(QWidget *parent) : QGLWidget(parent) {
  xpos = 0.0;
  ypos = 0.0;
  zpos = 0.0;
  xrot = 0.0;
  yrot = 0.0;
  zrot = 0.0;
  mouse_x = 0;
  mouse_y = 0;
  disk = NULL;
  showbox = 0;
  showsphere = 1;
  showwireframe = 0;
  quasimc_update = 1;
  sky_radius = 0.f;

  // setup popup menu
  popup_menu = new QMenu(this);

  showsphereAct = new QAction(tr("Show light source"), this);
  connect(showsphereAct, SIGNAL(triggered()), this, SLOT(showBoundingSphere()));
  showsphereAct->setCheckable(TRUE);
  showsphereAct->setChecked(showsphere);
  popup_menu->addAction(showsphereAct);

  showboxAct = new QAction(tr("Show bounding box"), this);
  connect(showboxAct, SIGNAL(triggered()), this, SLOT(showBoundingBox()));
  showboxAct->setCheckable(TRUE);
  showboxAct->setChecked(showbox);
  popup_menu->addAction(showboxAct);

  showwireframeAct = new QAction(tr("Show wireframe"), this);
  connect(showwireframeAct, SIGNAL(triggered()), this, SLOT(showWireFrame()));
  showwireframeAct->setCheckable(TRUE);
  showwireframeAct->setChecked(showwireframe);
  popup_menu->addAction(showwireframeAct);


  QAction *outputvertices =
      new QAction(tr("Output vertices ('vertices.dat')"), this);
  connect(outputvertices, SIGNAL(triggered()), this, SLOT(outputVertices()));
  popup_menu->addAction(outputvertices);

  QAction *outputimage = new QAction(tr("Output image ('quasimc.png')"), this);
  connect(outputimage, SIGNAL(triggered()), this, SLOT(outputImage()));
  popup_menu->addAction(outputimage);
}
/* ------------------------------------------------------------ */
GLDisplay::~GLDisplay() {
  if (disk != NULL)
    gluDeleteQuadric(disk);
}
/* ------------------------------------------------------------ */
void GLDisplay::paintGL() {
  int i;
  float colour;

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // check if quasiMC is busy computing...if yes, print message and check again.
  if (scene_mutex.tryLock() == FALSE) {
    // print wait message - if user tries to interact with debug window
    glColor3f(0.1f, 0.9f, 0.1f);
    renderText(10, 10, "Please wait...");
    return;
  }

  // zoom out far enough so that the entire bbox and bsph can be seen
  if (quasimc_update && zpos > -scene.grid.bsph_radius * 2.5f) {
    quasimc_update = 0;
    zpos = -scene.grid.bsph_radius * 2.5f;
  }

  glPushMatrix();

  // in lpfg, the view is first rotated around z-axis, then
  // translanted then rotated around xy
  glRotatef(zrot, 0.f, 0.f, 1.f);

  glTranslatef(xpos, ypos, zpos);

  glRotatef(xrot, 1.f, 0.f, 0.f);
  glRotatef(yrot, 0.f, 1.f, 0.f);

  glTranslatef(-scene.grid.bsph_centre[0], -scene.grid.bsph_centre[1],
               -scene.grid.bsph_centre[2]);
 
  glLineWidth(1.0);
  glColor3f(0.9f, 0.f, 0.f);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  // bounding box (show the grid)
  if (showbox)
    DrawGrid();

  // bounding sphere with light source
  if (showsphere) {
    glPushMatrix();
    // translate using centre of sphere BUT for y use bbox so leaves and sphere
    // hit same ground plane
    glTranslatef(scene.grid.bsph_centre[0], scene.grid.bbox[1],
                 scene.grid.bsph_centre[2]);
    DrawSphere();
    if (!use_sky_model) {
      if (scene.max_light_weight > 0.0)
        colour = 1.f / scene.max_light_weight;
      else
        colour = 0.0;
      for (i = 0; i < scene.num_lights; i++) {
        glPointSize(10.0 * scene.lights[i].weight * colour + 1.0f);
        glBegin(GL_POINTS);
        glColor3f(scene.lights[i].weight * colour,
                  scene.lights[i].weight * colour, 0.f);
        glVertex3f(-scene.lights[i].dir[0] * sky_radius,
                   -scene.lights[i].dir[1] * sky_radius,
                   -scene.lights[i].dir[2] * sky_radius);
        glEnd();
      }
    }
    glPopMatrix();
  }

  // draw the primitives
  if (showwireframe)
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  else
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  if (rays_from_objects) {
    // in rays_from_objects, the queries do not store primitives
    for (i = 0; i < scene.num_primitives; i++) {
      // set colour of primitive according to 'return type'
      if (return_type == ABSORBED_FLUX)
        colour = scene.primitives[i].absorbed_flux[0] / max_intensity;
      else if (return_type == ABSORBED_IRRADIANCE)
        colour = scene.primitives[i].absorbed_flux[0] /
                 (scene.primitives[i].area * max_intensity);
      else if (return_type == UP_INCIDENT_IRRADIANCE || LW_INCIDENT_IRRADIANCE)
        colour = scene.primitives[i].incident_flux[0] /
                 (scene.primitives[i].area * max_intensity);

      glColor3f(colour * visualization.colour_mod[0],
                colour * visualization.colour_mod[1],
                colour * visualization.colour_mod[2]);
      if (scene.primitives[i].object_type == 3) {
        glBegin(GL_TRIANGLES);
        glVertex3fv(&scene.primitives[i].data[0]);
        glVertex3fv(&scene.primitives[i].data[3]);
        glVertex3fv(&scene.primitives[i].data[6]);
        glEnd();
      } else {
        glBegin(GL_QUADS);
        glVertex3fv(&scene.primitives[i].data[0]);
        glVertex3fv(&scene.primitives[i].data[3]);
        glVertex3fv(&scene.primitives[i].data[6]);
        glVertex3fv(&scene.primitives[i].data[9]);
        glEnd();
      }
    }

    // draw the query object?
  } else {
    // show all objects in one colour based on the associated query
    for (i = 0; i < queries.num_queries; i++) {
      // set colour of primitive using saved radiant flux for this query (at
      // first wavelength)
      colour = queries.query[i].radiant_flux[0] / max_intensity;
      glColor3f(colour * visualization.colour_mod[0],
                colour * visualization.colour_mod[1],
                colour * visualization.colour_mod[2]);

      if (queries.query[i].in_polygon != 0) {
        // CAREFUL: index 'i' is modified within the while loop!!!
        // why are 'polygons' implemented this way (made of several queries) and
        // not like Bezier surfaces?
        glBegin(GL_TRIANGLES);
        while (i + 1 < queries.num_queries &&
               queries.query[i].in_polygon == queries.query[i + 1].in_polygon) {
          int j = queries.query[i].primitive_index;
          glVertex3fv(&scene.primitives[j].data[0]);
          glVertex3fv(&scene.primitives[j].data[3]);
          glVertex3fv(&scene.primitives[j].data[6]);
          ++i;
        }
        int j = queries.query[i].primitive_index;
        glVertex3fv(&scene.primitives[j].data[0]);
        glVertex3fv(&scene.primitives[j].data[3]);
        glVertex3fv(&scene.primitives[j].data[6]);
        glEnd();
      } else {
        // if single 'T' or 'P'
        if (queries.query[i].surface_size == 1) {
          int j = queries.query[i].primitive_index;
          if (scene.primitives[j].object_type == 3) {
            glBegin(GL_TRIANGLES);
            glVertex3fv(&scene.primitives[j].data[0]);
            glVertex3fv(&scene.primitives[j].data[3]);
            glVertex3fv(&scene.primitives[j].data[6]);
            glEnd();
          } else {
            glBegin(GL_QUADS);
            glVertex3fv(&scene.primitives[j].data[0]);
            glVertex3fv(&scene.primitives[j].data[3]);
            glVertex3fv(&scene.primitives[j].data[6]);
            glVertex3fv(&scene.primitives[j].data[9]);
            glEnd();
          }
        } else // this is an object of several triangles: 'S' or 'I'
        {
          int pIndex = queries.query[i].primitive_index;
          glBegin(GL_TRIANGLES);
          for (int j = pIndex - queries.query[i].surface_size + 1;
               j < pIndex + 1; j++) {
            glVertex3fv(&scene.primitives[j].data[0]);
            glVertex3fv(&scene.primitives[j].data[3]);
            glVertex3fv(&scene.primitives[j].data[6]);
          }
          glEnd();
        }
      }
    }
  }

  // draw the rays
  for (i = 0; i < num_rays; i++) {
    glPointSize(3.0);
    glBegin(GL_POINTS);
    glColor3f(0.9f, 0.1f, 0.1f);
    glVertex3fv(rays[i]);
    glEnd();

    glLineWidth(1.0);
    glBegin(GL_LINES);
    glColor3f(0.9f, 0.1f, 0.1f);
    glVertex3f(rays[i][0], rays[i][1], rays[i][2]);
    glColor3f(0.9f, 0.9f, 0.9f);
    glVertex3f(rays[i][0] + rays[i][3], rays[i][1] + rays[i][4],
               rays[i][2] + rays[i][5]);
    glEnd();
  }

  glPopMatrix();

  scene_mutex.unlock();

  glFlush();
  return;
}
/* ------------------------------------------------------------ */
void GLDisplay::initializeGL() {
  glClearColor(0.0, 0.0, 0.0, 0.0);
  glClearDepth(1.0);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

  disk = gluNewQuadric();
}
/* ------------------------------------------------------------ */
void GLDisplay::resizeGL(int w, int h) {
  float backDistance;
  glViewport(0, 0, w, h);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  backDistance = sky_radius > 0.f ? sky_radius * 20.f : 8192.f;
  if (h == 0)
    gluPerspective(45.f, (float)w, 0.1f, backDistance); // 8192.0);
  else
    gluPerspective(45.f, (float)w / (float)h, 0.1f, backDistance); // 8192.0);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}
/* ------------------------------------------------------------ */
void GLDisplay::updateGLWindow(void) {
  int i, j;
  double azi_sample, alt_sample;

  quasimc_update = 1;
  max_intensity = 0.0001f;
  scene_mutex.lock();

  // colour primitives according to return type
  if (rays_from_objects) {
    for (i = 0; i < scene.num_primitives; i++) {
      if (return_type == UP_INCIDENT_IRRADIANCE ||
          return_type == LW_INCIDENT_IRRADIANCE) {
        if (scene.primitives[i].incident_flux[0] / scene.primitives[i].area >
            max_intensity)
          max_intensity =
              scene.primitives[i].incident_flux[0] / scene.primitives[i].area;
      } else if (return_type == ABSORBED_IRRADIANCE) {
        if (scene.primitives[i].absorbed_flux[0] / scene.primitives[i].area >
            max_intensity)
          max_intensity =
              scene.primitives[i].absorbed_flux[0] / scene.primitives[i].area;
      } else if (return_type == ABSORBED_FLUX) {
        if (scene.primitives[i].absorbed_flux[0] > max_intensity)
          max_intensity = scene.primitives[i].absorbed_flux[0];
      }
    }
  } else {
    for (i = 0; i < queries.num_queries; i++)
      if (queries.query[i].radiant_flux[0] > max_intensity)
        max_intensity = queries.query[i].radiant_flux[0];
  }

  // calculate pts in sky to display intensities - should be done everytime
  // bounding sphere radius changes
  sky_radius = Length(scene.grid.bbox, scene.grid.bbox + 3);
  for (i = 0; i < AZIMUTH_SAMPLES; i++) // azimuth phi in [-pi,pi)
  {
    azi_sample = (((double)i) * 2.0 * M_PI) / ((double)AZIMUTH_SAMPLES) - M_PI;
    for (j = 0; j < ALTITUDE_SAMPLES; j++) // inclination theta in [0,pi]
    {
#if NEW_SKY_MODEL == 0
      alt_sample = ((double)j) * M_PI * 0.5 / ((double)ALTITUDE_SAMPLES);
      // change to spherical coordinates where Z is up
      // (this is how the CIE luminance distribution functions are defined
      // note that openGL still expects Y to be up, so have to adjust indicies
      // accordingly in DrawSphere()
      sky_pos[i][j][0] = -sin(azi_sample) * cos(alt_sample) * sky_radius;
      sky_pos[i][j][1] = -cos(azi_sample) * cos(alt_sample) * sky_radius;
      sky_pos[i][j][2] = sin(alt_sample) * sky_radius;
#else
      // rotate azi sample by -pi*0.5 to match coordinate system from old sky
      // model
      alt_sample =
          M_PI * 0.5 - ((double)j) * M_PI * 0.5 / ((double)ALTITUDE_SAMPLES);
      sky_pos[i][j][2] = -sin(alt_sample) * cos(azi_sample) * sky_radius;
      sky_pos[i][j][0] = sin(alt_sample) * sin(azi_sample) * sky_radius;
      sky_pos[i][j][1] = cos(alt_sample) * sky_radius;
#endif
    }
  }

  scene_mutex.unlock();

  updateGL();
}
/* ------------------------------------------------------------ */
void GLDisplay::updateGLVisualization(void) {
  showbox = visualization.show_box;
  showboxAct->setChecked(showbox);
  showsphere = visualization.show_light;
  showsphereAct->setChecked(showsphere);
  showwireframe = visualization.show_wireframe;
  showwireframeAct->setChecked(showwireframe);
  setBackgroundColour();
}
/* ------------------------------------------------------------ */
void GLDisplay::mousePressEvent(QMouseEvent *event) {
  if ((event->buttons() & Qt::LeftButton) ||
      (event->buttons() & Qt::MiddleButton)) {
    mouse_x = event->x();
    mouse_y = event->y();
    updateGL();
  } else if (event->buttons() & Qt::RightButton) {
    popup_menu->exec(QCursor::pos());
  }
}
/* ------------------------------------------------------------ */
void GLDisplay::mouseMoveEvent(QMouseEvent *event) {
  if ((event->buttons() & Qt::LeftButton) && (event->modifiers() & Qt::SHIFT)) {
    // Panning
    xpos += -(mouse_x - event->x()) * (sky_radius * 0.01f);
    ypos += (mouse_y - event->y()) * (sky_radius * 0.01f);
    updateGL();
  } else if ((event->buttons() & Qt::LeftButton) &&
             (event->modifiers() & Qt::CTRL)) {
    // Zooming
    zpos += (mouse_y - event->y()) * (sky_radius * 0.01f);
    updateGL();
  } else if (event->buttons() & Qt::LeftButton) {
    // Rotation of XY plane
    xrot -= (mouse_y - (float)event->y()) * 0.5f;
    if (xrot >= 360.f)
      xrot -= 360.f;
    else if (xrot < 0.f)
      xrot += 360.f;

    yrot -= (mouse_x - (float)event->x()) * 0.5f;
    if (yrot >= 360.f)
      yrot -= 360.f;
    else if (yrot < 0.f)
      yrot += 360.f;
    updateGL();
  } else if ((event->buttons() & Qt::MiddleButton) &&
             (event->modifiers() & Qt::SHIFT)) {
    // Roll around z-axis
    zrot += (mouse_x - (float)event->x()) * 0.5f;
    if (zrot >= 360.f)
      zrot -= 360.f;
    else if (zrot < 0.f)
      zrot += 360.f;
    updateGL();
  } else if (event->buttons() & Qt::MiddleButton) {
    // Zooming
    zpos += (mouse_y - (float)event->y()) * (sky_radius * 0.01f);
    updateGL();
  }
  mouse_x = event->x();
  mouse_y = event->y();
}
/* ------------------------------------------------------------ */
void GLDisplay::showBoundingSphere(void) {
  if (showsphere) {
    showsphereAct->setChecked(FALSE);
    showsphere = 0;
  } else {
    showsphereAct->setChecked(TRUE);
    showsphere = 1;
  }
  updateGL();
}
/* ------------------------------------------------------------ */
void GLDisplay::showBoundingBox(void) {
  if (showbox) {
    showboxAct->setChecked(FALSE);
    showbox = 0;
  } else {
    showboxAct->setChecked(TRUE);
    showbox = 1;
  }
  updateGL();
}
/* ------------------------------------------------------------ */
void GLDisplay::showWireFrame(void) {
  if (showwireframe) {
    showwireframeAct->setChecked(FALSE);
    showwireframe = 0;
  } else {
    showwireframeAct->setChecked(TRUE);
    showwireframe = 1;
  }
  updateGL();
}

/* ------------------------------------------------------------ */
void GLDisplay::setBackgroundColour(void) {
  glClearColor(visualization.background[0], visualization.background[1],
               visualization.background[2], 0.0);
  updateGL();
}
/* ------------------------------------------------------------ */
void GLDisplay::outputVertices(void) {
  FILE *output_file = NULL;

  if ((output_file = fopen("vertices.dat", "w")) == NULL) {
    fprintf(stderr, "ERROR: cannot output vertices to file vertices.dat\n");
    return;
  }

  for (int i = 0; i < scene.num_primitives; i++) {
    fprintf(output_file, "%d: ", i + 1);
    fprintf(output_file, "%f %f %f  ", scene.primitives[i].data[0],
            scene.primitives[i].data[1], scene.primitives[i].data[2]);
    fprintf(output_file, "%f %f %f  ", scene.primitives[i].data[3],
            scene.primitives[i].data[4], scene.primitives[i].data[5]);
    fprintf(output_file, "%f %f %f  ", scene.primitives[i].data[6],
            scene.primitives[i].data[7], scene.primitives[i].data[8]);
    // check if this is a parallelogram
    if (scene.primitives[i].object_type == 4) {
      fprintf(output_file, "%f %f %f", scene.primitives[i].data[9],
              scene.primitives[i].data[10], scene.primitives[i].data[11]);
    }
    fprintf(output_file, "\n");
  }

  fclose(output_file);
  return;
}
/* ------------------------------------------------------------ */
void GLDisplay::outputImage(void) {
  QImage img = this->grabFrameBuffer();

  if (img.save("quasimc.png", "PNG") == FALSE)
    fprintf(stderr, "QuasiMC - cannot output image\n");

  return;
}

/* ------------------------------------------------------------ */
/* End of implementation of GLDisplay class */
/* ------------------------------------------------------------ */

/* ------------------------------------------------------------ */

void glDisplayRay(RAY ray) {
  if (num_rays < MAX_RAYS) {
    rays[num_rays][0] = ray.pt[0];
    rays[num_rays][1] = ray.pt[1];
    rays[num_rays][2] = ray.pt[2];

    if (!rays_from_objects) {
      // reverse dir, so ray is drawn from pt to light source
      rays[num_rays][3] = -ray.dir[0];
      rays[num_rays][4] = -ray.dir[1];
      rays[num_rays][5] = -ray.dir[2];
    } else {
      rays[num_rays][3] = ray.dir[0];
      rays[num_rays][4] = ray.dir[1];
      rays[num_rays][5] = ray.dir[2];
    }
    ++num_rays;
  }
  return;
}

/* ------------------------------------------------------------ */
static void DrawGrid(void)
// draws the grid the bounding box is divided into
// the loops could use 'floats' to update x,y,z but the precision
// sometimes causes the last lines to not be drawn
{
  float x, y, z;
  int i, j;

  glBegin(GL_LINES);

  // draw lines along Y-axis
  x = scene.grid.bbox[X];
  for (i = 0; i <= scene.grid.size[X]; i++) {
    z = scene.grid.bbox[Z];
    for (j = 0; j <= scene.grid.size[Z]; j++) {
      glVertex3f(x, scene.grid.bbox[1], z);
      glVertex3f(x, scene.grid.bbox[4], z);
      z += scene.grid.cell_size[Z];
    }
    x += scene.grid.cell_size[X];
  }

  // draw lines along X-axis
  y = scene.grid.bbox[Y];
  for (i = 0; i <= scene.grid.size[Y]; i++) {
    z = scene.grid.bbox[Z];
    for (j = 0; j <= scene.grid.size[Z]; j++) {
      glVertex3f(scene.grid.bbox[0], y, z);
      glVertex3f(scene.grid.bbox[3], y, z);
      z += scene.grid.cell_size[Z];
    }
    y += scene.grid.cell_size[Y];
  }

  // draw lines along X-axis
  x = scene.grid.bbox[X];
  for (i = 0; i <= scene.grid.size[X]; i++) {
    y = scene.grid.bbox[Y];
    for (j = 0; j <= scene.grid.size[Y]; j++) {
      glVertex3f(x, y, scene.grid.bbox[2]);
      glVertex3f(x, y, scene.grid.bbox[5]);
      y += scene.grid.cell_size[Y];
    }
    x += scene.grid.cell_size[X];
  }

  glEnd();
  return;
}
/* ------------------------------------------------------------ */
static void DrawSphere(void) {
  int i, j;
  float intensity_invert;

  if (use_sky_model && max_sky_intensity > 0.0)
    intensity_invert = 1.f / max_sky_intensity;
  else
    intensity_invert = 0.0;

  for (i = 0; i < AZIMUTH_SAMPLES; i++) {
    for (j = 0; j < ALTITUDE_SAMPLES; j++) {
      glPointSize(10.f * intensity[i][j] * intensity_invert + 0.1f);
      glBegin(GL_POINTS);
      glColor3f(intensity[i][j] * intensity_invert,
                intensity[i][j] * intensity_invert, 0.f);
#if NEW_SKY_MODEL == 0
      // change coordinate system from (x,y,z)
      // where +x is right, +y is back, and +z is up, as defined for CIE
      // luminance functions to display correctly in openGL (-y,z,-x)
      glVertex3f(-sky_pos[i][j][1], sky_pos[i][j][2], -sky_pos[i][j][0]);
#else
      glVertex3f(sky_pos[i][j][0], sky_pos[i][j][1], sky_pos[i][j][2]);
#endif
      glEnd();
    }
  }

  return;
}
/* ------------------------------------------------------------ */
