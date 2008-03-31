#include <cstdio>
#include <cstdlib>

#include <QFont>
#include <QGLWidget>
#include <QImage>
#include <QApplication>
#include <QDateTime>
#include <QKeyEvent>

#include "renderer.h"
#include "CConfigurator.h"
#include "CEngine.h"


#include "CStacksManager.h"
#include "utils.h"
#include "CRoomManager.h"
#include "CSquare.h"

#include "CFrustum.h"
#include "userfunc.h"


#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

GLfloat marker_colour[4] =  {1.0, 0.1, 0.1, 0.9};


#define MARKER_SIZE           (ROOM_SIZE/1.85)
#define PICK_TOL              50 

class CMainWindow *renderer_window;

RendererWidget::RendererWidget( QWidget *parent )
     : QGLWidget( parent )
{

  print_debug(DEBUG_RENDERER , "in renderer constructor");

  anglex = conf->get_renderer_angle_x();
  angley = conf->get_renderer_angle_y();
  anglez = conf->get_renderer_angle_z();
  userx = (GLfloat) conf->get_renderer_position_x();
  usery = (GLfloat) conf->get_renderer_position_y();
    
  userz = (GLfloat) conf->get_renderer_position_z();	/* additional shift added by user */

  if (userz == 0)
    userz = BASE_Z;

  curx = 0;
  cury = 0;
  curz = 0;			/* current rooms position */
  
  userLayerShift = 0;

  last_drawn_marker = 0;
  last_drawn_trail = 0;

  redraw = false;
}


void RendererWidget::initializeGL()
{
  unsigned int i;

  setMouseTracking(true);

  //textFont = new QFont("Times", 10, QFont::Bold);
  
  glShadeModel(GL_SMOOTH);
  glClearColor (0.0, 0.0, 0.0, 0.0);	/* This Will Clear The Background Color To Black */
  glPointSize (4.0);		/* Add point size, to make it clear */
  glLineWidth (2.0);		/* Add line width,   ditto */

  glEnable(GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  
  glEnable(GL_LINE_SMOOTH);    
  glEnable(GL_POLYGON_SMOOTH);  

  glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
  glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
  glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    

    
//    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_TEXTURE_ENV_MODE_REPLACE);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
  glPixelStorei(GL_RGBA, 1);


    print_debug(DEBUG_RENDERER, "in init()");
    

    
    basic_gllist = glGenLists(1);
    if (basic_gllist != 0) {
      glNewList(basic_gllist, GL_COMPILE);
      glRectf( -ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);          
      glEndList();
    }

    for (i = 0; i < conf->sectors.size(); i++) {
        conf->load_texture(&conf->sectors[i]);
    }
}


void RendererWidget::setupViewingModel(  int width, int height ) 
{
    gluPerspective(50.0f, (GLfloat) width / (GLfloat) height, 5.0f, conf->get_details_vis()*1.0f);
    glMatrixMode (GL_MODELVIEW);	
}


void RendererWidget::resizeGL( int width, int height )
{
    print_debug(DEBUG_RENDERER, "in resizeGL()");
  
    glViewport (0, 0, (GLint) width, (GLint) height);	
    glMatrixMode (GL_PROJECTION);	
    glLoadIdentity ();		
  
    setupViewingModel( width, height );
    
    redraw = true;
//    display();
}


void RendererWidget::paintGL()
{
    print_debug(DEBUG_RENDERER, "in paintGL()");
    QTime t;
    t.start();
    
    display();
    draw();
    
    
    print_debug(DEBUG_RENDERER, "Rendering's done. Time elapsed %d ms\r\n", t.elapsed());
}


/* mode 1 = full marker, mode 2 - partial marker */
void RendererWidget::drawMarker(int dx, int dy, int dz, int mode)
{
          /* upper */
      glBegin(GL_TRIANGLES);
      glVertex3f(               dx, MARKER_SIZE + dy + ROOM_SIZE,  0.0f + dz);
      glVertex3f(-MARKER_SIZE + dx,               dy + ROOM_SIZE,  0.0f + dz);
      glVertex3f(+MARKER_SIZE + dx,               dy + ROOM_SIZE,  0.0f + dz);
      glEnd();

      /* lower */
      glBegin(GL_TRIANGLES);
      glVertex3f(               dx,-MARKER_SIZE + dy - ROOM_SIZE,  0.0f + dz);
      glVertex3f(-MARKER_SIZE + dx,               dy - ROOM_SIZE,  0.0f + dz);
      glVertex3f(+MARKER_SIZE + dx,               dy - ROOM_SIZE,  0.0f + dz);
      glEnd();

      /* right */
      glBegin(GL_TRIANGLES);
      glVertex3f(               dx + ROOM_SIZE, +MARKER_SIZE + dy, 0.0f + dz);
      glVertex3f( MARKER_SIZE + dx + ROOM_SIZE,            dy,     0.0f + dz);
      glVertex3f(               dx + ROOM_SIZE, -MARKER_SIZE + dy, 0.0f + dz);
      glEnd();

      /* left */
      glBegin(GL_TRIANGLES);
      glVertex3f(               dx - ROOM_SIZE, +MARKER_SIZE + dy, 0.0f + dz);
      glVertex3f(-MARKER_SIZE + dx - ROOM_SIZE,            dy,     0.0f + dz);
      glVertex3f(               dx - ROOM_SIZE, -MARKER_SIZE + dy, 0.0f + dz);
      glEnd();
    

      if (mode == 1) {
        /* left */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy - ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE                      , dy - ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE                      , dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glEnd();

        /* right */
        glBegin(GL_QUADS);
        glVertex3f(dx + ROOM_SIZE                      , dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE                      , dy - ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy - ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glEnd();

        /* upper */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy + ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy + ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy + ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glEnd();

        /* lower */
        glBegin(GL_QUADS);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy - ROOM_SIZE                      , 0.0f + dz);
        glVertex3f(dx - ROOM_SIZE - (MARKER_SIZE / 3.5), dy - ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy - ROOM_SIZE + (MARKER_SIZE / 3.5), 0.0f + dz);
        glVertex3f(dx + ROOM_SIZE + (MARKER_SIZE / 3.5), dy - ROOM_SIZE                      , 0.0f + dz);
        glEnd();
    }
}


void RendererWidget::glDrawMarkers()
{
    CRoom *p;
    unsigned int k;
    
    int dx, dy, dz;

    if (stacker.amount() == 0)
        return;
    
    
    glColor4f(marker_colour[0],marker_colour[1],marker_colour[2],marker_colour[3]);
    for (k = 0; k < stacker.amount(); k++) {
        
        p = stacker.get(k);
    
        if (p == NULL) {
            print_debug(DEBUG_RENDERER, "RENDERER ERROR: Stuck upon corrupted room while drawing red pointers.\r\n");
            continue;
        }
    
        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;
    

        drawMarker(dx, dy, dz, 1);
    }
    
    
    if (last_drawn_marker != stacker.first()->id) {
        last_drawn_trail = last_drawn_marker;
        last_drawn_marker = stacker.first()->id;
    }

    if (last_drawn_trail) {
        glColor4f(marker_colour[0] / 1.1, marker_colour[1] / 1.3, marker_colour[2] / 1.3, marker_colour[3] / 1.3);
        p = Map.getRoom(last_drawn_trail);
        if (p != NULL) {
            dx = p->getX() - curx;
            dy = p->getY() - cury;
            dz = (p->getZ() - curz) ;
            drawMarker(dx, dy, dz, 2);
        }
    }
    
}




void RendererWidget::glDrawRoom(CRoom *p)
{
    GLfloat dx, dx2, dy, dy2, dz, dz2;
    CRoom *r;
    int k;
    float distance;
    bool details, texture;    
    QFont textFont("Times", 10, QFont::Bold);



    rooms_drawn_csquare++;
    
    dx = p->getX() - curx;
    dy = p->getY() - cury;
    dz = (p->getZ() - curz) /* * DIST_Z */;
    dx2 = 0;
    dy2 = 0;
    dz2 = 0;
    details = 1;
    texture = 1;
    
    if (frustum.isPointInFrustum(dx, dy, dz) != true)
      return;
    
    rooms_drawn_total++;


    distance = frustum.distance(dx, dy, dz);
    
    if (distance >= conf->get_details_vis()) 
      details = 0;

    if (distance >= conf->get_texture_vis()) 
      texture = 0;

    
    glTranslatef(dx, dy, dz);
    if (p->getTerrain() && texture) {

        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, conf->sectors[ p->getTerrain() ].texture);
        glCallList(conf->sectors[ p->getTerrain() ].gllist);  
        glDisable(GL_TEXTURE_2D);
        
       if (conf->get_display_regions_renderer() &&  engine->get_last_region() == p->getRegion()  ) {


            // The Regions rectangle around the room
            glColor4f(0.20, 0.20, 0.20, colour[3]-0.1);

            glRectf(-ROOM_SIZE*2, -ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE); // left  
            glRectf(ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE*2, ROOM_SIZE);   // right
            glRectf(-ROOM_SIZE, ROOM_SIZE, +ROOM_SIZE, ROOM_SIZE*2);  // upper 
            glRectf(-ROOM_SIZE, -ROOM_SIZE*2, +ROOM_SIZE, -ROOM_SIZE);   // lower

            glColor4f(colour[0], colour[1], colour[2], colour[3]);
        } 

        // slow version of selection drawing 
        // should be enough for start
        if (Map.selections.isSelected( p->id ) == true ) {
            glColor4f(0.20, 0.20, 0.80, colour[3]-0.1);
            glRectf(-ROOM_SIZE*2, -ROOM_SIZE*2, ROOM_SIZE*2, ROOM_SIZE*2); // left  
            glColor4f(colour[0], colour[1], colour[2], colour[3]);
        }
    } else {
        glCallList(basic_gllist);
    }              
    
    glTranslatef(-dx, -dy, -dz);



    if (details == 0)
      return;
    
    for (k = 0; k <= 5; k++) 
      if (p->isExitPresent(k) == true) {
        if (p->isExitNormal(k)) {
            GLfloat kx, ky, kz;
            GLfloat sx, sy;

            r = p->exits[k];

            dx2 = r->getX() - curx;
            dy2 = r->getY() - cury;
            dz2 = (r->getZ() - curz) /* * DIST_Z */;

            dx2 = (dx + dx2) / 2;
            dy2 = (dy + dy2) / 2;
            dz2 = (dz + dz2) / 2;

            if (k == NORTH) {
                kx = 0;
                ky = +(ROOM_SIZE + 0.2);
                kz = 0;
                sx = 0;
                sy = +ROOM_SIZE;
            } else if (k == EAST) {
                kx = +(ROOM_SIZE + 0.2);
                ky = 0;
                kz = 0;
                sx = +ROOM_SIZE;
                sy = 0;
            } else if (k == SOUTH) {
                kx = 0;
                ky = -(ROOM_SIZE + 0.2);
                kz = 0;
                sx = 0;
                sy = -ROOM_SIZE;
            } else if (k == WEST) {
                kx = -(ROOM_SIZE + 0.2);
                ky = 0;
                kz = 0;
                sx = -ROOM_SIZE;
                sy = 0;
            } else if (k == UP) {
                kx = 0;
                ky = 0;
                kz = +(ROOM_SIZE + 0.2);
                sx = 0;
                sy = 0;
            } else /*if (k == DOWN) */{
                kx = 0;
                ky = 0;
                kz = -(ROOM_SIZE + 0.2);
                sx = 0;
                sy = 0;
            } 
            if (p->getDoor(k) != "") {
                if (p->isDoorSecret(k) == false) {
                    glColor4f(0, 1.0, 0.0, colour[3] + 0.2);
                } else {
                    // Draw the secret door ...
                    QByteArray info;
                    QByteArray alias;                    
                    info = p->getDoor(k);
                    if (conf->get_show_regions_info() == true) {
                        alias = engine->get_users_region()->getAliasByDoor( info, k);
                        if (alias != "") {
                            info += " [";
                            info += alias;
                            info += "]";  
                        }
                        renderText((dx + dx2) / 2, (dy + dy2) / 2 , (dz +dz)/2 + ROOM_SIZE / 2 , info, textFont); 
                    }
                
                    glColor4f(1.0, 0.0, 0.0, colour[3] + 0.2);
                }
            }
                
            glBegin(GL_LINES);
            glVertex3f(dx + sx, dy + sy, dz);
            glVertex3f(dx + kx, dy + ky, dz + kz);
            glVertex3f(dx + kx, dy + ky, dz + kz);
            glVertex3f(dx2, dy2, dz2);
            glEnd();


            if (conf->get_show_notes_renderer() == true) {
                glColor4f( 0.4, 1.0, 0.98, colour[3]);
                renderText(dx, dy, dz + ROOM_SIZE / 2, p->getNote(), textFont);    
            }


            glColor4f(colour[0], colour[1], colour[2], colour[3]);

        } 

	  else {
            GLfloat kx, ky, kz;

            if (k == NORTH) {
                dx2 = dx;
                dy2 = dy + 0.5;
                dz2 = dz;
            } else if (k == EAST) {
                dx2 = dx + 0.5;
                dy2 = dy;
                dz2 = dz;
            } else if (k == SOUTH) {
                dx2 = dx;
                dy2 = dy - 0.5;
                dz2 = dz;
            } else if (k == WEST) {
                dx2 = dx - 0.5;
                dy2 = dy;
                dz2 = dz;
            } else if (k == UP) {
                dx2 = dx;
                dy2 = dy;
                dz2 = dz + 0.5;
            } else if (k == DOWN) {
                dx2 = dx;
                dy2 = dy;
                dz2 = dz - 0.5;
            }

            kx = 0;
            ky = 0;
            kz = 0;


            if (k == NORTH) {
                ky = +(ROOM_SIZE);
            } else if (k == EAST) {
                kx = +(ROOM_SIZE);
            } else if (k == SOUTH) {
                ky = -(ROOM_SIZE);
            } else if (k == WEST) {
                kx = -(ROOM_SIZE);
            } else if (k == UP) {
                kz = 0;
            } else if (k == DOWN) {
                kz = 0;
            }


            if (p->isExitUndefined(k)) {
              glColor4f(1.0, 1.0, 0.0, colour[3] + 0.2);
            } 
            
            if (p->isExitDeath(k)) {
                glColor4f(1.0, 0.0, 0.0, colour[3] + 0.2);
            } 

            glBegin(GL_LINES);
            glVertex3f(dx + kx, dy + ky, dz);
            glVertex3f(dx2 + kx, dy2 + ky, dz2);
            glEnd();
            
            GLuint death_terrain = conf->get_texture_by_desc("DEATH");
	    if (death_terrain && p->isExitDeath(k)) {
              glTranslatef(dx2 + kx, dy2 + ky, dz2);
              
              glEnable(GL_TEXTURE_2D);
              glBindTexture(GL_TEXTURE_2D, death_terrain);
        
              glBegin(GL_QUADS);
              
              glTexCoord2f(0.0, 1.0);
              glVertex3f(-ROOM_SIZE,  ROOM_SIZE, 0.0f);
              glTexCoord2f(0.0, 0.0);
              glVertex3f(-ROOM_SIZE, -ROOM_SIZE, 0.0f);
              glTexCoord2f(1.0, 0.0);
              glVertex3f( ROOM_SIZE, -ROOM_SIZE, 0.0f);
              glTexCoord2f(1.0, 1.0);
              glVertex3f( ROOM_SIZE,  ROOM_SIZE, 0.0f);
              
              glEnd();
              glDisable(GL_TEXTURE_2D);

              glTranslatef(-(dx2 + kx), -(dy2 + ky), -dz2);
              
            }

            glColor4f(colour[0], colour[1], colour[2], colour[3]);
            
        }
    }

        
    
}


// renderingMode serves for separating Pickup GLSELECT and normal GL_SELECT mode
void RendererWidget::glDrawCSquare(CSquare *p, int renderingMode)
{
    int k;
    
    if (!frustum.isSquareInFrustum(p)) {
        return; // this square is not in view 
    }
    
    if (p->toBePassed()) {
//         go deeper 
        if (p->subsquares[ CSquare::Left_Upper ])
            glDrawCSquare( p->subsquares[ CSquare::Left_Upper ], renderingMode);
        if (p->subsquares[ CSquare::Right_Upper ])
            glDrawCSquare( p->subsquares[ CSquare::Right_Upper ], renderingMode);
        if (p->subsquares[ CSquare::Left_Lower ])
            glDrawCSquare( p->subsquares[ CSquare::Left_Lower ], renderingMode);
        if (p->subsquares[ CSquare::Right_Lower ])
            glDrawCSquare( p->subsquares[ CSquare::Right_Lower ], renderingMode);
    } else {
        if (renderingMode == GL_SELECT) {
            for (k = 0; k < p->rooms.size(); k++) 
                renderPickupRoom(p->rooms[k]);
        } else {
            for (k = 0; k < p->rooms.size(); k++) 
                glDrawRoom(p->rooms[k]);
        }

    }
}


// this sets curx, cury, curz based on hrm internal rules
void RendererWidget::setupNewBaseCoordinates()
{
    CRoom *p = NULL;
    CRoom *newRoom = NULL;
    long long bestDistance, dist;
    int newX, newY, newZ;
    unsigned int i;

    print_debug(DEBUG_RENDERER, "calculating new Base coordinates");

    // in case we lost sync, stay at the last position 
    if (stacker.amount() == 0) 
        return;

    // initial unbeatably worst value for euclidean test
    bestDistance = 30000*30000*30000;
    for (i = 0; i < stacker.amount(); i++) {
        p = stacker.get(i);
        newX = curx - p->getX();
        newY = cury - p->getY();
        newZ = curz - p->getZ() + userLayerShift;
        dist = newX * newX + newY * newY + newZ * newZ; 
        if (dist < bestDistance) {
            bestDistance = dist;
            newRoom = p;
        }
    }

    curx = newRoom->getX();
    cury = newRoom->getY();
    curz = newRoom->getZ() + userLayerShift;
}

void RendererWidget::centerOnRoom(unsigned int id)
{
    CRoom *r = Map.getRoom(id);

    userx = (double) (curx - r->getX());
    usery = (double) (cury - r->getY());
    changeUserLayerShift(0 - (curz - r->getZ()));

/*
    curx = r->getX();
    cury = r->getY();
    curz = r->getZ();
    userx = 0;
    usery = 0;
    userLayerShift = 0;
*/

    toggle_renderer_reaction();
}

void RendererWidget::draw(void)
{
    CPlane *plane;  
    const float alphaChannelTable[] = { 0.85, 0.4, 0.37, 0.28, 0.25, 0.15, 0.15, 0.13, 0.1, 0.1, 0.1};
//                                       0    1     2      3    4      5     6    7    8     9    10 


    redraw = false;
    
    rooms_drawn_csquare=0;
    rooms_drawn_total=0;
//    square_frustum_checks = 0;
    
    int z = 0;
    
    print_debug(DEBUG_RENDERER, "in draw()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_DEPTH_TEST);    
    
    glColor3ub(255, 0, 0);

    glTranslatef(0, 0, userz);

    glRotatef(anglex, 1.0f, 0.0f, 0.0f);
    glRotatef(angley, 0.0f, 1.0f, 0.0f);
    glRotatef(anglez, 0.0f, 0.0f, 1.0f);
    glTranslatef(userx, usery, 0);

    setupNewBaseCoordinates(); 
    
    frustum.calculateFrustum(curx, cury, curz);
    
    // calculate the lower and the upper borders
    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;    

    lowerZ = curz - (side * 2);
    upperZ = curz + (side * 2);
    upperZ -= (1 - visibleLayers % 2) << 1; 
//    print_debug(DEBUG_RENDERER, "drawing %i rooms", Map.size());

    glColor4f(0.1, 0.8, 0.8, 0.4);
    colour[0] = 0.1; colour[1] = 0.8; colour[2] = 0.8; colour[3] = 0.4; 

    plane = Map.planes;
    while (plane) {
        if (plane->z < lowerZ || plane->z > upperZ) {
            plane = plane->next;
            continue;
        }
        
        z = plane->z - curz;
        if (z < 0)
            z = -z;

        if (z > 10) 
            z = 10;
        
        colour[0] = 1; colour[1] = 1; colour[2] = 1; 
        colour[3] = alphaChannelTable[z];

        glColor4f(colour[0], colour[1], colour[2], colour[3]);
        
        current_plane_z = plane->z;
        
        glDrawCSquare(plane->squares, GL_RENDER);
        plane = plane->next;
    }
    
    
//    print_debug(DEBUG_RENDERER, "Drawn %i rooms, after dot elimination %i, %i square frustum checks done", 
//            rooms_drawn_csquare, rooms_drawn_total, square_frustum_checks);
//    print_debug(DEBUG_RENDERER, "Drawing markers");

    glDrawMarkers();
    
//    print_debug(DEBUG_RENDERER, "draw() done");
  
    this->swapBuffers();
}

void RendererWidget::display(void)
{
  if (redraw) {
    QTime t;
    t.start();

    draw();
    print_debug(DEBUG_RENDERER, "Rendering's done. Time elapsed %d ms", t.elapsed());
  }  
}


void RendererWidget::renderPickupObjects()
{
    CPlane *plane;  
    int z = 0;
    
    print_debug(DEBUG_RENDERER, "in Object pickup fake draw()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);

    glColor3ub(255, 0, 0);

    glTranslatef(0, 0, userz);

    glRotatef(anglex, 1.0f, 0.0f, 0.0f);
    glRotatef(angley, 0.0f, 1.0f, 0.0f);
    glRotatef(anglez, 0.0f, 0.0f, 1.0f);
    glTranslatef(userx, usery, 0);

    glColor4f(0.1, 0.8, 0.8, 0.4);

    setupNewBaseCoordinates(); 


    frustum.calculateFrustum(curx, cury, curz);
    
    // calculate the lower and the upper borders
    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;    

    lowerZ = curz - (side * 2);
    upperZ = curz + (side * 2);
    upperZ -= (1 - visibleLayers % 2) << 1; 
//    print_debug(DEBUG_RENDERER, "drawing %i rooms", Map.size());

    plane = Map.planes;
    while (plane) {
        if (plane->z < lowerZ || plane->z > upperZ) {
            plane = plane->next;
            continue;
        }

        z = plane->z - curz;
        
        current_plane_z = plane->z;

        glDrawCSquare(plane->squares, GL_SELECT);
        plane = plane->next;
    }
    
}


void RendererWidget::renderPickupRoom(CRoom *p)
{
    GLfloat dx, dy, dz;
    
    dx = p->getX() - curx;
    dy = p->getY() - cury;
    dz = (p->getZ() - curz) /* * DIST_Z */;


    //if (p->id == 20989 || p->id == 20973) 
      //  printf("preparing to render the room in pickup mode! id %i\r\n", p->id);

    if (frustum.isPointInFrustum(dx, dy, dz) != true)
      return;

        
    //printf("Rendering the pickup room %i\r\n", p->id);

    glTranslatef(dx, dy, dz);
    glLoadName( p->id + 1 );
    glCallList(basic_gllist);
    glTranslatef(-dx, -dy, -dz);
}

bool RendererWidget::doSelect(QPoint pos, unsigned int &id)
{
    int viewport[4];
    int i;
    GLint   hits;
    GLint temphit = 32767;
    GLuint  zval;
    bool    selected;

    glSelectBuffer( MAXHITS, selectBuf );
    glRenderMode( GL_SELECT );
    glInitNames();

    glPushName( 0 );

    // setting up the viewing modell
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();

    glGetIntegerv( GL_VIEWPORT, viewport );
    gluPickMatrix( (double) pos.x(), (double) (viewport[3] - pos.y()), PICK_TOL, PICK_TOL, viewport);
    
    setupViewingModel( width(), height() );

    renderPickupObjects();

    // find the number of hits
    hits = glRenderMode( GL_RENDER );
    print_debug(DEBUG_INTERFACE, "Matches for selection click : %i\r\n", hits);    


    // reset viewing model ?
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    setupViewingModel( width(), height() );

    selected = false;
    if (hits <= 0) {
        return false;
    } else {
        zval = 50000;
        for ( i = 0; i < hits; i++) { // for each hit
            int tempId = selectBuf[ 4 * i + 3 ] - 1;
            CRoom *r = Map.getRoom( tempId );
            if (r == NULL) 
                continue;
            

            // and now the selection logic
            if (conf->getSelectOAnyLayer() ) {
                unsigned int val = abs(curz - r->getZ());
                // if we select on any layers ...
                // then favour the ones with minimal distance to our current layer
                if (val < zval ) {
                    zval = val;
                    temphit = selectBuf[ 4 * i + 3 ] - 1;
                    selected = true;
                }
            } else { 
                if (r->getZ() == curz ) {
                    zval = 0;
                    temphit = selectBuf[ 4 * i + 3 ] - 1;
                    selected = true;
                }
            }


        }

    }

    if (selected) 
        print_debug(DEBUG_INTERFACE, "Clicked on : %i", temphit);
    else 
        print_debug(DEBUG_INTERFACE, "Selection failed");
    id = temphit;

    return selected;
}



