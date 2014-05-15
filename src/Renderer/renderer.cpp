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
#include <QGLWidget>
#include <QImage>
#include <QApplication>
#include <QDateTime>
#include <QKeyEvent>
#include <QTimer>
#include <GL/glu.h>

#include "CConfigurator.h"
#include "utils.h"


#include "Renderer/renderer.h"
#include "Renderer/CSquare.h"
#include "Renderer/CFrustum.h"

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

GLubyte halftone[] = {
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55,
  0xAA, 0xAA, 0xAA, 0xAA, 0x55, 0x55, 0x55, 0x55};


  GLubyte quadtoneline = 0x88;
  GLubyte quadtone[] = {
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22,
    0x88, 0x88, 0x88, 0x88, 0x22, 0x22, 0x22, 0x22};


#define MARKER_SIZE           (ROOM_SIZE/1.85)
#define CONNECTION_THICKNESS_DIVIDOR	  5
#define PICK_TOL              50 


RendererWidget::RendererWidget(CRoomManager *_map, QWidget *parent )
     : map(_map), QGLWidget( parent )
{
  print_debug(DEBUG_RENDERER , "in renderer constructor");

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

  redraw = false;
}


void RendererWidget::initializeGL()
{
	unsigned int i;

	setMouseTracking(true);
	setAutoBufferSwap( false );
    textFont = new QFont("Times", 12, QFont::Bold );
    textFont->setStyleHint(QFont::System, QFont::OpenGLCompatible);
    textFont->setStretch(QFont::Unstretched);

    glShadeModel(GL_FLAT);
    glClearColor (0.25, 0.25, 0.25, 0.0);	/* This Will Clear The Background Color To Black */
    glPointSize (3.0);		/* Add point size, to make it clear */
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
    conf->loadNormalTexture(":/textures/exit_normal.png", &exit_normal_texture );
    conf->loadNormalTexture(":/textures/exit_door.png", &exit_door_texture );
    conf->loadNormalTexture(":/textures/exit_secret.png", &exit_secret_texture );
    conf->loadNormalTexture(":/textures/exit_undef.png", &exit_undef_texture );


    //
    m_roomShadowPixmap = new QPixmap(30,30);
    m_roomShadowPixmap->fill(QColor(0,0,0,175));


    for (int i=0; i<16; i++)
    {
        m_terrainPixmaps[i] = new QPixmap(QString(":/textures/terrain%1.png").arg(i));
        m_roadPixmaps[i] = new QPixmap(QString(":/pixmaps/road%1.png").arg(i));
        m_loadPixmaps[i] = new QPixmap(QString(":/pixmaps/load%1.png").arg(i));
        m_mobPixmaps[i] = new QPixmap(QString(":/pixmaps/mob%1.png").arg(i));
        m_trailPixmaps[i] = new QPixmap(QString(":/pixmaps/trail%1.png").arg(i));
    }

}


QSize RendererWidget::minimumSizeHint() const
{
    return QSize(100, 100);
}

QSize RendererWidget::sizeHint() const
{
    return QSize(300, 200);
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
    // for some reason this is needed on windows
    display();
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


void RendererWidget::paintGL()
{
    print_debug(DEBUG_RENDERER, "in paintGL()");

    QTime t;
    t.start();

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
    QByteArray lastMovement;
    int dx, dy, dz;

    if (engine->getCandidatesAmount() == 0)
        return;
    
    
    glColor4f(marker_colour[0],marker_colour[1],marker_colour[2],marker_colour[3]);
    CStacksManager * stacker = engine->getStacker();
    for (k = 0; k < stacker->amount(); k++) {
        
        p = stacker->get(k);
    
        if (p == NULL) {
            print_debug(DEBUG_RENDERER, "RENDERER ERROR: Stuck upon corrupted room while drawing red pointers.\r\n");
            continue;
        }
    
        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;
    

        drawMarker(dx, dy, dz, 2);
        
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
        	drawCone();
        	glPopMatrix();
        } else {
        	drawCone();
        }
        
    }
    
    if (last_drawn_marker != stacker->first()->getId()) {
        last_drawn_trail = last_drawn_marker;
        last_drawn_marker = stacker->first()->getId();
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
            drawMarker(dx, dy, dz, 2);
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

    CRoom *p = map->getRoom( line->at(0) );

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

        CRoom *p = map->getRoom(id);

        if (p == NULL)
            continue;

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        //drawMarker(dx, dy, dz, 2);

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
	drawCone();

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
        p = map->getRoom(pos);

        if (p == NULL) {
            continue;
        }

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        drawMarker(dx, dy, dz, 2);
        
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
        	drawCone();
        	glPopMatrix();
        } else {
        	drawCone();
        }

        glTranslatef(-dx, -dy, -(dz + 0.2f));

    }
}


void RendererWidget::alphaOverlayTexture(const QString &texture)
{
    bindTexture(QPixmap(texture));
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
}

void RendererWidget::alphaOverlayTexture(QPixmap *pix)
{
    bindTexture(*pix);
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
}

// TODO: removed:
// selection markers
// billboards of any kind
void RendererWidget::generateDisplayList(CSquare *square)
{
    GLfloat dx, dx2, dy, dy2, dz, dz2;
    CRoom *r;
    ExitDirection k;

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

        quint32 roadindex = 0;
        if ( p->isExitFlagSet(ED_NORTH, EF_ROAD)) SET_BIT(roadindex, bit1);
        if ( p->isExitFlagSet(ED_SOUTH, EF_ROAD)) SET_BIT(roadindex, bit2);
        if ( p->isExitFlagSet(ED_EAST,  EF_ROAD)) SET_BIT(roadindex, bit3);
        if ( p->isExitFlagSet(ED_WEST,  EF_ROAD)) SET_BIT(roadindex, bit4);



        glPushMatrix();
        glTranslatef(dx, dy, dz);
        if (p->getTerrain()) {


            glEnable(GL_TEXTURE_2D);
            if ( p->getTerrain() == RTT_ROAD)
                bindTexture(*m_roadPixmaps[roadindex]);
            else
                glBindTexture(GL_TEXTURE_2D, conf->sectors[ p->getTerrain() ].texture );
                //bindTexture(*m_terrainPixmaps[ p->getTerrain() ]);
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

        } else {
            glRectf( -ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
        }

        glEnable(GL_BLEND);

        // Make dark rooms look dark
        if (p->getLightType() == RLT_DARK)
        {
            GLdouble oldcolour[4];
            glGetDoublev(GL_CURRENT_COLOR, oldcolour);

            glTranslated(0, 0, 0.005);

            glColor4d(0.2f, 0.0f, 0.0f, 0.6f);
            const float kantik = 1.1f;

            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glBegin(GL_QUADS);
                glTexCoord2f(0.0, 1.0);
                glVertex3f(-ROOM_SIZE * kantik,  ROOM_SIZE * kantik, 0.0f);
                glTexCoord2f(0.0, 0.0);
                glVertex3f(-ROOM_SIZE * kantik, -ROOM_SIZE * kantik, 0.0f);
                glTexCoord2f(1.0, 0.0);
                glVertex3f( ROOM_SIZE * kantik, -ROOM_SIZE * kantik, 0.0f);
                glTexCoord2f(1.0, 1.0);
                glVertex3f( ROOM_SIZE * kantik,  ROOM_SIZE * kantik, 0.0f);
            glEnd();

            glColor4d(oldcolour[0], oldcolour[1], oldcolour[2], oldcolour[3]);
        }

        // Draw a little red cross on noride rooms
        if (p->getRidableType()==RRT_NOTRIDABLE)
        {
            GLdouble oldcolour[4];
            glGetDoublev(GL_CURRENT_COLOR, oldcolour);

            qglColor(Qt::red);
            glBegin(GL_LINES);
            glVertex3d(0.6, 0.2, 0.005);
            glVertex3d(0.8, 0.4, 0.005);
            glVertex3d(0.8, 0.2, 0.005);
            glVertex3d(0.6, 0.4, 0.005);
            glEnd();

            glColor4d(oldcolour[0], oldcolour[1], oldcolour[2], oldcolour[3]);
        }

        // Trail Support
        glTranslated(0, 0, 0.005);
        if (roadindex > 0 && (p->getTerrain()) != RTT_ROAD) {
            alphaOverlayTexture(m_trailPixmaps[roadindex]);
            glTranslated(0, 0, 0.005);
        }

        //RMF_RENT, RMF_SHOP, RMF_WEAPONSHOP, RMF_ARMOURSHOP, RMF_FOODSHOP, RMF_PETSHOP,
        //RMF_GUILD, RMF_SCOUTGUILD, RMF_MAGEGUILD, RMF_CLERICGUILD, RMF_WARRIORGUILD,
        //RMF_RANGERGUILD, RMF_SMOB, RMF_QUEST, RMF_ANY
        if (p->isMobFlagSet(RMF_RENT))
            alphaOverlayTexture(m_mobPixmaps[0]);
        if (p->isMobFlagSet(RMF_SHOP))
            alphaOverlayTexture(m_mobPixmaps[1]);
        if (p->isMobFlagSet(RMF_WEAPONSHOP))
            alphaOverlayTexture(m_mobPixmaps[2]);
        if (p->isMobFlagSet(RMF_ARMOURSHOP))
            alphaOverlayTexture(m_mobPixmaps[3]);
        if (p->isMobFlagSet(RMF_FOODSHOP))
            alphaOverlayTexture(m_mobPixmaps[4]);
        if (p->isMobFlagSet(RMF_PETSHOP))
            alphaOverlayTexture(m_mobPixmaps[5]);
        if (p->isMobFlagSet(RMF_GUILD))
            alphaOverlayTexture(m_mobPixmaps[6]);
        if (p->isMobFlagSet(RMF_SCOUTGUILD))
            alphaOverlayTexture(m_mobPixmaps[7]);
        if (p->isMobFlagSet(RMF_MAGEGUILD))
            alphaOverlayTexture(m_mobPixmaps[8]);
        if (p->isMobFlagSet(RMF_CLERICGUILD))
            alphaOverlayTexture(m_mobPixmaps[9]);
        if (p->isMobFlagSet(RMF_WARRIORGUILD))
            alphaOverlayTexture(m_mobPixmaps[10]);
        if (p->isMobFlagSet(RMF_RANGERGUILD))
            alphaOverlayTexture(m_mobPixmaps[11]);
        if (p->isMobFlagSet(RMF_SMOB))
            alphaOverlayTexture(m_mobPixmaps[12]);
        if (p->isMobFlagSet(RMF_QUEST))
            alphaOverlayTexture(m_mobPixmaps[13]);
        if (p->isMobFlagSet(RMF_ANY))
            alphaOverlayTexture(m_mobPixmaps[14]);

        //RLF_TREASURE, RLF_ARMOUR, RLF_WEAPON, RLF_WATER, RLF_FOOD, RLF_HERB
        //RLF_KEY, RLF_MULE, RLF_HORSE, RLF_PACKHORSE, RLF_TRAINEDHORSE
        //RLF_ROHIRRIM, RLF_WARG, RLF_BOAT
        glTranslated(0, 0, 0.005);
        if (p->isLoadFlagSet(RLF_TREASURE))
            alphaOverlayTexture(m_loadPixmaps[0]);
        if (p->isLoadFlagSet(RLF_ARMOUR))
            alphaOverlayTexture(m_loadPixmaps[1]);
        if (p->isLoadFlagSet(RLF_WEAPON))
            alphaOverlayTexture(m_loadPixmaps[2]);
        if (p->isLoadFlagSet(RLF_WATER))
            alphaOverlayTexture(m_loadPixmaps[3]);
        if (p->isLoadFlagSet(RLF_FOOD))
            alphaOverlayTexture(m_loadPixmaps[4]);
        if (p->isLoadFlagSet(RLF_HERB))
            alphaOverlayTexture(m_loadPixmaps[5]);
        if (p->isLoadFlagSet(RLF_KEY))
            alphaOverlayTexture(m_loadPixmaps[6]);
        if (p->isLoadFlagSet(RLF_MULE))
            alphaOverlayTexture(m_loadPixmaps[7]);
        if (p->isLoadFlagSet(RLF_HORSE))
            alphaOverlayTexture(m_loadPixmaps[8]);
        if (p->isLoadFlagSet(RLF_PACKHORSE))
            alphaOverlayTexture(m_loadPixmaps[9]);
        if (p->isLoadFlagSet(RLF_TRAINEDHORSE))
            alphaOverlayTexture(m_loadPixmaps[10]);
        if (p->isLoadFlagSet(RLF_ROHIRRIM))
            alphaOverlayTexture(m_loadPixmaps[11]);
        if (p->isLoadFlagSet(RLF_WARG))
            alphaOverlayTexture(m_loadPixmaps[12]);
        if (p->isLoadFlagSet(RLF_BOAT))
            alphaOverlayTexture(m_loadPixmaps[13]);

        glTranslated(0, 0, 0.005);
        if (p->isLoadFlagSet(RLF_ATTENTION))
            alphaOverlayTexture(m_loadPixmaps[14]);
        if (p->isLoadFlagSet(RLF_TOWER))
            alphaOverlayTexture(m_loadPixmaps[15]);


        glDisable(GL_TEXTURE_2D);


        // jump back to the old coordinates
        glPopMatrix();
        //glTranslatef(-dx, -dy, -dz);


        if (p->getNote().isEmpty() != true) {
        	QColor color;
            if(p->getNoteColor() == "")
                color = QColor((QString)conf->getNoteColor());
            else
                color = QColor((QString)p->getNoteColor());


        	square->notesBillboards.append( new Billboard(p->getX(), p->getY(), p->getZ() + ROOM_SIZE / 2, p->getNote(), color) );
        }

        for (int kind = 0; kind <= 5; kind++) {
            ExitDirection k = static_cast<ExitDirection>(kind);
          if (p->isExitPresent(k) == true) {
              GLfloat kx, ky, kz;
              GLfloat sx, sy, sz;
              GLuint exit_texture = exit_normal_texture;
              int thickness = CONNECTION_THICKNESS_DIVIDOR;

              if (k == ED_NORTH) {
                  kx = 0;
                  ky = +(ROOM_SIZE + 0.2);
                  kz = 0;
                  sx = 0;
                  sy = +ROOM_SIZE;
                  sz = 0;
              } else if (k == ED_EAST) {
                  kx = +(ROOM_SIZE + 0.2);
                  ky = 0;
                  kz = 0;
                  sx = +ROOM_SIZE;
                  sy = 0;
                  sz = 0;
              } else if (k == ED_SOUTH) {
                  kx = 0;
                  ky = -(ROOM_SIZE + 0.2);
                  kz = 0;
                  sx = 0;
                  sy = -ROOM_SIZE;
                  sz = 0;
              } else if (k == ED_WEST) {
                  kx = -(ROOM_SIZE + 0.2);
                  ky = 0;
                  kz = 0;
                  sx = -ROOM_SIZE;
                  sy = 0;
                  sz = 0;
              } else if (k == ED_UP) {
                  kx = 0;
                  ky = 0;
                  kz = +(ROOM_SIZE + 0.2);
                  sx = ROOM_SIZE / 2;
                  sy = 0;
                  sz = 0;
              } else if (k == ED_DOWN) {
                  kx = 0;
                  ky = 0;
                  kz = -(ROOM_SIZE + 0.2);
                  sx = 0;
                  sy = ROOM_SIZE / 2;
                  sz = 0;
              }

              if (p->isExitNormal(k)) {

                r = p->getExitRoom(k);

                dx2 = r->getX() - square->centerx;
                dy2 = r->getY() - square->centery;
                dz2 = r->getZ() - current_plane_z;

                dx2 = (dx + dx2) / 2;
                dy2 = (dy + dy2) / 2;
                dz2 = (dz + dz2) / 2;


                if (p->getDoor(k) != "") {
                    if (p->isDoorSecret(k) == false) {
                        exit_texture = exit_door_texture;
                    } else {
                        exit_texture = exit_secret_texture;

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


                        bool twinSecret = false;
                        CRoom *other = p->getExitRoom(k);
                        if (other != NULL) {
                            ExitDirection revDir = reversenum(k);
                            if (other->getDoor(revDir) == p->getDoor(k))
                                twinSecret = true;
                        }

                        double divFactor = 3.0;
                        if (twinSecret) {
                            divFactor = 2.0;
                        }
                        double deltaX = (r->getX() - p->getX()) / divFactor;
                        double deltaY = (r->getY() - p->getY()) / divFactor;
                        double deltaZ = (r->getZ() - p->getZ()) / divFactor;

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

                        // in case of "twinSecret", i.e. same named secret doors in both rooms
                        // only draw the billboard once, and in room with higher ID
                        if (!twinSecret || other->getId() < p->getId() )
                            square->doorsBillboards.append( new Billboard( x, y , z, info, QColor(255, 255, 255, 255)) );
                    }
                }


                glEnable(GL_TEXTURE_2D);
                glBindTexture(GL_TEXTURE_2D, exit_texture);

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

                if (k == ED_NORTH) {
                    dx2 = dx;
                    dy2 = dy + 0.5;
                    dz2 = dz;
                } else if (k == ED_EAST) {
                    dx2 = dx + 0.5;
                    dy2 = dy;
                    dz2 = dz;
                } else if (k == ED_SOUTH) {
                    dx2 = dx;
                    dy2 = dy - 0.5;
                    dz2 = dz;
                } else if (k == ED_WEST) {
                    dx2 = dx - 0.5;
                    dy2 = dy;
                    dz2 = dz;
                } else if (k == ED_UP) {
                    dx2 = dx;
                    dy2 = dy;
                    dz2 = dz + 0.5;
                } else if (k == ED_DOWN) {
                    dx2 = dx;
                    dy2 = dy;
                    dz2 = dz - 0.5;
                }

                kx = 0;
                ky = 0;
                kz = 0;


                if (k == ED_NORTH) {
                    ky = +(ROOM_SIZE);
                } else if (k == ED_EAST) {
                    kx = +(ROOM_SIZE);
                } else if (k == ED_SOUTH) {
                    ky = -(ROOM_SIZE);
                } else if (k == ED_WEST) {
                    kx = -(ROOM_SIZE);
                } else if (k == ED_UP) {
                    kz = 0;
                } else if (k == ED_DOWN) {
                    kz = 0;
                }

                exit_texture = exit_undef_texture;

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


                GLuint death_terrain = conf->getTerrainTextureByDesc("DEATH");
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

//                    glColor4f(red, green, blue, colour[3] + 0.2);
                    qglColor(QColor( billboard->color.red(), billboard->color.green(), billboard->color.blue(), colour[3] * 255 ) );
                    renderText(dx, dy, dz, billboard->text, *textFont);
                }

            }

            // draw doors, if needed
            if (conf->getShowRegionsInfo() == true && p->doorsBillboards.isEmpty() != true) {

//                qglColor( QColor( 255, 255, 255, colour[3] * 255 ) );
				glColor4f(1.0, 1.0, 1.0, colour[3]);
                for (int n = 0; n < p->doorsBillboards.size(); n++) {
                	Billboard *billboard = p->doorsBillboards[n];

                    renderText(billboard->x - curx, billboard->y - cury, billboard->z - curz, billboard->text, *textFont);
                }

            }

            // if needed, draw selected rooms
            if (!map->selections.isEmpty()) {
				glColor4f(0.20, 0.20, 0.80, colour[3]-0.1);

				for (k = 0; k < p->rooms.size(); k++) {
                    if (map->selections.isSelected( p->rooms[k]->getId() ) == true ) {
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
    if (engine->getCandidatesAmount() == 0)
        return;

    // initial unbeatably worst value for euclidean test
    bestDistance = (long long) 32000*32000*1000;
    for (i = 0; i < engine->getStacker()->amount(); i++) {
        p = engine->getStacker()->get(i);
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

void RendererWidget::centerOnRoom(RoomId id)
{
    CRoom *r = map->getRoom(id);

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

    if (map->isBlocked()) {
    	// well, not much we can do - ignore the message
    	printf("Map is blocked. Delaying the redraw\r\n");
    	fflush(stdout);
		print_debug(DEBUG_GENERAL, "Map is blocked. Delaying the redraw.");
		QTimer::singleShot( 500, this, SLOT( display() ) );
		return;
    }


    makeCurrent();

    redraw = false;

    int z = 0;

    print_debug(DEBUG_RENDERER, "in draw()");
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
    

    plane = map->getPlanes();
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
    this->swapBuffers();
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
    plane = map->getPlanes();
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
    glLoadName( p->getId() + 1 );
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
            CRoom *r = map->getRoom( tempId );
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




void RendererWidget::drawCone()
{
    glNormal3f(0.634392, 0.773011, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.036944, 0.059248, -0.361754);
        glVertex3f(0.036944, 0.059248, -0.000645);
        glVertex3f(0.046794, 0.051164, -0.000645);
        glVertex3f(0.046794, 0.051164, -0.361754);
    glEnd();

    glNormal3f(0.471394, 0.881923, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.025707, 0.065254, -0.361754);
        glVertex3f(0.025706, 0.065255, -0.000645);
        glVertex3f(0.036944, 0.059248, -0.000645);
        glVertex3f(0.036944, 0.059248, -0.361754);
    glEnd();

    glNormal3f(0.290282, 0.956941, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.013513, 0.068953, -0.361754);
        glVertex3f(0.013512, 0.068953, -0.000645);
        glVertex3f(0.025706, 0.065255, -0.000645);
        glVertex3f(0.025707, 0.065254, -0.361754);
    glEnd();

    glNormal3f(0.098014, 0.995185, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.070202, -0.361754);
        glVertex3f(0.000832, 0.070202, -0.000645);
        glVertex3f(0.013512, 0.068953, -0.000645);
        glVertex3f(0.013513, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.098020, 0.995184, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.011849, 0.068953, -0.361754);
        glVertex3f(-0.011849, 0.068953, -0.000645);
        glVertex3f(0.000832, 0.070202, -0.000645);
        glVertex3f(0.000832, 0.070202, -0.361754);
    glEnd();

    glNormal3f(-0.290287, 0.956940, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.024042, 0.065254, -0.361754);
        glVertex3f(-0.024043, 0.065254, -0.000645);
        glVertex3f(-0.011849, 0.068953, -0.000645);
        glVertex3f(-0.011849, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.471399, 0.881920, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.035280, 0.059248, -0.361754);
        glVertex3f(-0.035280, 0.059247, -0.000645);
        glVertex3f(-0.024043, 0.065254, -0.000645);
        glVertex3f(-0.024042, 0.065254, -0.361754);
    glEnd();

    glNormal3f(-0.634395, 0.773009, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.045130, 0.051164, -0.361754);
        glVertex3f(-0.045130, 0.051164, -0.000645);
        glVertex3f(-0.035280, 0.059247, -0.000645);
        glVertex3f(-0.035280, 0.059248, -0.361754);
    glEnd();

    glNormal3f(-0.773012, 0.634392, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.053213, 0.041314, -0.361754);
        glVertex3f(-0.053213, 0.041314, -0.000645);
        glVertex3f(-0.045130, 0.051164, -0.000645);
        glVertex3f(-0.045130, 0.051164, -0.361754);
    glEnd();

    glNormal3f(-0.881922, 0.471395, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.059220, 0.030077, -0.361754);
        glVertex3f(-0.059220, 0.030076, -0.000645);
        glVertex3f(-0.053213, 0.041314, -0.000645);
        glVertex3f(-0.053213, 0.041314, -0.361754);
    glEnd();

    glNormal3f(-0.956941, 0.290283, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.062919, 0.017883, -0.361754);
        glVertex3f(-0.062919, 0.017883, -0.000645);
        glVertex3f(-0.059220, 0.030076, -0.000645);
        glVertex3f(-0.059220, 0.030077, -0.361754);
    glEnd();

    glNormal3f(-0.995185, 0.098016, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.064168, 0.005202, -0.361754);
        glVertex3f(-0.064168, 0.005202, -0.000645);
        glVertex3f(-0.062919, 0.017883, -0.000645);
        glVertex3f(-0.062919, 0.017883, -0.361754);
    glEnd();

    glNormal3f(-0.995185, -0.098018, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.062919, -0.007478, -0.361754);
        glVertex3f(-0.062919, -0.007479, -0.000645);
        glVertex3f(-0.064168, 0.005202, -0.000645);
        glVertex3f(-0.064168, 0.005202, -0.361754);
    glEnd();

    glNormal3f(-0.956940, -0.290286, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.059220, -0.019672, -0.361754);
        glVertex3f(-0.059220, -0.019672, -0.000645);
        glVertex3f(-0.062919, -0.007479, -0.000645);
        glVertex3f(-0.062919, -0.007478, -0.361754);
    glEnd();

    glNormal3f(-0.881921, -0.471398, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.053213, -0.030909, -0.361754);
        glVertex3f(-0.053213, -0.030910, -0.000645);
        glVertex3f(-0.059220, -0.019672, -0.000645);
        glVertex3f(-0.059220, -0.019672, -0.361754);
    glEnd();

    glNormal3f(-0.773010, -0.634394, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.045130, -0.040759, -0.361754);
        glVertex3f(-0.045129, -0.040759, -0.000645);
        glVertex3f(-0.053213, -0.030910, -0.000645);
        glVertex3f(-0.053213, -0.030909, -0.361754);
    glEnd();

    glNormal3f(-0.634392, -0.773011, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.035280, -0.048843, -0.361754);
        glVertex3f(-0.035280, -0.048843, -0.000645);
        glVertex3f(-0.045129, -0.040759, -0.000645);
        glVertex3f(-0.045130, -0.040759, -0.361754);
    glEnd();

    glNormal3f(-0.471396, -0.881922, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.024042, -0.054850, -0.361754);
        glVertex3f(-0.024042, -0.054850, -0.000645);
        glVertex3f(-0.035280, -0.048843, -0.000645);
        glVertex3f(-0.035280, -0.048843, -0.361754);
    glEnd();

    glNormal3f(-0.290285, -0.956940, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(-0.011849, -0.058548, -0.361754);
        glVertex3f(-0.011849, -0.058548, -0.000645);
        glVertex3f(-0.024042, -0.054850, -0.000645);
        glVertex3f(-0.024042, -0.054850, -0.361754);
    glEnd();

    glNormal3f(-0.098016, -0.995185, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, -0.059797, -0.361754);
        glVertex3f(0.000832, -0.059797, -0.000645);
        glVertex3f(-0.011849, -0.058548, -0.000645);
        glVertex3f(-0.011849, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.098017, -0.995185, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.013513, -0.058548, -0.361754);
        glVertex3f(0.013513, -0.058548, -0.000645);
        glVertex3f(0.000832, -0.059797, -0.000645);
        glVertex3f(0.000832, -0.059797, -0.361754);
    glEnd();

    glNormal3f(0.290284, -0.956940, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.025706, -0.054850, -0.361754);
        glVertex3f(0.025706, -0.054850, -0.000645);
        glVertex3f(0.013513, -0.058548, -0.000645);
        glVertex3f(0.013513, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.471397, -0.881921, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.036944, -0.048843, -0.361754);
        glVertex3f(0.036944, -0.048843, -0.000645);
        glVertex3f(0.025706, -0.054850, -0.000645);
        glVertex3f(0.025706, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.634393, -0.773011, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.046794, -0.040759, -0.361754);
        glVertex3f(0.046794, -0.040759, -0.000645);
        glVertex3f(0.036944, -0.048843, -0.000645);
        glVertex3f(0.036944, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.773010, -0.634394, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.054877, -0.030910, -0.361754);
        glVertex3f(0.054877, -0.030910, -0.000645);
        glVertex3f(0.046794, -0.040759, -0.000645);
        glVertex3f(0.046794, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.881921, -0.471397, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.060884, -0.019672, -0.361754);
        glVertex3f(0.060884, -0.019672, -0.000645);
        glVertex3f(0.054877, -0.030910, -0.000645);
        glVertex3f(0.054877, -0.030910, -0.361754);
    glEnd();

    glNormal3f(0.956940, -0.290285, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.064583, -0.007478, -0.361754);
        glVertex3f(0.064583, -0.007479, -0.000645);
        glVertex3f(0.060884, -0.019672, -0.000645);
        glVertex3f(0.060884, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.995185, -0.098018, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.065832, 0.005202, -0.361754);
        glVertex3f(0.065832, 0.005202, -0.000645);
        glVertex3f(0.064583, -0.007479, -0.000645);
        glVertex3f(0.064583, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.995185, 0.098015, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.064583, 0.017883, -0.361754);
        glVertex3f(0.064583, 0.017883, -0.000645);
        glVertex3f(0.065832, 0.005202, -0.000645);
        glVertex3f(0.065832, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.956941, 0.290284, -0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.060884, 0.030077, -0.361754);
        glVertex3f(0.060884, 0.030077, -0.000645);
        glVertex3f(0.064583, 0.017883, -0.000645);
        glVertex3f(0.064583, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.881922, 0.471396, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.054877, 0.041314, -0.361754);
        glVertex3f(0.054878, 0.041314, -0.000645);
        glVertex3f(0.060884, 0.030077, -0.000645);
        glVertex3f(0.060884, 0.030077, -0.361754);
    glEnd();

    glNormal3f(0.773011, 0.634393, 0.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.046794, 0.051164, -0.361754);
        glVertex3f(0.046794, 0.051164, -0.000645);
        glVertex3f(0.054878, 0.041314, -0.000645);
        glVertex3f(0.054877, 0.041314, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.046794, 0.051164, -0.000645);
        glVertex3f(0.036944, 0.059248, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.036944, 0.059248, -0.361754);
        glVertex3f(0.046794, 0.051164, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.036944, 0.059248, -0.000645);
        glVertex3f(0.025706, 0.065255, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.025707, 0.065254, -0.361754);
        glVertex3f(0.036944, 0.059248, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.025706, 0.065255, -0.000645);
        glVertex3f(0.013512, 0.068953, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.013513, 0.068953, -0.361754);
        glVertex3f(0.025707, 0.065254, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.013512, 0.068953, -0.000645);
        glVertex3f(0.000832, 0.070202, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.000832, 0.070202, -0.361754);
        glVertex3f(0.013513, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.000832, 0.070202, -0.000645);
        glVertex3f(-0.011849, 0.068953, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.011849, 0.068953, -0.361754);
        glVertex3f(0.000832, 0.070202, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.011849, 0.068953, -0.000645);
        glVertex3f(-0.024043, 0.065254, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.024042, 0.065254, -0.361754);
        glVertex3f(-0.011849, 0.068953, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.024043, 0.065254, -0.000645);
        glVertex3f(-0.035280, 0.059247, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.035280, 0.059248, -0.361754);
        glVertex3f(-0.024042, 0.065254, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.035280, 0.059247, -0.000645);
        glVertex3f(-0.045130, 0.051164, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.045130, 0.051164, -0.361754);
        glVertex3f(-0.035280, 0.059248, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.045130, 0.051164, -0.000645);
        glVertex3f(-0.053213, 0.041314, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.053213, 0.041314, -0.361754);
        glVertex3f(-0.045130, 0.051164, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.053213, 0.041314, -0.000645);
        glVertex3f(-0.059220, 0.030076, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.059220, 0.030077, -0.361754);
        glVertex3f(-0.053213, 0.041314, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.059220, 0.030076, -0.000645);
        glVertex3f(-0.062919, 0.017883, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.062919, 0.017883, -0.361754);
        glVertex3f(-0.059220, 0.030077, -0.361754);
    glEnd();

    glNormal3f(-0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.062919, 0.017883, -0.000645);
        glVertex3f(-0.064168, 0.005202, -0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.064168, 0.005202, -0.361754);
        glVertex3f(-0.062919, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.064168, 0.005202, -0.000645);
        glVertex3f(-0.062919, -0.007479, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.062919, -0.007478, -0.361754);
        glVertex3f(-0.064168, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.062919, -0.007479, -0.000645);
        glVertex3f(-0.059220, -0.019672, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.059220, -0.019672, -0.361754);
        glVertex3f(-0.062919, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.059220, -0.019672, -0.000645);
        glVertex3f(-0.053213, -0.030910, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.053213, -0.030909, -0.361754);
        glVertex3f(-0.059220, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.053213, -0.030910, -0.000645);
        glVertex3f(-0.045129, -0.040759, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.045130, -0.040759, -0.361754);
        glVertex3f(-0.053213, -0.030909, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.045129, -0.040759, -0.000645);
        glVertex3f(-0.035280, -0.048843, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.035280, -0.048843, -0.361754);
        glVertex3f(-0.045130, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.035280, -0.048843, -0.000645);
        glVertex3f(-0.024042, -0.054850, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.024042, -0.054850, -0.361754);
        glVertex3f(-0.035280, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.024042, -0.054850, -0.000645);
        glVertex3f(-0.011849, -0.058548, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(-0.011849, -0.058548, -0.361754);
        glVertex3f(-0.024042, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.000000, -0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(-0.011849, -0.058548, -0.000645);
        glVertex3f(0.000832, -0.059797, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.000832, -0.059797, -0.361754);
        glVertex3f(-0.011849, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.000832, -0.059797, -0.000645);
        glVertex3f(0.013513, -0.058548, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.013513, -0.058548, -0.361754);
        glVertex3f(0.000832, -0.059797, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.013513, -0.058548, -0.000645);
        glVertex3f(0.025706, -0.054850, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.025706, -0.054850, -0.361754);
        glVertex3f(0.013513, -0.058548, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.025706, -0.054850, -0.000645);
        glVertex3f(0.036944, -0.048843, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.036944, -0.048843, -0.361754);
        glVertex3f(0.025706, -0.054850, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.036944, -0.048843, -0.000645);
        glVertex3f(0.046794, -0.040759, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.046794, -0.040759, -0.361754);
        glVertex3f(0.036944, -0.048843, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.046794, -0.040759, -0.000645);
        glVertex3f(0.054877, -0.030910, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.054877, -0.030910, -0.361754);
        glVertex3f(0.046794, -0.040759, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.054877, -0.030910, -0.000645);
        glVertex3f(0.060884, -0.019672, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.060884, -0.019672, -0.361754);
        glVertex3f(0.054877, -0.030910, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.060884, -0.019672, -0.000645);
        glVertex3f(0.064583, -0.007479, -0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.064583, -0.007478, -0.361754);
        glVertex3f(0.060884, -0.019672, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.064583, -0.007479, -0.000645);
        glVertex3f(0.065832, 0.005202, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.065832, 0.005202, -0.361754);
        glVertex3f(0.064583, -0.007478, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.065832, 0.005202, -0.000645);
        glVertex3f(0.064583, 0.017883, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.064583, 0.017883, -0.361754);
        glVertex3f(0.065832, 0.005202, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.064583, 0.017883, -0.000645);
        glVertex3f(0.060884, 0.030077, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.060884, 0.030077, -0.361754);
        glVertex3f(0.064583, 0.017883, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.060884, 0.030077, -0.000645);
        glVertex3f(0.054878, 0.041314, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.054877, 0.041314, -0.361754);
        glVertex3f(0.060884, 0.030077, -0.361754);
    glEnd();

    glNormal3f(0.000000, 0.000000, 1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.000645);
        glVertex3f(0.054878, 0.041314, -0.000645);
        glVertex3f(0.046794, 0.051164, -0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000832, 0.005202, -0.361754);
        glVertex3f(0.046794, 0.051164, -0.361754);
        glVertex3f(0.054877, 0.041314, -0.361754);
    glEnd();

    glNormal3f(0.653378, 0.536213, 0.534390);
    glBegin(GL_POLYGON);
        glVertex3f(0.192695, 0.128755, 0.000645);
        glVertex3f(0.163874, 0.163874, 0.000645);
        glVertex3f(0.000844, 0.002644, 0.361754);
    glEnd();

    glNormal3f(0.745118, 0.398271, 0.534957);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.214111, 0.088688, 0.000645);
        glVertex3f(0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(0.808079, 0.245128, 0.535650);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.227299, 0.045213, 0.000645);
        glVertex3f(0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(0.839872, 0.082722, 0.536443);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.231752, 0.000000, 0.000645);
        glVertex3f(0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(0.839327, -0.082666, 0.537305);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.227299, -0.045213, 0.000645);
        glVertex3f(0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(0.806524, -0.244657, 0.538202);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.214111, -0.088688, 0.000645);
        glVertex3f(0.227299, -0.045213, 0.000645);
    glEnd();

    glNormal3f(0.742791, -0.397029, 0.539100);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.192695, -0.128755, 0.000645);
        glVertex3f(0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.650634, -0.533961, 0.539965);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.163874, -0.163874, 0.000645);
        glVertex3f(0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.533636, -0.650238, 0.540762);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.128755, -0.192695, 0.000645);
        glVertex3f(0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.396315, -0.741453, 0.541463);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.088688, -0.214111, 0.000645);
        glVertex3f(0.128755, -0.192695, 0.000645);
    glEnd();

    glNormal3f(0.243941, -0.804167, 0.542041);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.045213, -0.227299, 0.000645);
        glVertex3f(0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(0.082341, -0.836028, 0.542473);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.000000, -0.231752, 0.000645);
        glVertex3f(0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.082325, -0.835853, 0.542745);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.045213, -0.227299, 0.000645);
        glVertex3f(-0.000000, -0.231752, 0.000645);
    glEnd();

    glNormal3f(-0.243790, -0.803670, 0.542845);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.088688, -0.214111, 0.000645);
        glVertex3f(-0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.466876, -0.702349, 0.537339);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.127585, -0.187549, 0.001567);
        glVertex3f(-0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.470363, -0.700011, 0.537348);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.163874, -0.163874, 0.000645);
        glVertex3f(-0.127585, -0.187549, 0.001567);
    glEnd();

    glNormal3f(-0.649565, -0.533083, 0.542114);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.192695, -0.128755, 0.000645);
        glVertex3f(-0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(-0.741400, -0.396286, 0.541557);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.214111, -0.088688, 0.000645);
        glVertex3f(-0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.804887, -0.244159, 0.540873);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.227299, -0.045212, 0.000645);
        glVertex3f(-0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.837556, -0.082492, 0.540088);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.231752, 0.000000, 0.000645);
        glVertex3f(-0.227299, -0.045212, 0.000645);
    glEnd();

    glNormal3f(-0.838103, 0.082547, 0.539231);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.227299, 0.045213, 0.000645);
        glVertex3f(-0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(-0.806442, 0.244633, 0.538336);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.214111, 0.088688, 0.000645);
        glVertex3f(-0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(-0.743727, 0.397532, 0.537436);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.192695, 0.128755, 0.000645);
        glVertex3f(-0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.652310, 0.535338, 0.536567);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.163873, 0.163874, 0.000645);
        glVertex3f(-0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.535661, 0.652707, 0.535762);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.128754, 0.192695, 0.000645);
        glVertex3f(-0.163873, 0.163874, 0.000645);
    glEnd();

    glNormal3f(-0.398244, 0.745064, 0.535052);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.088687, 0.214111, 0.000645);
        glVertex3f(-0.128754, 0.192695, 0.000645);
    glEnd();

    glNormal3f(-0.245345, 0.808797, 0.534465);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(-0.045212, 0.227299, 0.000645);
        glVertex3f(-0.088687, 0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.082869, 0.841398, 0.534024);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.000000, 0.231752, 0.000645);
        glVertex3f(-0.045212, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.082889, 0.841572, 0.533747);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.045213, 0.227299, 0.000645);
        glVertex3f(0.000000, 0.231752, 0.000645);
    glEnd();

    glNormal3f(0.245498, 0.809292, 0.533645);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.088688, 0.214111, 0.000645);
        glVertex3f(0.045213, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.398643, 0.745805, 0.533721);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.128755, 0.192695, 0.000645);
        glVertex3f(0.088688, 0.214111, 0.000645);
    glEnd();

    glNormal3f(0.536381, 0.653581, 0.533973);
    glBegin(GL_POLYGON);
        glVertex3f(0.000844, 0.002644, 0.361754);
        glVertex3f(0.163874, 0.163874, 0.000645);
        glVertex3f(0.128755, 0.192695, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.163874, 0.163874, 0.000645);
        glVertex3f(0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.192695, 0.128755, 0.000645);
        glVertex3f(0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.214111, 0.088688, 0.000645);
        glVertex3f(0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.227299, 0.045213, 0.000645);
        glVertex3f(0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.231752, 0.000000, 0.000645);
        glVertex3f(0.227299, -0.045213, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.227299, -0.045213, 0.000645);
        glVertex3f(0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.214111, -0.088688, 0.000645);
        glVertex3f(0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.192695, -0.128755, 0.000645);
        glVertex3f(0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.163874, -0.163874, 0.000645);
        glVertex3f(0.128755, -0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.128755, -0.192695, 0.000645);
        glVertex3f(0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.088688, -0.214111, 0.000645);
        glVertex3f(0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.045213, -0.227299, 0.000645);
        glVertex3f(-0.000000, -0.231752, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.000000, -0.231752, 0.000645);
        glVertex3f(-0.045213, -0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.045213, -0.227299, 0.000645);
        glVertex3f(-0.088688, -0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.018483, 0.007656, -0.999800);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.088688, -0.214111, 0.000645);
        glVertex3f(-0.127585, -0.187549, 0.001567);
    glEnd();

    glNormal3f(0.015380, -0.015380, -0.999763);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.127585, -0.187549, 0.001567);
        glVertex3f(-0.163874, -0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.163874, -0.163874, 0.000645);
        glVertex3f(-0.192695, -0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.192695, -0.128755, 0.000645);
        glVertex3f(-0.214111, -0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.214111, -0.088688, 0.000645);
        glVertex3f(-0.227299, -0.045212, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.227299, -0.045212, 0.000645);
        glVertex3f(-0.231752, 0.000000, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.231752, 0.000000, 0.000645);
        glVertex3f(-0.227299, 0.045213, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.227299, 0.045213, 0.000645);
        glVertex3f(-0.214111, 0.088688, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.214111, 0.088688, 0.000645);
        glVertex3f(-0.192695, 0.128755, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.192695, 0.128755, 0.000645);
        glVertex3f(-0.163873, 0.163874, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.163873, 0.163874, 0.000645);
        glVertex3f(-0.128754, 0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.128754, 0.192695, 0.000645);
        glVertex3f(-0.088687, 0.214111, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.088687, 0.214111, 0.000645);
        glVertex3f(-0.045212, 0.227299, 0.000645);
    glEnd();

    glNormal3f(0.000000, -0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(-0.045212, 0.227299, 0.000645);
        glVertex3f(0.000000, 0.231752, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.000000, 0.231752, 0.000645);
        glVertex3f(0.045213, 0.227299, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.045213, 0.227299, 0.000645);
        glVertex3f(0.088688, 0.214111, 0.000645);
    glEnd();

    glNormal3f(-0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.000000, -0.000000, 0.000645);
        glVertex3f(0.088688, 0.214111, 0.000645);
        glVertex3f(0.128755, 0.192695, 0.000645);
    glEnd();

    glNormal3f(0.000000, 0.000000, -1.000000);
    glBegin(GL_POLYGON);
        glVertex3f(0.128755, 0.192695, 0.000645);
        glVertex3f(0.163874, 0.163874, 0.000645);
        glVertex3f(0.000000, -0.000000, 0.000645);
    glEnd();
}
