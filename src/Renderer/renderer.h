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

#include <QMatrix4x4>
#include <QOpenGLBuffer>
#include <QOpenGLFramebufferObject>
#include <QOpenGLFunctions>
#include <QOpenGLShaderProgram>
#include <QOpenGLWidget>
#include <QVector>
#include <QVector3D>

class QFont;

// #define DIST_Z    2    /* the distance between 2 rooms */
#define BASE_Z -12      /* the distance to the "camera" */
#define ROOM_SIZE 1.0f /* the size of the rooms walls */
#define WALL_HEIGHT (ROOM_SIZE * 0.8f)
#define WALL_THICKNESS (ROOM_SIZE * 0.15f)
#define DOOR_WIDTH (ROOM_SIZE * 0.7f)
#define NEAR_CLIP_PLANE 0.01f /* near clipping plane distance - very close to allow deep zoom */
// Margin for frustum culling to account for walls, markers, notes extending beyond room center
// Needs to be generous to prevent border rooms from popping in/out
#define FRUSTUM_MARGIN 5.0f
// Radius for per-room sphere culling test (accounts for room size + walls + markers)
#define ROOM_CULL_RADIUS (ROOM_SIZE * 2.0f + WALL_HEIGHT)
#define MOB_FLAG_COUNT 19
#define LOAD_FLAG_COUNT 25

#define MAXHITS 200

class RendererWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

    struct RenderVertex
    {
        GLfloat position[3];
        GLfloat texCoord[2];
        GLfloat color[4];
    };

    struct RenderCommand
    {
        GLenum mode;
        int first;
        int count;
        GLuint texture;
        bool useTexture;
    };

    struct TextBillboard
    {
        QVector3D position;
        QString text;
        QColor color;
    };

    GLfloat colour[4];
    GLuint global_list;
    float curx;
    float cury;
    float curz;
    CRoom *baseRoom;
    float effectiveScale;
    float targetEffectiveScale;
    CFrustum frustum;
    int lowerZ;
    int upperZ;
    unsigned int last_drawn_marker;
    unsigned int last_drawn_trail;
    GLuint selectBuf[MAXHITS];

    QFont *textFont;

    GLfloat angleY;
    GLfloat angleX;
    GLfloat angleZ;
    float userX;
    float userY;
    float userZ;
    int userLayerShift;

    QMatrix4x4 projectionMatrix;
    QOpenGLShaderProgram *mapProgram;
    QOpenGLBuffer mapVbo;
    QOpenGLFramebufferObject *selectionFbo;
    QVector<RenderVertex> renderVertices;
    QVector<RenderCommand> renderCommands;
    QVector<TextBillboard> textBillboards;

    struct RenderTransform
    {
        QVector3D pos;
        float scale;
    };

    void glDrawGroupMarkers();
    void glDrawPrespamLine();
    void glDrawMarkers();
    void glDrawCSquare(CSquare *p, int renderingMode);
    void setupViewingModel(int width, int height);
    void renderPickupObjects();
    void renderPickupRoom(CRoom *p);
    RenderTransform getRenderTransform(CRoom *room);
    void updateEffectiveScale();
    void setupNewBaseCoordinates();
    void draw();
    void rebuildSquareBillboards(CSquare *square);
    void appendRoomGeometry(CRoom *room);
    void appendSquareGeometry(CSquare *square);
    void resetRenderBatch();
    void appendCommand(GLenum mode, bool useTexture, GLuint texture, int first, int count);
    void appendQuad(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d,
                    const GLfloat *color);
    void appendTexturedQuad(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d,
                            const QVector2D &ta, const QVector2D &tb, const QVector2D &tc, const QVector2D &td,
                            const GLfloat *color, GLuint texture);
    void appendQuadStrip4(const QVector3D &v0, const QVector3D &v1, const QVector3D &v2, const QVector3D &v3,
                          const QVector2D &t0, const QVector2D &t1, const QVector2D &t2, const QVector2D &t3,
                          const GLfloat *color, GLuint texture);
    void appendQuadStrip6(const QVector3D &v0, const QVector3D &v1, const QVector3D &v2, const QVector3D &v3,
                          const QVector3D &v4, const QVector3D &v5, const QVector2D &t0, const QVector2D &t1,
                          const QVector2D &t2, const QVector2D &t3, const QVector2D &t4, const QVector2D &t5,
                          const GLfloat *color, GLuint texture);
    void appendWallPrism(float x0, float x1, float y0, float y1, float z0, float z1,
                         const GLfloat *color, GLuint texture);
    void appendLine(const QVector3D &a, const QVector3D &b, const GLfloat *color);
    void appendMarkerGeometry(float dx, float dy, float dz, int mode, const GLfloat *color, float scaleFactor);
    void appendConeGeometry(float dx, float dy, float dz, float rotX, float rotY, const GLfloat *color,
                            float scaleFactor);
    void renderBatch(const QVector<RenderVertex> &vertices, const QVector<RenderCommand> &commands,
                     const QMatrix4x4 &mvp);
    void drawTextOverlay();

  public:
    int current_plane_z;
    GLuint basic_gllist;
    bool redraw;
    unsigned int deletedRoom;
    GLuint wall_textures[4];
    GLuint door_textures[4];
    GLuint road_textures[16];
    GLuint trail_textures[16];
    GLuint mob_textures[MOB_FLAG_COUNT];
    GLuint load_textures[LOAD_FLAG_COUNT];
    GLuint no_ride_texture;
    RendererWidget(QWidget *parent = 0);
    void initializeGL() override;
    void resizeGL(int width, int height) override;
    void changeUserLayerShift(int byValue)
    {
        userLayerShift += byValue;
        curz += byValue;
    }

    bool doSelect(QPoint pos, unsigned int &id);
    void centerOnRoom(unsigned int id);

    void resetViewSettings()
    {
        angleY = 0;
        angleX = 0;
        angleZ = 0;
        conf->setRendererAngles(angleX, angleY, angleZ);
        userX = 0;
        userY = 0;
        userZ = BASE_Z;
        conf->setRendererPosition(userX, userY, userZ);
        userLayerShift = 0;
    }

    GLfloat getAngleX() const { return angleX; }

    GLfloat getAngleY() const { return angleY; }

    GLfloat getAngleZ() const { return angleZ; }

    int getUserLayerShift() const { return userLayerShift; }

    float getUserX() const { return userX; }

    float getUserY() const { return userY; }

    float getUserZ() const { return userZ; }
    float getEffectiveScale() const { return effectiveScale; }

    void setAngleX(GLfloat angleX, bool dontsave = false)
    {
        this->angleX = angleX;
        if (!dontsave)
            conf->setRendererAngles(angleX, angleY, angleZ);
    }

    void setAngleY(GLfloat angleY, bool dontsave = false)
    {
        this->angleY = angleY;
        if (!dontsave)
            conf->setRendererAngles(angleX, angleY, angleZ);
    }

    void setAngleZ(GLfloat angleZ, bool dontsave = false)
    {
        this->angleZ = angleZ;
        if (!dontsave)
            conf->setRendererAngles(angleX, angleY, angleZ);
    }

    void setUserLayerShift(int userLayerShift) { this->userLayerShift = userLayerShift; }

    void setUserX(float userX, bool dontsave = false)
    {
        this->userX = userX;
        if (!dontsave)
            conf->setRendererPosition(userX, userY, userZ);
    }

    void setUserY(float userY, bool dontsave = false)
    {
        this->userY = userY;
        if (!dontsave)
            conf->setRendererPosition(userX, userY, userZ);
    }

    void setUserZ(float userZ, bool dontsave = false)
    {
        this->userZ = userZ;
        if (!dontsave)
            conf->setRendererPosition(userX, userY, userZ);
    }

  public slots:
    void display(void);
    void paintGL() override;

  signals:
    void updateCharPosition(unsigned int);
};

#endif
