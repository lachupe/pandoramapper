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
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <cstddef>
#include <algorithm>
#include <cmath>
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
#define GL_MULTISAMPLE 0x809D
#endif

#if defined(Q_CC_MSVC)
#pragma warning(disable : 4305)  // init: truncation from const double to float
#endif

GLfloat marker_colour[4] = {1.0, 0.1, 0.1, 1.0};

#define MARKER_SIZE (ROOM_SIZE / 1.85)
#define CONNECTION_THICKNESS_DIVIDOR 5
#define PICK_TOL 50

RendererWidget::RendererWidget(QWidget *parent)
    : QOpenGLWidget(parent), mapProgram(nullptr), mapVbo(QOpenGLBuffer::VertexBuffer), selectionFbo(nullptr)
{
    print_debug(DEBUG_RENDERER, "in renderer constructor");

    // Disable transparency - we want a fully opaque OpenGL widget
    setAttribute(Qt::WA_OpaquePaintEvent, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);

    angleX = conf->getRendererAngleX();
    angleY = conf->getRendererAngleY();
    angleZ = conf->getRendererAngleZ();
    userX = (GLfloat)conf->getRendererPositionX();
    userY = (GLfloat)conf->getRendererPositionY();
    userZ = (GLfloat)conf->getRendererPositionZ(); /* additional shift added by user */

    if (userZ == 0)
        userZ = BASE_Z;

    curx = 0;
    cury = 0;
    curz = 0; /* current rooms position */

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
    textFont = new QFont("Times", 12, QFont::Bold);

    // textFont = new QFont("Times", 10, QFont::Bold);

    glShadeModel(GL_SMOOTH);
    glClearColor(0.15, 0.15, 0.15, 1.0); /* This Will Clear The Background Color To Dark Gray */
    glPointSize(4.0);                    /* Add point size, to make it clear */
    glLineWidth(2.0);                    /* Add line width,   ditto */

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

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

    static const char *vertexShaderSrc = "attribute vec3 aPosition;\n"
                                         "attribute vec2 aTexCoord;\n"
                                         "attribute vec4 aColor;\n"
                                         "uniform mat4 uMvp;\n"
                                         "varying vec2 vTexCoord;\n"
                                         "varying vec4 vColor;\n"
                                         "void main() {\n"
                                         "  gl_Position = uMvp * vec4(aPosition, 1.0);\n"
                                         "  vTexCoord = aTexCoord;\n"
                                         "  vColor = aColor;\n"
                                         "}\n";

    static const char *fragmentShaderSrc =
        "uniform sampler2D uTexture;\n"
        "uniform int uUseTexture;\n"
        "varying vec2 vTexCoord;\n"
        "varying vec4 vColor;\n"
        "void main() {\n"
        "  vec4 texColor = (uUseTexture == 1) ? texture2D(uTexture, vTexCoord) : vec4(1.0);\n"
        "  gl_FragColor = texColor * vColor;\n"
        "}\n";

    mapProgram = new QOpenGLShaderProgram(this);
    mapProgram->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexShaderSrc);
    mapProgram->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentShaderSrc);
    mapProgram->bindAttributeLocation("aPosition", 0);
    mapProgram->bindAttributeLocation("aTexCoord", 1);
    mapProgram->bindAttributeLocation("aColor", 2);
    mapProgram->link();

    mapVbo.create();
    mapVbo.setUsagePattern(QOpenGLBuffer::DynamicDraw);

    print_debug(DEBUG_RENDERER, "in init()");

    basic_gllist = glGenLists(1);
    if (basic_gllist != 0) {
        glNewList(basic_gllist, GL_COMPILE);
        glRectf(-ROOM_SIZE, -ROOM_SIZE, ROOM_SIZE, ROOM_SIZE);
        glEndList();
    }

    print_debug(DEBUG_RENDERER, "loading %i secto textures", conf->sectors.size());

    for (i = 0; i < conf->sectors.size(); i++) {
        conf->loadSectorTexture(&conf->sectors[i]);
    }

    // load the exits texture
    conf->loadNormalTexture("images/exit_normal.png", &conf->exit_normal_texture);
    conf->loadNormalTexture("images/exit_door.png", &conf->exit_door_texture);
    conf->loadNormalTexture("images/exit_secret.png", &conf->exit_secret_texture);
    conf->loadNormalTexture("images/exit_undef.png", &conf->exit_undef_texture);
}

void RendererWidget::setupViewingModel(int width, int height)
{
    gluPerspective(60.0f, (GLfloat)width / (GLfloat)height, 1.0f, conf->getDetailsVisibility() * 1.1f);
    glMatrixMode(GL_MODELVIEW);
}

void RendererWidget::resizeGL(int width, int height)
{
    print_debug(DEBUG_RENDERER, "in resizeGL()");

    glViewport(0, 0, (GLint)width, (GLint)height);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    setupViewingModel(width, height);
    projectionMatrix.setToIdentity();
    projectionMatrix.perspective(60.0f, static_cast<float>(width) / static_cast<float>(height), 1.0f,
                                 conf->getDetailsVisibility() * 1.1f);

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
    drawTextOverlay();
}

void RendererWidget::glDrawMarkers()
{
    CRoom *p;
    unsigned int k;
    QByteArray lastMovement;
    int dx, dy, dz;

    if (stacker.amount() == 0)
        return;

    GLfloat markerColor[4] = {marker_colour[0], marker_colour[1], marker_colour[2], marker_colour[3]};
    for (k = 0; k < stacker.amount(); k++) {
        p = stacker.get(k);

        if (p == nullptr) {
            print_debug(DEBUG_RENDERER, "RENDERER ERROR: Stuck upon corrupted room while drawing red pointers.\r\n");
            continue;
        }

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        appendMarkerGeometry(dx, dy, dz, 2, markerColor);

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

            appendConeGeometry(dx, dy, dz + 0.2f, rotX, rotY, markerColor);
        } else {
            appendConeGeometry(dx, dy, dz + 0.2f, 0.0f, 0.0f, markerColor);
        }
    }

    if (last_drawn_marker != stacker.first()->id) {
        last_drawn_trail = last_drawn_marker;
        last_drawn_marker = stacker.first()->id;
        renderer_window->getGroupManager()->setCharPosition(last_drawn_marker);
        // emit updateCharPosition(last_drawn_marker);
    }

    /*
    if (last_drawn_trail) {
        glColor4f(marker_colour[0] / 1.1, marker_colour[1] / 1.3, marker_colour[2] / 1.3, marker_colour[3] / 1.3);
        p = Map.getRoom(last_drawn_trail);
        if (p != nullptr) {
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
    auto line = engine->getPrespammedDirs();
    int prevx, prevy, prevz, dx, dy, dz;

    if (!line)
        return;

    CRoom *p = Map.getRoom(line->at(0));

    if (p == nullptr) {
        return;
    }

    prevx = p->getX() - curx;
    prevy = p->getY() - cury;
    prevz = (p->getZ() - curz) /* * DIST_Z */;

    GLfloat lineColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};

    for (int i = 1; i < line->size(); i++) {
        // connect all rooms with lines
        unsigned int id = line->at(i);

        CRoom *p = Map.getRoom(id);

        if (p == nullptr)
            continue;

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        // glDrawMarkerPrimitive(dx, dy, dz, 2);

        // glTranslatef(dx, dy, dz + 0.2f);

        appendLine(QVector3D(prevx, prevy, prevz), QVector3D(dx, dy, dz), lineColor);

        prevx = dx;
        prevy = dy;
        prevz = dz;
    }

    // and draw a cone in the last room
    appendConeGeometry(prevx, prevy, prevz + 0.2f, 0.0f, 0.0f, lineColor);

    // unique_ptr automatically handles cleanup
}

void RendererWidget::glDrawGroupMarkers()
{
    CRoom *p;
    QByteArray lastMovement;
    int dx, dy, dz;
    QVector<CGroupChar *> chars;
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
            continue;  // do not draw markers in the same room as our own marker

        QColor color = ch->getColor();

        double red = color.red() / 255.;
        double green = color.green() / 255.;
        double blue = color.blue() / 255.;
        double alpha = color.alpha() / 255.;

        GLfloat markerColor[4] = {static_cast<GLfloat>(red), static_cast<GLfloat>(green), static_cast<GLfloat>(blue),
                                  static_cast<GLfloat>(alpha)};
        p = Map.getRoom(pos);

        if (p == nullptr) {
            continue;
        }

        dx = p->getX() - curx;
        dy = p->getY() - cury;
        dz = (p->getZ() - curz) /* * DIST_Z */;

        appendMarkerGeometry(dx, dy, dz, 2, markerColor);

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

            appendConeGeometry(dx, dy, dz + 0.2f, rotX, rotY, markerColor);
        } else {
            appendConeGeometry(dx, dy, dz + 0.2f, 0.0f, 0.0f, markerColor);
        }
    }
}

void RendererWidget::resetRenderBatch()
{
    renderVertices.clear();
    renderCommands.clear();
}

void RendererWidget::appendCommand(GLenum mode, bool useTexture, GLuint texture, int first, int count)
{
    if (!renderCommands.isEmpty()) {
        RenderCommand &last = renderCommands.last();
        if (last.mode == mode && last.useTexture == useTexture && last.texture == texture &&
            last.first + last.count == first) {
            last.count += count;
            return;
        }
    }

    RenderCommand cmd;
    cmd.mode = mode;
    cmd.first = first;
    cmd.count = count;
    cmd.texture = texture;
    cmd.useTexture = useTexture;
    renderCommands.append(cmd);
}

void RendererWidget::appendQuad(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d,
                                const GLfloat *color)
{
    RenderVertex v0 = {{a.x(), a.y(), a.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v1 = {{b.x(), b.y(), b.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v2 = {{c.x(), c.y(), c.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v3 = {{d.x(), d.y(), d.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};

    int first = renderVertices.size();
    renderVertices.append(v0);
    renderVertices.append(v1);
    renderVertices.append(v2);
    renderVertices.append(v0);
    renderVertices.append(v2);
    renderVertices.append(v3);
    appendCommand(GL_TRIANGLES, false, 0, first, 6);
}

void RendererWidget::appendTexturedQuad(const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d,
                                        const QVector2D &ta, const QVector2D &tb, const QVector2D &tc,
                                        const QVector2D &td, const GLfloat *color, GLuint texture)
{
    RenderVertex v0 = {{a.x(), a.y(), a.z()}, {ta.x(), ta.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v1 = {{b.x(), b.y(), b.z()}, {tb.x(), tb.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v2 = {{c.x(), c.y(), c.z()}, {tc.x(), tc.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v3 = {{d.x(), d.y(), d.z()}, {td.x(), td.y()}, {color[0], color[1], color[2], color[3]}};

    int first = renderVertices.size();
    renderVertices.append(v0);
    renderVertices.append(v1);
    renderVertices.append(v2);
    renderVertices.append(v0);
    renderVertices.append(v2);
    renderVertices.append(v3);
    appendCommand(GL_TRIANGLES, true, texture, first, 6);
}

void RendererWidget::appendQuadStrip4(const QVector3D &v0, const QVector3D &v1, const QVector3D &v2,
                                      const QVector3D &v3, const QVector2D &t0, const QVector2D &t1,
                                      const QVector2D &t2, const QVector2D &t3, const GLfloat *color, GLuint texture)
{
    RenderVertex a0 = {{v0.x(), v0.y(), v0.z()}, {t0.x(), t0.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a1 = {{v1.x(), v1.y(), v1.z()}, {t1.x(), t1.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a2 = {{v2.x(), v2.y(), v2.z()}, {t2.x(), t2.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a3 = {{v3.x(), v3.y(), v3.z()}, {t3.x(), t3.y()}, {color[0], color[1], color[2], color[3]}};

    int first = renderVertices.size();
    renderVertices.append(a0);
    renderVertices.append(a1);
    renderVertices.append(a2);
    renderVertices.append(a1);
    renderVertices.append(a3);
    renderVertices.append(a2);
    appendCommand(GL_TRIANGLES, true, texture, first, 6);
}

void RendererWidget::appendQuadStrip6(const QVector3D &v0, const QVector3D &v1, const QVector3D &v2,
                                      const QVector3D &v3, const QVector3D &v4, const QVector3D &v5,
                                      const QVector2D &t0, const QVector2D &t1, const QVector2D &t2,
                                      const QVector2D &t3, const QVector2D &t4, const QVector2D &t5,
                                      const GLfloat *color, GLuint texture)
{
    RenderVertex a0 = {{v0.x(), v0.y(), v0.z()}, {t0.x(), t0.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a1 = {{v1.x(), v1.y(), v1.z()}, {t1.x(), t1.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a2 = {{v2.x(), v2.y(), v2.z()}, {t2.x(), t2.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a3 = {{v3.x(), v3.y(), v3.z()}, {t3.x(), t3.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a4 = {{v4.x(), v4.y(), v4.z()}, {t4.x(), t4.y()}, {color[0], color[1], color[2], color[3]}};
    RenderVertex a5 = {{v5.x(), v5.y(), v5.z()}, {t5.x(), t5.y()}, {color[0], color[1], color[2], color[3]}};

    int first = renderVertices.size();
    renderVertices.append(a0);
    renderVertices.append(a1);
    renderVertices.append(a2);
    renderVertices.append(a1);
    renderVertices.append(a3);
    renderVertices.append(a2);
    renderVertices.append(a2);
    renderVertices.append(a3);
    renderVertices.append(a4);
    renderVertices.append(a3);
    renderVertices.append(a5);
    renderVertices.append(a4);
    appendCommand(GL_TRIANGLES, true, texture, first, 12);
}

void RendererWidget::appendLine(const QVector3D &a, const QVector3D &b, const GLfloat *color)
{
    RenderVertex v0 = {{a.x(), a.y(), a.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
    RenderVertex v1 = {{b.x(), b.y(), b.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};

    int first = renderVertices.size();
    renderVertices.append(v0);
    renderVertices.append(v1);
    appendCommand(GL_LINES, false, 0, first, 2);
}

void RendererWidget::appendMarkerGeometry(float dx, float dy, float dz, int mode, const GLfloat *color)
{
    auto pushVertex = [&](const QVector3D &pos) {
        RenderVertex v;
        v.position[0] = pos.x();
        v.position[1] = pos.y();
        v.position[2] = pos.z();
        v.texCoord[0] = 0.0f;
        v.texCoord[1] = 0.0f;
        v.color[0] = color[0];
        v.color[1] = color[1];
        v.color[2] = color[2];
        v.color[3] = color[3];
        renderVertices.append(v);
    };

    QVector3D upperA(dx, MARKER_SIZE + dy + ROOM_SIZE, dz);
    QVector3D upperB(-MARKER_SIZE + dx, dy + ROOM_SIZE, dz);
    QVector3D upperC(MARKER_SIZE + dx, dy + ROOM_SIZE, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(upperA);
    pushVertex(upperB);
    pushVertex(upperC);

    QVector3D lowerA(dx, -MARKER_SIZE + dy - ROOM_SIZE, dz);
    QVector3D lowerB(-MARKER_SIZE + dx, dy - ROOM_SIZE, dz);
    QVector3D lowerC(MARKER_SIZE + dx, dy - ROOM_SIZE, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(lowerA);
    pushVertex(lowerB);
    pushVertex(lowerC);

    QVector3D rightA(dx + ROOM_SIZE, MARKER_SIZE + dy, dz);
    QVector3D rightB(MARKER_SIZE + dx + ROOM_SIZE, dy, dz);
    QVector3D rightC(dx + ROOM_SIZE, -MARKER_SIZE + dy, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(rightA);
    pushVertex(rightB);
    pushVertex(rightC);

    QVector3D leftA(dx - ROOM_SIZE, MARKER_SIZE + dy, dz);
    QVector3D leftB(-MARKER_SIZE + dx - ROOM_SIZE, dy, dz);
    QVector3D leftC(dx - ROOM_SIZE, -MARKER_SIZE + dy, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(leftA);
    pushVertex(leftB);
    pushVertex(leftC);

    if (mode == 1) {
        QVector3D l0(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        QVector3D l1(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy - ROOM_SIZE, dz);
        QVector3D l2(dx - ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D l3(dx - ROOM_SIZE, dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        appendQuad(l0, l1, l2, l3, color);

        QVector3D r0(dx + ROOM_SIZE, dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        QVector3D r1(dx + ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D r2(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy - ROOM_SIZE, dz);
        QVector3D r3(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        appendQuad(r0, r1, r2, r3, color);

        QVector3D u0(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        QVector3D u1(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy + ROOM_SIZE, dz);
        QVector3D u2(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy + ROOM_SIZE, dz);
        QVector3D u3(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy + ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        appendQuad(u0, u1, u2, u3, color);

        QVector3D d0(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy - ROOM_SIZE, dz);
        QVector3D d1(dx - ROOM_SIZE - (MARKER_SIZE / 3.5f), dy - ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        QVector3D d2(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy - ROOM_SIZE + (MARKER_SIZE / 3.5f), dz);
        QVector3D d3(dx + ROOM_SIZE + (MARKER_SIZE / 3.5f), dy - ROOM_SIZE, dz);
        appendQuad(d0, d1, d2, d3, color);
    }
}

void RendererWidget::appendConeGeometry(float dx, float dy, float dz, float rotX, float rotY, const GLfloat *color)
{
    const float height = 0.36f;
    const float radius = 0.08f;
    const int segments = 16;
    const float twoPi = 6.28318530718f;

    auto pushVertex = [&](const QVector3D &pos) {
        RenderVertex v;
        v.position[0] = pos.x();
        v.position[1] = pos.y();
        v.position[2] = pos.z();
        v.texCoord[0] = 0.0f;
        v.texCoord[1] = 0.0f;
        v.color[0] = color[0];
        v.color[1] = color[1];
        v.color[2] = color[2];
        v.color[3] = color[3];
        renderVertices.append(v);
    };

    QMatrix4x4 transform;
    transform.translate(dx, dy, dz);
    if (rotX != 0.0f)
        transform.rotate(rotX, 1.0f, 0.0f, 0.0f);
    if (rotY != 0.0f)
        transform.rotate(rotY, 0.0f, 1.0f, 0.0f);

    QVector3D tip = transform * QVector3D(0.0f, 0.0f, height);

    for (int i = 0; i < segments; i++) {
        float a0 = (static_cast<float>(i) / segments) * twoPi;
        float a1 = (static_cast<float>(i + 1) / segments) * twoPi;

        QVector3D base0(radius * std::cos(a0), radius * std::sin(a0), 0.0f);
        QVector3D base1(radius * std::cos(a1), radius * std::sin(a1), 0.0f);
        base0 = transform * base0;
        base1 = transform * base1;

        appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
        pushVertex(tip);
        pushVertex(base0);
        pushVertex(base1);
    }
}

void RendererWidget::renderBatch(const QVector<RenderVertex> &vertices, const QVector<RenderCommand> &commands,
                                 const QMatrix4x4 &mvp)
{
    if (!mapProgram || vertices.isEmpty() || commands.isEmpty())
        return;

    mapProgram->bind();
    mapProgram->setUniformValue("uMvp", mvp);
    mapProgram->setUniformValue("uTexture", 0);
    glActiveTexture(GL_TEXTURE0);

    mapVbo.bind();
    mapVbo.allocate(vertices.constData(), vertices.size() * static_cast<int>(sizeof(RenderVertex)));

    mapProgram->enableAttributeArray(0);
    mapProgram->enableAttributeArray(1);
    mapProgram->enableAttributeArray(2);
    mapProgram->setAttributeBuffer(0, GL_FLOAT, offsetof(RenderVertex, position), 3, sizeof(RenderVertex));
    mapProgram->setAttributeBuffer(1, GL_FLOAT, offsetof(RenderVertex, texCoord), 2, sizeof(RenderVertex));
    mapProgram->setAttributeBuffer(2, GL_FLOAT, offsetof(RenderVertex, color), 4, sizeof(RenderVertex));

    for (int i = 0; i < commands.size(); i++) {
        const RenderCommand &cmd = commands[i];
        mapProgram->setUniformValue("uUseTexture", cmd.useTexture ? 1 : 0);
        if (cmd.useTexture) {
            glBindTexture(GL_TEXTURE_2D, cmd.texture);
        }
        glDrawArrays(cmd.mode, cmd.first, cmd.count);
    }

    mapVbo.release();
    mapProgram->disableAttributeArray(0);
    mapProgram->disableAttributeArray(1);
    mapProgram->disableAttributeArray(2);
    mapProgram->release();
}

void RendererWidget::drawTextOverlay()
{
    if (textBillboards.isEmpty() || !textFont)
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setFont(*textFont);

    QMatrix4x4 viewMatrix;
    viewMatrix.setToIdentity();
    viewMatrix.translate(0.0f, 0.0f, userZ);
    viewMatrix.rotate(angleX, 1.0f, 0.0f, 0.0f);
    viewMatrix.rotate(angleY, 0.0f, 1.0f, 0.0f);
    viewMatrix.rotate(angleZ, 0.0f, 0.0f, 1.0f);
    viewMatrix.translate(userX, userY, 0.0f);

    QMatrix4x4 mvp = projectionMatrix * viewMatrix;

    for (int i = 0; i < textBillboards.size(); i++) {
        const TextBillboard &entry = textBillboards[i];
        QVector4D clip = mvp * QVector4D(entry.position, 1.0f);
        if (clip.w() <= 0.0f)
            continue;

        float invW = 1.0f / clip.w();
        float ndcX = clip.x() * invW;
        float ndcY = clip.y() * invW;

        if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f)
            continue;

        float screenX = (ndcX * 0.5f + 0.5f) * static_cast<float>(width());
        float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(height());

        painter.setPen(entry.color);
        painter.drawText(QPointF(screenX, screenY), entry.text);
    }
}

void RendererWidget::rebuildSquareBillboards(CSquare *square)
{
    square->clearDoorsList();
    square->clearNotesList();

    for (int ind = 0; ind < square->rooms.size(); ind++) {
        CRoom *p = square->rooms[ind];

        if (p->getNote().isEmpty() != true) {
            QColor color;
            if (p->getNoteColor() == "")
                color = QColor((QString)conf->getNoteColor());
            else
                color = QColor((QString)p->getNoteColor());

            square->notesBillboards.append(
                new Billboard(p->getX(), p->getY(), p->getZ() + ROOM_SIZE / 2, p->getNote(), color));
        }

        for (int k = 0; k <= 5; k++) {
            if (!p->isExitPresent(k))
                continue;
            if (!p->isExitNormal(k))
                continue;

            CRoom *r = p->exits[k];
            if (!r)
                continue;

            if (p->getDoor(k) != "") {
                if (p->isDoorSecret(k) == false) {
                    continue;
                }

                QByteArray info;
                QByteArray alias;
                info = p->getDoor(k);
                alias = engine->get_users_region()->getAliasByDoor(info, k);
                if (alias != "" && p->getRegion() == engine->get_users_region()) {
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

                square->doorsBillboards.append(new Billboard(x, y, z, info, QColor(255, 255, 255, 255)));
            }
        }
    }

    square->rebuild_display_list = false;
}

void RendererWidget::appendRoomGeometry(CRoom *p)
{
    float dx = p->getX() - curx;
    float dy = p->getY() - cury;
    float dz = p->getZ() - curz;

    if (p->getTerrain()) {
        GLuint texture = conf->sectors[p->getTerrain()].texture;
        QVector3D a(dx - ROOM_SIZE, dy + ROOM_SIZE, dz);
        QVector3D b(dx - ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D c(dx + ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D d(dx + ROOM_SIZE, dy + ROOM_SIZE, dz);
        appendTexturedQuad(a, b, c, d, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f), QVector2D(1.0f, 0.0f),
                           QVector2D(1.0f, 1.0f), colour, texture);
    } else {
        QVector3D a(dx - ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D b(dx + ROOM_SIZE, dy - ROOM_SIZE, dz);
        QVector3D c(dx + ROOM_SIZE, dy + ROOM_SIZE, dz);
        QVector3D d(dx - ROOM_SIZE, dy + ROOM_SIZE, dz);
        appendQuad(a, b, c, d, colour);
    }

    for (int k = 0; k <= 5; k++) {
        if (!p->isExitPresent(k))
            continue;

        float kx = 0;
        float ky = 0;
        float kz = 0;
        float sx = 0;
        float sy = 0;
        GLuint exit_texture = conf->exit_normal_texture;
        int thickness = CONNECTION_THICKNESS_DIVIDOR;

        if (k == NORTH) {
            kx = 0;
            ky = +(ROOM_SIZE + 0.2f);
            kz = 0;
            sx = 0;
            sy = +ROOM_SIZE;
        } else if (k == EAST) {
            kx = +(ROOM_SIZE + 0.2f);
            ky = 0;
            kz = 0;
            sx = +ROOM_SIZE;
            sy = 0;
        } else if (k == SOUTH) {
            kx = 0;
            ky = -(ROOM_SIZE + 0.2f);
            kz = 0;
            sx = 0;
            sy = -ROOM_SIZE;
        } else if (k == WEST) {
            kx = -(ROOM_SIZE + 0.2f);
            ky = 0;
            kz = 0;
            sx = -ROOM_SIZE;
            sy = 0;
        } else if (k == UP) {
            kx = 0;
            ky = 0;
            kz = +(ROOM_SIZE + 0.2f);
            sx = ROOM_SIZE / 2;
            sy = 0;
        } else {
            kx = 0;
            ky = 0;
            kz = -(ROOM_SIZE + 0.2f);
            sx = 0;
            sy = ROOM_SIZE / 2;
        }

        if (p->isExitNormal(k)) {
            CRoom *r = p->exits[k];
            if (!r)
                continue;

            float dx2 = r->getX() - curx;
            float dy2 = r->getY() - cury;
            float dz2 = r->getZ() - curz;

            dx2 = (dx + dx2) / 2.0f;
            dy2 = (dy + dy2) / 2.0f;
            dz2 = (dz + dz2) / 2.0f;

            if (p->getDoor(k) != "") {
                if (p->isDoorSecret(k) == false) {
                    exit_texture = conf->exit_door_texture;
                } else {
                    exit_texture = conf->exit_secret_texture;
                }
            }

            QVector3D v0(dx + sx - sy / thickness, dy + sy - sx / thickness, dz);
            QVector3D v1(dx + sx + sy / thickness, dy + sy + sx / thickness, dz);
            QVector3D v2(dx + kx - sy / thickness, dy + ky - sx / thickness, dz + kz);
            QVector3D v3(dx + kx + sy / thickness, dy + ky + sx / thickness, dz + kz);
            QVector3D v4(dx2 - sy / thickness, dy2 - sx / thickness, dz2);
            QVector3D v5(dx2 + sy / thickness, dy2 + sx / thickness, dz2);

            appendQuadStrip6(v0, v1, v2, v3, v4, v5, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
                             QVector2D(0.5f, 1.0f), QVector2D(0.5f, 0.0f), QVector2D(1.0f, 1.0f), QVector2D(1.0f, 0.0f),
                             colour, exit_texture);
        } else {
            float dx2 = dx;
            float dy2 = dy;
            float dz2 = dz;

            if (k == NORTH) {
                dy2 = dy + 0.5f;
            } else if (k == EAST) {
                dx2 = dx + 0.5f;
            } else if (k == SOUTH) {
                dy2 = dy - 0.5f;
            } else if (k == WEST) {
                dx2 = dx - 0.5f;
            } else if (k == UP) {
                dz2 = dz + 0.5f;
            } else if (k == DOWN) {
                dz2 = dz - 0.5f;
            }

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

            QVector3D v0(dx + kx - sy / thickness, dy + ky - sx / thickness, dz);
            QVector3D v1(dx + kx + sy / thickness, dy + ky + sx / thickness, dz);
            QVector3D v2(dx2 + kx - sy / thickness, dy2 + ky - sx / thickness, dz2);
            QVector3D v3(dx2 + kx + sy / thickness, dy2 + ky + sx / thickness, dz2);

            appendQuadStrip4(v0, v1, v2, v3, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f), QVector2D(1.0f, 1.0f),
                             QVector2D(1.0f, 0.0f), colour, exit_texture);

            GLuint death_terrain = conf->getTextureByDesc("DEATH");
            if (death_terrain && p->isExitDeath(k)) {
                QVector3D da(dx2 + kx - ROOM_SIZE, dy2 + ky + ROOM_SIZE, dz2);
                QVector3D db(dx2 + kx - ROOM_SIZE, dy2 + ky - ROOM_SIZE, dz2);
                QVector3D dc(dx2 + kx + ROOM_SIZE, dy2 + ky - ROOM_SIZE, dz2);
                QVector3D dd(dx2 + kx + ROOM_SIZE, dy2 + ky + ROOM_SIZE, dz2);
                appendTexturedQuad(da, db, dc, dd, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f), QVector2D(1.0f, 0.0f),
                                   QVector2D(1.0f, 1.0f), colour, death_terrain);
            }
        }
    }
}

void RendererWidget::appendSquareGeometry(CSquare *square)
{
    if (square->rebuild_display_list)
        rebuildSquareBillboards(square);

    for (int ind = 0; ind < square->rooms.size(); ind++) {
        CRoom *room = square->rooms[ind];
        appendRoomGeometry(room);

        if (!Map.selections.isEmpty() && Map.selections.isSelected(room->id) == true) {
            GLfloat highlight[4] = {0.20f, 0.20f, 0.80f, colour[3] - 0.1f};
            if (highlight[3] < 0.0f)
                highlight[3] = 0.0f;
            float dx = room->getX() - curx;
            float dy = room->getY() - cury;
            float dz = room->getZ() - curz;
            QVector3D a(dx - ROOM_SIZE * 2, dy - ROOM_SIZE * 2, dz);
            QVector3D b(dx + ROOM_SIZE * 2, dy - ROOM_SIZE * 2, dz);
            QVector3D c(dx + ROOM_SIZE * 2, dy + ROOM_SIZE * 2, dz);
            QVector3D d(dx - ROOM_SIZE * 2, dy + ROOM_SIZE * 2, dz);
            appendQuad(a, b, c, d, highlight);
        }

        if (conf->getDisplayRegionsRenderer()) {
            if (room->getRegion() && room->getRegion() == engine->get_users_region()) {
                GLfloat regionColor[4] = {0.50f, 0.50f, 0.50f, colour[3] - 0.2f};
                if (regionColor[3] < 0.0f)
                    regionColor[3] = 0.0f;
                float dx = room->getX() - curx;
                float dy = room->getY() - cury;
                float dz = room->getZ() - curz;
                QVector3D a(dx - ROOM_SIZE * 1.5f, dy - ROOM_SIZE * 1.5f, dz);
                QVector3D b(dx + ROOM_SIZE * 1.5f, dy - ROOM_SIZE * 1.5f, dz);
                QVector3D c(dx + ROOM_SIZE * 1.5f, dy + ROOM_SIZE * 1.5f, dz);
                QVector3D d(dx - ROOM_SIZE * 1.5f, dy + ROOM_SIZE * 1.5f, dz);
                appendQuad(a, b, c, d, regionColor);
            }
        }
    }
}

// renderingMode serves for separating Pickup GLSELECT and normal GL_SELECT mode
void RendererWidget::glDrawCSquare(CSquare *p, int renderingMode)
{
    int k;

    if (!frustum.isSquareInFrustum(p)) {
        return;  // this square is not in view
    }

    if (p->toBePassed()) {
        //         go deeper
        if (p->subsquares[CSquare::Left_Upper])
            glDrawCSquare(p->subsquares[CSquare::Left_Upper], renderingMode);
        if (p->subsquares[CSquare::Right_Upper])
            glDrawCSquare(p->subsquares[CSquare::Right_Upper], renderingMode);
        if (p->subsquares[CSquare::Left_Lower])
            glDrawCSquare(p->subsquares[CSquare::Left_Lower], renderingMode);
        if (p->subsquares[CSquare::Right_Lower])
            glDrawCSquare(p->subsquares[CSquare::Right_Lower], renderingMode);
    } else {
        if (renderingMode == GL_SELECT) {
            for (k = 0; k < p->rooms.size(); k++)
                renderPickupRoom(p->rooms[k]);
        } else {
            appendSquareGeometry(p);

            // draw notes, if needed
            if (conf->getShowNotesRenderer() == true && p->notesBillboards.isEmpty() != true) {
                for (int n = 0; n < p->notesBillboards.size(); n++) {
                    Billboard *billboard = p->notesBillboards[n];

                    TextBillboard entry;
                    entry.position = QVector3D(billboard->x - curx, billboard->y - cury, billboard->z - curz);
                    entry.text = billboard->text;
                    entry.color = billboard->color;
                    textBillboards.append(entry);
                }
            }

            // draw doors, if needed
            if (conf->getShowRegionsInfo() == true && p->doorsBillboards.isEmpty() != true) {
                for (int n = 0; n < p->doorsBillboards.size(); n++) {
                    Billboard *billboard = p->doorsBillboards[n];
                    TextBillboard entry;
                    entry.position = QVector3D(billboard->x - curx, billboard->y - cury, billboard->z - curz);
                    entry.text = billboard->text;
                    entry.color = billboard->color;
                    textBillboards.append(entry);
                }
            }

            glColor4f(colour[0], colour[1], colour[2], colour[3]);
        }
    }
}

// this sets curx, cury, curz based on hrm internal rules
void RendererWidget::setupNewBaseCoordinates()
{
    CRoom *p = nullptr;
    CRoom *newRoom = nullptr;
    unsigned long long bestDistance, dist;
    int newX, newY, newZ;
    unsigned int i;

    print_debug(DEBUG_RENDERER, "calculating new Base coordinates");

    // in case we lost sync, stay at the last position
    if (stacker.amount() == 0)
        return;

    // initial unbeatably worst value for euclidean test
    bestDistance = (long long)32000 * 32000 * 1000;
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

    if (newRoom != nullptr) {
        curx = newRoom->getX();
        cury = newRoom->getY();
        curz = newRoom->getZ() + userLayerShift;

        //        printf("Base room: %i\r\n", newRoom->id);
        //        fflush(stdout);
    } else {
        //        printf("No base room for coordinates setup found!\r\n");
        //        fflush(stdout);
    }
}

void RendererWidget::centerOnRoom(unsigned int id)
{
    CRoom *r = Map.getRoom(id);

    userX = (double)(curx - r->getX());
    userY = (double)(cury - r->getY());
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
    // const float alphaChannelTable[] = { 0.95, 0.35, 0.30, 0.28, 0.25, 0.15, 0.15, 0.13, 0.1, 0.1, 0.1};
    const float alphaChannelTable[] = {0.95, 0.25, 0.20, 0.15, 0.10, 0.1, 0.1, 0.1, 0.1, 0.1, 0.1};
    //                                       0    1     2      3    4      5     6    7    8     9    10

    makeCurrent();

    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    const qreal dpr = devicePixelRatioF();
    glViewport(0, 0, static_cast<GLint>(width() * dpr), static_cast<GLint>(height() * dpr));

    redraw = false;
    int z = 0;

    print_debug(DEBUG_RENDERER, "in draw()");

    // Always clear the screen first to avoid artifacts
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (Map.isBlocked()) {
        // Map is blocked, just show cleared screen and retry later
        printf("Map is blocked. Delaying the redraw\r\n");
        fflush(stdout);
        print_debug(DEBUG_GENERAL, "Map is blocked. Delaying the redraw.");
        QTimer::singleShot(500, this, SLOT(display()));
        return;
    }

    glLoadIdentity();

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    //    glEnable(GL_DEPTH_TEST);
    //    glColor3ub(255, 0, 0);

    glTranslatef(0, 0, userZ);

    glRotatef(angleX, 1.0f, 0.0f, 0.0f);
    glRotatef(angleY, 0.0f, 1.0f, 0.0f);
    glRotatef(angleZ, 0.0f, 0.0f, 1.0f);
    glTranslatef(userX, userY, 0);

    setupNewBaseCoordinates();
    textBillboards.clear();

    frustum.calculateFrustum(curx, cury, curz);

    // calculate the lower and the upper borders
    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;

    lowerZ = curz - (side * 2);
    upperZ = curz + (side * 2);
    upperZ -= (1 - visibleLayers % 2) << 1;
    //    print_debug(DEBUG_RENDERER, "drawing %i rooms", Map.size());

    glColor4f(0.1, 0.8, 0.8, 0.4);
    colour[0] = 0.1;
    colour[1] = 0.8;
    colour[2] = 0.8;
    colour[3] = 0.4;

    plane = Map.getPlanes();
    resetRenderBatch();
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

        colour[0] = 1;
        colour[1] = 1;
        colour[2] = 1;
        colour[3] = alphaChannelTable[z];

        glColor4f(colour[0], colour[1], colour[2], colour[3]);

        current_plane_z = plane->z;

        glDrawCSquare(plane->squares, GL_RENDER);

        plane = plane->next;
    }

    glDrawMarkers();
    glDrawGroupMarkers();
    glDrawPrespamLine();

    if (!renderVertices.isEmpty()) {
        QMatrix4x4 viewMatrix;
        viewMatrix.setToIdentity();
        viewMatrix.translate(0.0f, 0.0f, userZ);
        viewMatrix.rotate(angleX, 1.0f, 0.0f, 0.0f);
        viewMatrix.rotate(angleY, 0.0f, 1.0f, 0.0f);
        viewMatrix.rotate(angleZ, 0.0f, 0.0f, 1.0f);
        viewMatrix.translate(userX, userY, 0.0f);

        QMatrix4x4 mvp = projectionMatrix * viewMatrix;
        renderBatch(renderVertices, renderCommands, mvp);
    }

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

    // if (p->id == 20989 || p->id == 20973)
    //   printf("preparing to render the room in pickup mode! id %i\r\n", p->id);

    if (frustum.isPointInFrustum(dx, dy, dz) != true)
        return;

    // printf("Rendering the pickup room %i\r\n", p->id);

    glTranslatef(dx, dy, dz);
    glLoadName(p->id + 1);
    glCallList(basic_gllist);
    glTranslatef(-dx, -dy, -dz);
}

bool RendererWidget::doSelect(QPoint pos, unsigned int &id)
{
    makeCurrent();

    GLboolean blendEnabled = glIsEnabled(GL_BLEND);
    GLboolean textureEnabled = glIsEnabled(GL_TEXTURE_2D);
    GLfloat clearColor[4] = {0.0f, 0.0f, 0.0f, 0.0f};
    glGetFloatv(GL_COLOR_CLEAR_VALUE, clearColor);

    QSize fboSize(width(), height());
    if (!selectionFbo || selectionFbo->size() != fboSize) {
        delete selectionFbo;
        QOpenGLFramebufferObjectFormat format;
        format.setAttachment(QOpenGLFramebufferObject::Depth);
        format.setInternalTextureFormat(GL_RGBA8);
        selectionFbo = new QOpenGLFramebufferObject(fboSize, format);
    }

    selectionFbo->bind();
    glViewport(0, 0, fboSize.width(), fboSize.height());
    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setupNewBaseCoordinates();
    frustum.calculateFrustum(curx, cury, curz);

    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;
    lowerZ = curz - (side * 2);
    upperZ = curz + (side * 2);
    upperZ -= (1 - visibleLayers % 2) << 1;

    QVector<RenderVertex> pickVertices;
    QVector<RenderCommand> pickCommands;
    pickVertices.reserve(4096);

    auto appendPickCommand = [&](GLenum mode, int first, int count) {
        if (!pickCommands.isEmpty()) {
            RenderCommand &last = pickCommands.last();
            if (last.mode == mode && last.useTexture == false && last.texture == 0 &&
                last.first + last.count == first) {
                last.count += count;
                return;
            }
        }
        RenderCommand cmd;
        cmd.mode = mode;
        cmd.first = first;
        cmd.count = count;
        cmd.texture = 0;
        cmd.useTexture = false;
        pickCommands.append(cmd);
    };

    auto appendPickQuad = [&](const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d,
                              const GLfloat *color) {
        RenderVertex v0 = {{a.x(), a.y(), a.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
        RenderVertex v1 = {{b.x(), b.y(), b.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
        RenderVertex v2 = {{c.x(), c.y(), c.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};
        RenderVertex v3 = {{d.x(), d.y(), d.z()}, {0.0f, 0.0f}, {color[0], color[1], color[2], color[3]}};

        int first = pickVertices.size();
        pickVertices.append(v0);
        pickVertices.append(v1);
        pickVertices.append(v2);
        pickVertices.append(v0);
        pickVertices.append(v2);
        pickVertices.append(v3);
        appendPickCommand(GL_TRIANGLES, first, 6);
    };

    auto encodeColor = [](unsigned int value, GLfloat *outColor) {
        outColor[0] = static_cast<GLfloat>(value & 0xFF) / 255.0f;
        outColor[1] = static_cast<GLfloat>((value >> 8) & 0xFF) / 255.0f;
        outColor[2] = static_cast<GLfloat>((value >> 16) & 0xFF) / 255.0f;
        outColor[3] = 1.0f;
    };

    CPlane *plane = Map.getPlanes();
    while (plane) {
        if (plane->z < lowerZ || plane->z > upperZ) {
            plane = plane->next;
            continue;
        }

        current_plane_z = plane->z;
        CSquare *square = plane->squares;
        if (!square) {
            plane = plane->next;
            continue;
        }

        QVector<CSquare *> stack;
        stack.append(square);
        while (!stack.isEmpty()) {
            CSquare *sq = stack.takeLast();
            if (!frustum.isSquareInFrustum(sq))
                continue;

            if (sq->toBePassed()) {
                for (int i = 0; i < 4; i++) {
                    if (sq->subsquares[i])
                        stack.append(sq->subsquares[i]);
                }
                continue;
            }

            for (int k = 0; k < sq->rooms.size(); k++) {
                CRoom *room = sq->rooms[k];
                if (!room)
                    continue;

                float dx = room->getX() - curx;
                float dy = room->getY() - cury;
                float dz = room->getZ() - curz;
                if (frustum.isPointInFrustum(dx, dy, dz) != true)
                    continue;

                GLfloat pickColor[4];
                encodeColor(room->id + 1, pickColor);
                QVector3D a(dx - ROOM_SIZE, dy - ROOM_SIZE, dz);
                QVector3D b(dx + ROOM_SIZE, dy - ROOM_SIZE, dz);
                QVector3D c(dx + ROOM_SIZE, dy + ROOM_SIZE, dz);
                QVector3D d(dx - ROOM_SIZE, dy + ROOM_SIZE, dz);
                appendPickQuad(a, b, c, d, pickColor);
            }
        }

        plane = plane->next;
    }

    QMatrix4x4 viewMatrix;
    viewMatrix.setToIdentity();
    viewMatrix.translate(0.0f, 0.0f, userZ);
    viewMatrix.rotate(angleX, 1.0f, 0.0f, 0.0f);
    viewMatrix.rotate(angleY, 0.0f, 1.0f, 0.0f);
    viewMatrix.rotate(angleZ, 0.0f, 0.0f, 1.0f);
    viewMatrix.translate(userX, userY, 0.0f);

    QMatrix4x4 mvp = projectionMatrix * viewMatrix;
    renderBatch(pickVertices, pickCommands, mvp);

    int readSize = PICK_TOL;
    int half = readSize / 2;
    int x = pos.x();
    int y = fboSize.height() - 1 - pos.y();
    int x0 = std::max(0, x - half);
    int y0 = std::max(0, y - half);
    int w = std::min(readSize, fboSize.width() - x0);
    int h = std::min(readSize, fboSize.height() - y0);

    QVector<unsigned char> pixels(w * h * 4);
    glReadPixels(x0, y0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pixels.data());

    bool selected = false;
    int bestDist = 0x7fffffff;
    unsigned int bestId = 0;
    for (int yy = 0; yy < h; yy++) {
        for (int xx = 0; xx < w; xx++) {
            int index = (yy * w + xx) * 4;
            unsigned int value = pixels[index] | (pixels[index + 1] << 8) | (pixels[index + 2] << 16);
            if (value == 0)
                continue;
            int dx = (x0 + xx) - x;
            int dy = (y0 + yy) - y;
            int dist = dx * dx + dy * dy;
            if (dist < bestDist) {
                bestDist = dist;
                bestId = value - 1;
                selected = true;
            }
        }
    }

    selectionFbo->release();
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
    glViewport(0, 0, fboSize.width(), fboSize.height());
    if (blendEnabled) {
        glEnable(GL_BLEND);
    } else {
        glDisable(GL_BLEND);
    }
    if (textureEnabled) {
        glEnable(GL_TEXTURE_2D);
    } else {
        glDisable(GL_TEXTURE_2D);
    }
    glClearColor(clearColor[0], clearColor[1], clearColor[2], clearColor[3]);

    if (selected)
        print_debug(DEBUG_INTERFACE, "Clicked on : %i", bestId);
    else
        print_debug(DEBUG_INTERFACE, "Selection failed");
    id = bestId;
    return selected;
}
