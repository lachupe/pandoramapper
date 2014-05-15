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

#ifndef RENDERER_H 
#define RENDERER_H 


#include "defines.h"
#include "CConfigurator.h"

#include "Renderer/CFrustum.h"

#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

class QFont;

//#define DIST_Z	2	/* the distance between 2 rooms */
#define BASE_Z  -12	/* the distance to the "camera" */
#define ROOM_SIZE 0.47f	/* the size of the rooms walls */

#define MAXHITS 200


class RendererWidget : public QGLWidget
{
    Q_OBJECT
    
    CRoomManager    * map;
    GLfloat       colour[4];
    GLuint        global_list;
    int           curx;
    int           cury;
    int           curz;
    CFrustum frustum;
    int lowerZ;
    int upperZ;
    unsigned int last_drawn_marker;
    unsigned int last_drawn_trail;
    GLuint selectBuf[MAXHITS];

    QFont 		*textFont;

    GLfloat angleY;
    GLfloat angleX;
    GLfloat angleZ;
    float userX;
    float userY;
    float userZ;
    int userLayerShift;

    GLuint	exit_normal_texture;
    GLuint	exit_door_texture;
    GLuint	exit_secret_texture;
    GLuint	exit_undef_texture;


    QPixmap *m_terrainPixmaps[16];
    QPixmap *m_roadPixmaps[16];
    QPixmap *m_loadPixmaps[16];
    QPixmap *m_mobPixmaps[16];
    QPixmap *m_trailPixmaps[16]; // trail support

    QPixmap *m_roomShadowPixmap;


    void glDrawGroupMarkers();
    void glDrawPrespamLine();
    void glDrawMarkers();
    void drawMarker(int, int, int, int);
    void glDrawCSquare(CSquare *p, int renderingMode);
    void setupViewingModel(int width, int height);
    void renderPickupObjects();
    void renderPickupRoom(CRoom *p);
    void setupNewBaseCoordinates();
    void draw();
    void drawCone();
    void generateDisplayList(CSquare *p);

    void alphaOverlayTexture(const QString &texture);
    void alphaOverlayTexture(QPixmap *pix);

public:
    int current_plane_z;
    GLuint basic_gllist;
    bool redraw;
    unsigned int deletedRoom;
    RendererWidget(CRoomManager *_map, QWidget *parent = 0);
    void initializeGL();
    void resizeGL(int width, int height);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


    void changeUserLayerShift(int byValue)
    {
        userLayerShift += byValue;
        curz += byValue;
    }

    bool doSelect(QPoint pos, unsigned int & id);
    void centerOnRoom(RoomId id);

    void resetViewSettings() {
    	angleY = 0;
        angleX = 0;
        angleZ = 0;
        conf->setRendererAngles(angleX,angleY,angleZ);
        userX = 0;
        userY = 0;
        userZ = BASE_Z;
        conf->setRendererPosition(userX,userY,userZ);
        userLayerShift = 0;
    }

    GLfloat getAngleX() const
    {
        return angleX;
    }

    GLfloat getAngleY() const
    {
        return angleY;
    }

    GLfloat getAngleZ() const
    {
        return angleZ;
    }

    int getUserLayerShift() const
    {
        return userLayerShift;
    }

    float getUserX() const
    {
        return userX;
    }

    float getUserY() const
    {
        return userY;
    }

    float getUserZ() const
    {
        return userZ;
    }

    void setAngleX(GLfloat angleX, bool dontsave = false)
    {
        this->angleX = angleX;
        if (!dontsave)
        	conf->setRendererAngles(angleX,angleY,angleZ);
    }

    void setAngleY(GLfloat angleY, bool dontsave = false)
    {
        this->angleY = angleY;
        if (!dontsave)
        	conf->setRendererAngles(angleX,angleY,angleZ);
    }

    void setAngleZ(GLfloat angleZ, bool dontsave = false)
    {
        this->angleZ = angleZ;
        if (!dontsave)
        	conf->setRendererAngles(angleX,angleY,angleZ);
    }

    void setUserLayerShift(int userLayerShift)
    {
        this->userLayerShift = userLayerShift;
    }

    void setUserX(float userX, bool dontsave = false)
    {
        this->userX = userX;
        if (!dontsave)
        	conf->setRendererPosition(userX,userY,userZ);
    }

    void setUserY(float userY, bool dontsave = false)
    {
        this->userY = userY;
        if (!dontsave)
            conf->setRendererPosition(userX,userY,userZ);
    }

    void setUserZ(float userZ, bool dontsave = false)
    {
        this->userZ = userZ;
        if (!dontsave)
            conf->setRendererPosition(userX,userY,userZ);
    }

public slots:
	void display(void);
    void paintGL();

signals:
	void updateCharPosition(unsigned int);
};



#endif
