/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *
 *  This program is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU General Public License
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include <QFont>
#include <QOpenGLWidget>
#include <QImage>
#include <QApplication>
#include <QDateTime>
#include <QKeyEvent>
#include <QTimer>
#include <QPainter>
#include <GL/glu.h>

#include "CConfigurator.h"
#include "utils.h"


#include "Renderer/renderer.h"
#include "Renderer/CSquare.h"
#include "Renderer/CFrustum.h"
#include "Renderer/GLPrimitives.h"

#include "Engine/CEngine.h"
#include "Engine/CStacksManager.h"

#include "Gui/mainwindow.h"

#include "Map/CRoomManager.h"

#include "Proxy/userfunc.h"

#include "GroupManager/CGroupChar.h"


#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#if defined(Q_CC_MSVC)
#pragma warning(disable:4305) // init: truncation from const double to float
#endif

GLfloat marker_colour[4] =  {1.0, 0.1, 0.1, 1.0};


#define MARKER_SIZE           (ROOM_SIZE/1.85)
#define CONNECTION_THICKNESS_DIVIDOR	  5
#define PICK_TOL              50 


RendererWidget::RendererWidget( QWidget *parent )
     : QOpenGLWidget( parent )
{
  print_debug(DEBUG_RENDERER , "in renderer constructor");

  // Disable transparency - we want a fully opaque OpenGL widget
  setAttribute(Qt::WA_OpaquePaintEvent, true);
  setAttribute(Qt::WA_NoSystemBackground, true);
  setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

  angleX = conf->getRendererAngleX();
  angleY = conf->getRendererAngleY();
  angleZ = conf->getRendererAngleZ();
  userX = (GLfloat) conf->getRendererPositionX();
  userY = (GLfloat) conf->getRendererPositionY();
  userZ = (GLfloat) conf->getRendererPositionZ();	/* additional shift added by user */

  if (userZ == 0)
    userZ = BASE_Z;

  curx = 0;
  cury = 0;
  curz = 0;			/* current rooms position */

  userLayerShift = 0;

  last_drawn_marker = 0;
  last_drawn_trail = 0;

  redraw = true;

  // Set up a timer to periodically trigger repaints
  // QOpenGLWidget needs this for reliable updates
  QTimer *updateTimer = new QTimer(this);
  connect(updateTimer, &QTimer::timeout, this, QOverload<>::of(&QOpenGLWidget::update));
  updateTimer->start(33);  // ~30 FPS
}


void RendererWidget::initializeGL()
{
	unsigned int i;

	initializeOpenGLFunctions();

	setMouseTracking(true);
    textFont = new QFont("Times", 12, QFont::Bold );


	//textFont = new QFont("Times", 10, QFont::Bold);

	glShadeModel(GL_SMOOTH);
	glClearColor (0.15, 0.15, 0.15, 1.0);	/* This Will Clear The Background Color To Dark Gray */
	glPointSize (4.0);		/* Add point size, to make it clear */
	glLineWidth (2.0);		/* Add line width,   ditto */

	glEnable(GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
	glHint(GL_POINT_SMOOTH_HINT, GL_NICEST);
	glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);

	glEnable(GL_LINE_SMOOTH);
	glEnable(GL_POLYGON_SMOOTH);

	glEnable(GL_MULTISAMPLE);

	#ifdef GL_SAMPLE_BUFFERS
		GLint bufs;
		GLint samples;
		glGetIntegerv(GL_SAMPLE_BUFFERS, &bufs);
		glGetIntegerv(GL_SAMPLES, &samples);
		print_debug(DEBUG_SYSTEM, "Using %d buffers and %d samples", bufs, samples);
		printf("Using %d buffers and %d samples", bufs, samples);
	#endif


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

    print_debug(DEBUG_RENDERER, "loading %i secto textures", conf->sectors.size());

    for (i = 0; i < conf->sectors.size(); i++) {
        conf->loadSectorTexture(&conf->sectors[i]);
    }

    // load the exits texture
    conf->loadNormalTexture("images/exit_normal.png", &conf->exit_normal_texture );
    conf->loadNormalTexture("images/exit_door.png", &conf->exit_door_texture );
    conf->loadNormalTexture("images/exit_secret.png", &conf->exit_secret_texture );
    conf->loadNormalTexture("images/exit_undef.png", &conf->exit_undef_texture );
}


void RendererWidget::setupViewingModel(  int width, int height ) 
{
    gluPerspective(60.0f, (GLfloat) width / (GLfloat) height, 1.0f, conf->getDetailsVisibility()*1.1f);
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
    // Qt will automatically call paintGL() after resize
}


void RendererWidget::display(void)
{
	print_debug(DEBUG_RENDERER, "display() called");
	redraw = true;
	QOpenGLWidget::update();  // Schedule a repaint through Qt's event system
}


void RendererWidget::paintGL()
{
    print_debug(DEBUG_RENDERER, "in paintGL()");

    // Always draw when Qt requests a repaint
    draw();
}


void RendererWidget::glDrawMarkers()
{
    CRoom *p;
    unsigned int k;
    QByteArray lastMovement;
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
    

        glDrawMarkerPrimitive(dx, dy, dz, 2);
        
        glTranslatef(dx, dy, dz + 0.2f);
        
        lastMovement = engine->getLastMovement();
        if (lastMovement.isEmpty() == false) {
        	float rotX = 0;
        	float rotY = 0;

        	if (lastMovement[0] == 'n') {
    			rotX = -90.0;
    		} else if (lastMovement[0] == 'e') {
    			rotY = 90.0;
    		} else if (lastMovement[0] == 's') {
    			rotX = 90.0;
    		} else if (lastMovement[0] == 'w') {
    			rotY = -90.0;
    		} else if (lastMovement[0] == 'd') {
    			rotX = 180.0;
    		} 
        	

        	glPushMatrix(); 
            glRotatef(rotX, 1.0f, 0.0f, 0.0f);
            glRotatef(rotY, 0.0f, 1.0f, 0.0f);
            //glRotatef(anglez, 0.0f, 0.0f, 1.0f);
        	glDrawConePrimitive();
        	glPopMatrix();
        } else {
        	glDrawConePrimitive();
        }
        
    }
    
    if (last_drawn_marker != stacker.first()->id) {
        last_drawn_trail = last_drawn_marker;
        last_drawn_marker = stacker.first()->id;
        renderer_window->getGroupManager()->setCharPosition(last_drawn_marker);
        //emit updateCharPosition(last_drawn_marker);
    }

    /*
    if (last_drawn_trail) {
        glColor4f(marker_colour[0] / 1.1, marker_colour[1] / 1.3, marker_colour[2] / 1.3, marker_colour[3] / 1.3);
        p = Map.getRoom(last_drawn_trail);
        if (p != NULL) {
            dx = p->getX() - curx;
            dy = p->getY() - cury;
            dz = (p->getZ() - curz) ;
            glDrawMarkerPrimitive(dx, dy, dz, 2);
        }
    }
    */
}

void RendererWidget::glDrawPrespamLine()
{
	if (!conf->getDrawPrespam())
		return;
	QVector<unsigned int> *line = engine->getPrespammedDirs();
    int prevx, prevy, prevz, dx, dy, dz;

	if (line == NULL)
		return;

    CRoom *p = Map.getRoom( line->at(0) );

    if (p == NULL) {
        return;
    }

    prevx = p->getX() - curx;
    prevy = p->getY() - cury;
    prevz = (p->getZ() - curz) /* * DIST_Z */;

    glColor4f(1.0, 0.0, 1.0, 1.0);

	for (int i = 1; i < line->size(); i++) {
		// connect all rooms with lines
		unsigned int id = line->at(i);

        CRoom *p = Map.getRoom(id);

        if (p == NULL)
            continue;

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        //glDrawMarkerPrimitive(dx, dy, dz, 2);

        //glTranslatef(dx, dy, dz + 0.2f);

        glBegin(GL_LINES);
        glVertex3f(prevx, prevy, prevz);
        glVertex3f(dx, dy, dz);
        glEnd();


        prevx = dx;
        prevy = dy;
        prevz = dz;
	}

	// and draw a cone in the last room
	glTranslatef(prevx, prevy, prevz + 0.2f);
	glDrawConePrimitive();

	// dispose
	delete line;
}


void RendererWidget::glDrawGroupMarkers()
{
    CRoom *p;
    QByteArray lastMovement;
    int dx, dy, dz;
    QVector<CGroupChar *>  chars;
    CGroupChar *ch;
    
    chars = renderer_window->getGroupManager()->getChars();
    if (chars.isEmpty())
    	return;

    for (int i = 0; i < chars.size(); i++) {
    	ch = chars[i];
    	unsigned int pos = ch->getPosition();

    	if (pos == 0)
    		continue;

    	if (last_drawn_marker == pos)
    		continue; // do not draw markers in the same room as our own marker
    	
        QColor color = ch->getColor();
                        
        double red = color.red()/255.;
        double green = color.green()/255.;
        double blue = color.blue()/255.;
        double alpha = color.alpha()/255.;

        glColor4f(red, green, blue, alpha);
        p = Map.getRoom(pos);

        if (p == NULL) {
            continue;
        }

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        glDrawMarkerPrimitive(dx, dy, dz, 2);
        
        glTranslatef(dx, dy, dz + 0.2f);
        
        lastMovement = ch->getLastMovement();
        if (lastMovement.isEmpty() == false) {
        	float rotX = 0;
        	float rotY = 0;

        	if (lastMovement[0] == 'n') {
    			rotX = -90.0;
    		} else if (lastMovement[0] == 'e') {
    			rotY = 90.0;
    		} else if (lastMovement[0] == 's') {
    			rotX = 90.0;
    		} else if (lastMovement[0] == 'w') {
    			rotY = -90.0;
    		} else if (lastMovement[0] == 'd') {
    			rotX = 180.0;
    		} 
        	

        	glPushMatrix(); 
            glRotatef(rotX, 1.0f, 0.0f, 0.0f);
            glRotatef(rotY, 0.0f, 1.0f, 0.0f);
            //glRotatef(anglez, 0.0f, 0.0f, 1.0f);
        	glDrawConePrimitive();
        	glPopMatrix();
        } else {
        	glDrawConePrimitive();
        }

        glTranslatef(-dx, -dy, -(dz + 0.2f));

    }
}

// TODO: removed:
// selection markers
// billboards of any kind
void RendererWidget::generateDisplayList(CSquare *square)
{
    GLfloat dx, dx2, dy, dy2, dz, dz2;
    CRoom *r;
    int k;

    square->clearDoorsList();
    square->clearNotesList();

    if (square->gllist == -1)
    	square->gllist = glGenLists(1);


    glNewList(square->gllist, GL_COMPILE);

	// generate gl list for the square here
    for (int ind = 0; ind < square->rooms.size(); ind++) {
        CRoom *p = square->rooms[ind];

        dx = p->getX() - square->centerx;
        dy = p->getY() - square->centery;
        dz = 0;

        glTranslatef(dx, dy, dz);
        if (p->getTerrain()) {

            glEnable(GL_TEXTURE_2D);
            glBindTexture(GL_TEXTURE_2D, conf->sectors[ p->getTerrain() ].texture );
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

        } else {
            glRectf( -ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
        }
        glTranslatef(-dx, -dy, -dz);


        if (p->getNote().isEmpty() != true) {
        	QColor color;
            if(p->getNoteColor() == "")
                color = QColor((QString)conf->getNoteColor());
            else
                color = QColor((QString)p->getNoteColor());

        	square->notesBillboards.append( new Billboard(p->getX(), p->getY(), p->getZ() + ROOM_SIZE / 2, p->getNote(), color) );
        }

        for (k = 0; k <= 5; k++)
          if (p->isExitPresent(k) == true) {
              GLfloat kx, ky, kz;
              GLfloat sx, sy, sz;
              GLuint exit_texture = conf->exit_normal_texture;
              int thickness = CONNECTION_THICKNESS_DIVIDOR;

              if (k == NORTH) {
                  kx = 0;
                  ky = +(ROOM_SIZE + 0.2);
                  kz = 0;
                  sx = 0;
                  sy = +ROOM_SIZE;
                  sz = 0;
              } else if (k == EAST) {
                  kx = +(ROOM_SIZE + 0.2);
                  ky = 0;
                  kz = 0;
                  sx = +ROOM_SIZE;
                  sy = 0;
                  sz = 0;
              } else if (k == SOUTH) {
                  kx = 0;
                  ky = -(ROOM_SIZE + 0.2);
                  kz = 0;
                  sx = 0;
                  sy = -ROOM_SIZE;
                  sz = 0;
              } else if (k == WEST) {
                  kx = -(ROOM_SIZE + 0.2);
                  ky = 0;
                  kz = 0;
                  sx = -ROOM_SIZE;
                  sy = 0;
                  sz = 0;
              } else if (k == UP) {
                  kx = 0;
                  ky = 0;
                  kz = +(ROOM_SIZE + 0.2);
                  sx = ROOM_SIZE / 2;
                  sy = 0;
                  sz = 0;
              } else {
                  kx = 0;
                  ky = 0;
                  kz = -(ROOM_SIZE + 0.2);
                  sx = 0;
                  sy = ROOM_SIZE / 2;
                  sz = 0;
              }

              if (p->isExitNormal(k)) {

                r = p->exits[k];

                dx2 = r->getX() - square->centerx;
                dy2 = r->getY() - square->centery;
                dz2 = r->getZ() - current_plane_z;

                dx2 = (dx + dx2) / 2;
                dy2 = (dy + dy2) / 2;
                dz2 = (dz + dz2) / 2;


                if (p->getDoor(k) != "") {
                    if (p->isDoorSecret(k) == false) {
                    	exit_texture = conf->exit_door_texture;
                    } else {
                    	exit_texture = conf->exit_secret_texture;

						// Draw the secret door ...
						QByteArray info;
						QByteArray alias;
						info = p->getDoor(k);
						alias = engine->get_users_region()->getAliasByDoor( info, k);
						if (alias != "" && p->getRegion() == engine->get_users_region() ) {
							info += " [";
							info += alias;
							info += "]";
						}

						double deltaX = (r->getX() - p->getX()) / 3.0;
						double deltaY = (r->getY() - p->getY()) / 3.0;
						double deltaZ = (r->getZ() - p->getZ()) / 3.0;

						double shiftX = 0;
						double shiftY = 0;
						double shiftZ = ROOM_SIZE / 2.0;

						if (deltaX < 0) {
							shiftY = ROOM_SIZE / 2.0;
						} else {
							shiftY = -ROOM_SIZE / 2.0;
						}

						if (deltaY != 0) {
							shiftX = -ROOM_SIZE;
						}

						if (deltaZ < 0) {
							shiftZ *= -1;
						}

						double x = p->getX() + deltaX + shiftX;
						double y = p->getY() + deltaY + shiftY;
						double z = p->getZ() + deltaZ + shiftZ;

			        	square->doorsBillboards.append( new Billboard( x, y , z, info, QColor(255, 255, 255, 255)) );
                    }
                }


                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, exit_texture  );

                glBegin(GL_QUAD_STRIP);
                glTexCoord2f(0.0, 1.0);
                glVertex3f(dx + sx - sy / thickness, dy + sy - sx / thickness, dz);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(dx + sx + sy / thickness, dy + sy + sx / thickness, dz);

				glTexCoord2f(0.5, 1.0);
				glVertex3f(dx + kx - sy / thickness, dy + ky - sx / thickness, dz + kz);
				glTexCoord2f(0.5, 0.0);
				glVertex3f(dx + kx + sy / thickness, dy + ky + sx / thickness, dz + kz);

				glTexCoord2f(1.0, 1.0);
				glVertex3f(dx2 - sy / thickness, dy2 - sx / thickness, dz2);
				glTexCoord2f(1.0, 0.0);
				glVertex3f(dx2 + sy / thickness, dy2 + sx / thickness, dz2);
                glEnd();

                glDisable(GL_TEXTURE_2D);

            } else {
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

            	exit_texture = conf->exit_undef_texture;

                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, exit_texture  );

                glBegin(GL_QUAD_STRIP);
                glTexCoord2f(0.0, 1.0);
                glVertex3f(dx + kx - sy / thickness, dy + ky - sx / thickness, dz);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(dx + kx + sy / thickness, dy + ky + sx / thickness, dz);

                glTexCoord2f(1.0, 1.0);
                glVertex3f(dx2 + kx - sy / thickness, dy2 + ky - sx / thickness, dz2);
                glTexCoord2f(1.0, 0.0);
                glVertex3f(dx2 + kx + sy / thickness, dy2 + ky + sx / thickness, dz2);
                glEnd();
                glDisable(GL_TEXTURE_2D);


                GLuint death_terrain = conf->getTextureByDesc("DEATH");
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

            }
        }
    }

    glEndList();

    square->rebuild_display_list = false;
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
        	if (p->rebuild_display_list)
        		generateDisplayList(p);

//        	glColor4f(1.0, 1.0, 1.0, 1.0);
        	// translate to the spot
        	int squarex = p->centerx - curx;
        	int squarey = p->centery - cury;
        	int squarez = current_plane_z - curz;

            glTranslatef(squarex, squarey, squarez);
            glCallList(p->gllist);
            glTranslatef( -squarex, -squarey, -squarez);


            // draw notes, if needed
            if (conf->getShowNotesRenderer() == true && p->notesBillboards.isEmpty() != true) {

                for (int n = 0; n < p->notesBillboards.size(); n++) {
                	Billboard *billboard = p->notesBillboards[n];

//                	GLfloat red = billboard->color.red() / 256;
//                	GLfloat green = billboard->color.green() / 256;
//                	GLfloat blue = billboard->color.blue() / 256;
                    GLfloat dx = billboard->x - curx;
                    GLfloat dy = billboard->y - cury;
                    GLfloat dz = billboard->z - curz;
                    Q_UNUSED(dx); Q_UNUSED(dy); Q_UNUSED(dz);

                    // TODO: renderText() not available in QOpenGLWidget
                    // Need to implement text rendering using QPainter or textures
                    // qglColor(QColor( billboard->color.red(), billboard->color.green(), billboard->color.blue(), colour[3] * 255 ) );
                    // renderText(dx, dy, dz, billboard->text, *textFont);
                }

            }

            // draw doors, if needed
            if (conf->getShowRegionsInfo() == true && p->doorsBillboards.isEmpty() != true) {

                // TODO: renderText() not available in QOpenGLWidget
                // Need to implement text rendering using QPainter or textures
                // glColor4f(1.0, 1.0, 1.0, colour[3]);
                // for (int n = 0; n < p->doorsBillboards.size(); n++) {
                // 	Billboard *billboard = p->doorsBillboards[n];
                //     renderText(billboard->x - curx, billboard->y - cury, billboard->z - curz, billboard->text, *textFont);
                // }
                Q_UNUSED(p);

            }

            // if needed, draw selected rooms
            if (!Map.selections.isEmpty()) {
				glColor4f(0.20, 0.20, 0.80, colour[3]-0.1);

				for (k = 0; k < p->rooms.size(); k++) {
					if (Map.selections.isSelected( p->rooms[k]->id ) == true ) {
	                    int dx = p->rooms[k]->getX() - curx;
	                    int dy = p->rooms[k]->getY() - cury;
	                    int dz = p->rooms[k]->getZ() - curz;
	                    glTranslatef(dx, dy, dz);
						glRectf(-ROOM_SIZE*2, -ROOM_SIZE*2, ROOM_SIZE*2, ROOM_SIZE*2); // left
	                    glTranslatef(-dx, -dy, -dz);
					}
                }
            }

            // if needed, draw current region
            if (conf->getDisplayRegionsRenderer()) {
				glColor4f(0.50, 0.50, 0.50, colour[3]-0.2);

				// generate gl list for the square here
			    for (int ind = 0; ind < p->rooms.size(); ind++) {
			        CRoom *room = p->rooms[ind];

			        if (room->getRegion() && room->getRegion() == engine->get_users_region() ) {
	                    int dx = room->getX() - curx;
	                    int dy = room->getY() - cury;
	                    int dz = room->getZ() - curz;
	                    glTranslatef(dx, dy, dz);
						glRectf(-ROOM_SIZE*1.5, -ROOM_SIZE*1.5, ROOM_SIZE*1.5, ROOM_SIZE*1.5); // left
	                    glTranslatef(-dx, -dy, -dz);
			        }

			    }
            }


			glColor4f(colour[0], colour[1], colour[2], colour[3]);

        }
    }
}


// this sets curx, cury, curz based on hrm internal rules
void RendererWidget::setupNewBaseCoordinates()
{
    CRoom *p = NULL;
    CRoom *newRoom = NULL;
    unsigned long long bestDistance, dist;
    int newX, newY, newZ;
    unsigned int i;

    print_debug(DEBUG_RENDERER, "calculating new Base coordinates");

    // in case we lost sync, stay at the last position 
    if (stacker.amount() == 0) 
        return;

    // initial unbeatably worst value for euclidean test
    bestDistance = (long long) 32000*32000*1000;
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

    if (newRoom != NULL) {
		curx = newRoom->getX();
		cury = newRoom->getY();
		curz = newRoom->getZ() + userLayerShift;

//		printf("Base room: %i\r\n", newRoom->id);
//    	fflush(stdout);
    } else {
//    	printf("No base room for coordinates setup found!\r\n");
//    	fflush(stdout);
    }
}

void RendererWidget::centerOnRoom(unsigned int id)
{
    CRoom *r = Map.getRoom(id);

    userX = (double) (curx - r->getX());
    userY = (double) (cury - r->getY());
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
    //const float alphaChannelTable[] = { 0.95, 0.35, 0.30, 0.28, 0.25, 0.15, 0.15, 0.13, 0.1, 0.1, 0.1};
    const float alphaChannelTable[] = { 0.95, 0.25, 0.20, 0.15, 0.10, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
//                                       0    1     2      3    4      5     6    7    8     9    10

    makeCurrent();

    redraw = false;
    int z = 0;

    print_debug(DEBUG_RENDERER, "in draw()");

    // Always clear the screen first to avoid artifacts
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Map.isBlocked()) {
    	// Map is blocked, just show cleared screen and retry later
    	printf("Map is blocked. Delaying the redraw\r\n");
    	fflush(stdout);
		print_debug(DEBUG_GENERAL, "Map is blocked. Delaying the redraw.");
		QTimer::singleShot( 500, this, SLOT( display() ) );
		return;
    }

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
//    glEnable(GL_DEPTH_TEST);    
    
//    glColor3ub(255, 0, 0);

    glTranslatef(0, 0, userZ);

    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(userX, userY, 0);

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
    

    plane = Map.getPlanes();
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

    glDrawMarkers();
    glDrawGroupMarkers();
    glDrawPrespamLine();


//    print_debug(DEBUG_RENDERER, "draw() done");
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

    glTranslatef(0, 0, userZ);

    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(userX, userY, 0);

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

    // Sensitive ! lock for WRITE!
//    Map.lockForRead();
    plane = Map.getPlanes();
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
//    Map.unlock();
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

    makeCurrent();  // Ensure GL context is active
    glSelectBuffer(MAXHITS, selectBuf);  // Set up selection buffer
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

