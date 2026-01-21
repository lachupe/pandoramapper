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

static QByteArray pixmapPath(const char *base)
{
    return QByteArray(":/pixmaps/") + base + ".png";
}

struct IconMapping
{
    uint32_t flag;
    const char *name;
};

static const IconMapping mobIconMap[] = {
    {MM_MOB_RENT, "mob-rent"},
    {MM_MOB_SHOP, "mob-shop"},
    {MM_MOB_WEAPON_SHOP, "mob-weaponshop"},
    {MM_MOB_ARMOUR_SHOP, "mob-armourshop"},
    {MM_MOB_FOOD_SHOP, "mob-foodshop"},
    {MM_MOB_PET_SHOP, "mob-petshop"},
    {MM_MOB_GUILD, "mob-guild"},
    {MM_MOB_SCOUT_GUILD, "mob-scoutguild"},
    {MM_MOB_MAGE_GUILD, "mob-mageguild"},
    {MM_MOB_CLERIC_GUILD, "mob-clericguild"},
    {MM_MOB_WARRIOR_GUILD, "mob-warriorguild"},
    {MM_MOB_RANGER_GUILD, "mob-rangerguild"},
    {MM_MOB_AGGRESSIVE_MOB, "mob-aggmob"},
    {MM_MOB_QUEST_MOB, "mob-questmob"},
    {MM_MOB_PASSIVE_MOB, "mob-passivemob"},
    {MM_MOB_ELITE_MOB, "mob-elitemob"},
    {MM_MOB_SUPER_MOB, "mob-smob"},
    {MM_MOB_MILKABLE, "mob-milkable"},
    {MM_MOB_RATTLESNAKE, "mob-rattlesnake"},
};

static const IconMapping loadIconMap[] = {
    {MM_LOAD_TREASURE, "load-treasure"},
    {MM_LOAD_ARMOUR, "load-armour"},
    {MM_LOAD_WEAPON, "load-weapon"},
    {MM_LOAD_WATER, "load-water"},
    {MM_LOAD_FOOD, "load-food"},
    {MM_LOAD_HERB, "load-herb"},
    {MM_LOAD_KEY, "load-key"},
    {MM_LOAD_MULE, "load-mule"},
    {MM_LOAD_HORSE, "load-horse"},
    {MM_LOAD_PACK_HORSE, "load-pack"},
    {MM_LOAD_TRAINED_HORSE, "load-trained"},
    {MM_LOAD_ROHIRRIM, "load-rohirrim"},
    {MM_LOAD_WARG, "load-warg"},
    {MM_LOAD_BOAT, "load-boat"},
    {MM_LOAD_ATTENTION, "load-attention"},
    {MM_LOAD_TOWER, "load-watch"},
    {MM_LOAD_CLOCK, "load-clock"},
    {MM_LOAD_MAIL, "load-mail"},
    {MM_LOAD_STABLE, "load-stable"},
    {MM_LOAD_WHITE_WORD, "load-whiteword"},
    {MM_LOAD_DARK_WORD, "load-darkword"},
    {MM_LOAD_EQUIPMENT, "load-equipment"},
    {MM_LOAD_COACH, "load-coach"},
    {MM_LOAD_FERRY, "load-ferry"},
    {MM_LOAD_DEATHTRAP, "load-deathtrap"},
};

static const int mobIconCount = sizeof(mobIconMap) / sizeof(mobIconMap[0]);
static const int loadIconCount = sizeof(loadIconMap) / sizeof(loadIconMap[0]);

static const char *roadMaskToName(int mask)
{
    switch (mask) {
    case 0:
        return "road-none";
    case 1:
        return "road-n";
    case 2:
        return "road-e";
    case 4:
        return "road-s";
    case 8:
        return "road-w";
    case 3:
        return "road-ne";
    case 5:
        return "road-ns";
    case 9:
        return "road-nw";
    case 6:
        return "road-es";
    case 10:
        return "road-ew";
    case 12:
        return "road-sw";
    case 7:
        return "road-nes";
    case 11:
        return "road-new";
    case 13:
        return "road-nsw";
    case 14:
        return "road-esw";
    case 15:
        return "road-all";
    default:
        return "road-none";
    }
}

static const char *trailMaskToName(int mask)
{
    switch (mask) {
    case 0:
        return "trail-none";
    case 1:
        return "trail-n";
    case 2:
        return "trail-e";
    case 4:
        return "trail-s";
    case 8:
        return "trail-w";
    case 3:
        return "trail-ne";
    case 5:
        return "trail-ns";
    case 9:
        return "trail-nw";
    case 6:
        return "trail-es";
    case 10:
        return "trail-ew";
    case 12:
        return "trail-sw";
    case 7:
        return "trail-nes";
    case 11:
        return "trail-new";
    case 13:
        return "trail-nsw";
    case 14:
        return "trail-esw";
    case 15:
        return "trail-all";
    default:
        return "trail-none";
    }
}

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
    baseRoom = nullptr;

    userLayerShift = 0;

    last_drawn_marker = 0;
    last_drawn_trail = 0;

    for (int i = 0; i < 4; ++i) {
        wall_textures[i] = 0;
        door_textures[i] = 0;
    }
    for (int i = 0; i < 16; ++i) {
        road_textures[i] = 0;
        trail_textures[i] = 0;
    }
    for (int i = 0; i < MOB_FLAG_COUNT; ++i) {
        mob_textures[i] = 0;
    }
    for (int i = 0; i < LOAD_FLAG_COUNT; ++i) {
        load_textures[i] = 0;
    }
    no_ride_texture = 0;

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

    glDisable(GL_LINE_SMOOTH);
    glDisable(GL_POLYGON_SMOOTH);

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
    conf->loadNormalTexture(":/images/exit_normal.png", &conf->exit_normal_texture);
    conf->loadNormalTexture(":/images/exit_door.png", &conf->exit_door_texture);
    conf->loadNormalTexture(":/images/exit_secret.png", &conf->exit_secret_texture);
    conf->loadNormalTexture(":/images/exit_undef.png", &conf->exit_undef_texture);
    for (int mask = 0; mask < 16; ++mask) {
        conf->loadNormalTexture(pixmapPath(roadMaskToName(mask)), &road_textures[mask]);
        conf->loadNormalTexture(pixmapPath(trailMaskToName(mask)), &trail_textures[mask]);
    }

    for (int i = 0; i < mobIconCount && i < MOB_FLAG_COUNT; ++i) {
        conf->loadNormalTexture(pixmapPath(mobIconMap[i].name), &mob_textures[i]);
    }
    for (int i = 0; i < loadIconCount && i < LOAD_FLAG_COUNT; ++i) {
        conf->loadNormalTexture(pixmapPath(loadIconMap[i].name), &load_textures[i]);
    }
    conf->loadNormalTexture(pixmapPath("no-ride"), &no_ride_texture);
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
    float dx, dy, dz;

    if (stacker.amount() == 0)
        return;

    GLfloat markerColor[4] = {marker_colour[0], marker_colour[1], marker_colour[2], marker_colour[3]};
    for (k = 0; k < stacker.amount(); k++) {
        p = stacker.get(k);

        if (p == nullptr) {
            print_debug(DEBUG_RENDERER, "RENDERER ERROR: Stuck upon corrupted room while drawing red pointers.\r\n");
            continue;
        }

        RenderTransform t = getRenderTransform(p);
        dx = t.pos.x() - curx;
        dy = t.pos.y() - cury;
        dz = t.pos.z() - curz;

        appendMarkerGeometry(dx, dy, dz, 2, markerColor, t.scale);

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

            appendConeGeometry(dx, dy, dz + 0.2f * t.scale, rotX, rotY, markerColor, t.scale);
        } else {
            appendConeGeometry(dx, dy, dz + 0.2f * t.scale, 0.0f, 0.0f, markerColor, t.scale);
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
    float prevx, prevy, prevz, dx, dy, dz;

    if (!line)
        return;

    CRoom *p = Map.getRoom(line->at(0));

    if (p == nullptr) {
        return;
    }

    RenderTransform first = getRenderTransform(p);
    prevx = first.pos.x() - curx;
    prevy = first.pos.y() - cury;
    prevz = first.pos.z() - curz;
    float prevScale = first.scale;

    GLfloat lineColor[4] = {1.0f, 0.0f, 1.0f, 1.0f};

    for (int i = 1; i < line->size(); i++) {
        // connect all rooms with lines
        unsigned int id = line->at(i);

        CRoom *p = Map.getRoom(id);

        if (p == nullptr)
            continue;

        RenderTransform t = getRenderTransform(p);
        dx = t.pos.x() - curx;
        dy = t.pos.y() - cury;
        dz = t.pos.z() - curz;
        prevScale = t.scale;

        // glDrawMarkerPrimitive(dx, dy, dz, 2);

        // glTranslatef(dx, dy, dz + 0.2f);

        appendLine(QVector3D(prevx, prevy, prevz), QVector3D(dx, dy, dz), lineColor);

        prevx = dx;
        prevy = dy;
        prevz = dz;
    }

    // and draw a cone in the last room
    appendConeGeometry(prevx, prevy, prevz + 0.2f * prevScale, 0.0f, 0.0f, lineColor, prevScale);

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

        RenderTransform t = getRenderTransform(p);
        dx = t.pos.x() - curx;
        dy = t.pos.y() - cury;
        dz = t.pos.z() - curz;

        appendMarkerGeometry(dx, dy, dz, 2, markerColor, t.scale);

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

            appendConeGeometry(dx, dy, dz + 0.2f * t.scale, rotX, rotY, markerColor, t.scale);
        } else {
            appendConeGeometry(dx, dy, dz + 0.2f * t.scale, 0.0f, 0.0f, markerColor, t.scale);
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

void RendererWidget::appendWallPrism(float x0, float x1, float y0, float y1, float z0, float z1,
                                     const GLfloat *color, GLuint texture)
{
    QVector3D v000(x0, y0, z0);
    QVector3D v001(x0, y0, z1);
    QVector3D v010(x0, y1, z0);
    QVector3D v011(x0, y1, z1);
    QVector3D v100(x1, y0, z0);
    QVector3D v101(x1, y0, z1);
    QVector3D v110(x1, y1, z0);
    QVector3D v111(x1, y1, z1);

    auto addFace = [&](const QVector3D &a, const QVector3D &b, const QVector3D &c, const QVector3D &d) {
        if (texture != 0) {
            appendTexturedQuad(a, b, c, d, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
                               QVector2D(1.0f, 0.0f), QVector2D(1.0f, 1.0f), color, texture);
        } else {
            appendQuad(a, b, c, d, color);
        }
    };

    addFace(v011, v111, v110, v010); /* north */
    addFace(v001, v000, v100, v101); /* south */
    addFace(v001, v011, v010, v000); /* west */
    addFace(v101, v100, v110, v111); /* east */
    addFace(v001, v101, v111, v011); /* top */
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

void RendererWidget::appendMarkerGeometry(float dx, float dy, float dz, int mode, const GLfloat *color,
                                          float scaleFactor)
{
    float markerSize = MARKER_SIZE * scaleFactor;
    float roomSize = ROOM_SIZE * scaleFactor;

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

    QVector3D upperA(dx, markerSize + dy + roomSize, dz);
    QVector3D upperB(-markerSize + dx, dy + roomSize, dz);
    QVector3D upperC(markerSize + dx, dy + roomSize, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(upperA);
    pushVertex(upperB);
    pushVertex(upperC);

    QVector3D lowerA(dx, -markerSize + dy - roomSize, dz);
    QVector3D lowerB(-markerSize + dx, dy - roomSize, dz);
    QVector3D lowerC(markerSize + dx, dy - roomSize, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(lowerA);
    pushVertex(lowerB);
    pushVertex(lowerC);

    QVector3D rightA(dx + roomSize, markerSize + dy, dz);
    QVector3D rightB(markerSize + dx + roomSize, dy, dz);
    QVector3D rightC(dx + roomSize, -markerSize + dy, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(rightA);
    pushVertex(rightB);
    pushVertex(rightC);

    QVector3D leftA(dx - roomSize, markerSize + dy, dz);
    QVector3D leftB(-markerSize + dx - roomSize, dy, dz);
    QVector3D leftC(dx - roomSize, -markerSize + dy, dz);
    appendCommand(GL_TRIANGLES, false, 0, renderVertices.size(), 3);
    pushVertex(leftA);
    pushVertex(leftB);
    pushVertex(leftC);

    if (mode == 1) {
        float inset = markerSize / 3.5f;
        QVector3D l0(dx - roomSize - inset, dy + roomSize + inset, dz);
        QVector3D l1(dx - roomSize - inset, dy - roomSize, dz);
        QVector3D l2(dx - roomSize, dy - roomSize, dz);
        QVector3D l3(dx - roomSize, dy + roomSize + inset, dz);
        appendQuad(l0, l1, l2, l3, color);

        QVector3D r0(dx + roomSize, dy + roomSize + inset, dz);
        QVector3D r1(dx + roomSize, dy - roomSize, dz);
        QVector3D r2(dx + roomSize + inset, dy - roomSize, dz);
        QVector3D r3(dx + roomSize + inset, dy + roomSize + inset, dz);
        appendQuad(r0, r1, r2, r3, color);

        QVector3D u0(dx - roomSize - inset, dy + roomSize + inset, dz);
        QVector3D u1(dx - roomSize - inset, dy + roomSize, dz);
        QVector3D u2(dx + roomSize + inset, dy + roomSize, dz);
        QVector3D u3(dx + roomSize + inset, dy + roomSize + inset, dz);
        appendQuad(u0, u1, u2, u3, color);

        QVector3D d0(dx - roomSize - inset, dy - roomSize, dz);
        QVector3D d1(dx - roomSize - inset, dy - roomSize + inset, dz);
        QVector3D d2(dx + roomSize + inset, dy - roomSize + inset, dz);
        QVector3D d3(dx + roomSize + inset, dy - roomSize, dz);
        appendQuad(d0, d1, d2, d3, color);
    }
}

void RendererWidget::appendConeGeometry(float dx, float dy, float dz, float rotX, float rotY, const GLfloat *color,
                                        float scaleFactor)
{
    const float height = 0.36f * scaleFactor;
    const float radius = 0.08f * scaleFactor;
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

    QVector3D tip = transform.map(QVector3D(0.0f, 0.0f, height));

    for (int i = 0; i < segments; i++) {
        float a0 = (static_cast<float>(i) / segments) * twoPi;
        float a1 = (static_cast<float>(i + 1) / segments) * twoPi;

        QVector3D base0(radius * std::cos(a0), radius * std::sin(a0), 0.0f);
        QVector3D base1(radius * std::cos(a1), radius * std::sin(a1), 0.0f);
        base0 = transform.map(base0);
        base1 = transform.map(base1);

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

    QMatrix4x4 viewMatrix;
    viewMatrix.setToIdentity();
    viewMatrix.translate(0.0f, 0.0f, userZ);
    viewMatrix.rotate(angleX, 1.0f, 0.0f, 0.0f);
    viewMatrix.rotate(angleY, 0.0f, 1.0f, 0.0f);
    viewMatrix.rotate(angleZ, 0.0f, 0.0f, 1.0f);
    viewMatrix.translate(userX, userY, 0.0f);

    QMatrix4x4 mvp = projectionMatrix * viewMatrix;
    const float maxDistance = conf->getNotesVisibilityRange();
    const float fadeStart = maxDistance * 0.7f;
    const float depthEpsilon = 0.002f;

    struct DrawText {
        QPointF screen;
        QString text;
        QColor color;
    };
    QVector<DrawText> drawList;
    drawList.reserve(textBillboards.size());

    for (int i = 0; i < textBillboards.size(); i++) {
        const TextBillboard &entry = textBillboards[i];
        QVector4D view = viewMatrix * QVector4D(entry.position, 1.0f);
        float dist = view.toVector3D().length();
        if (dist > maxDistance)
            continue;

        QVector4D clip = projectionMatrix * view;
        if (clip.w() <= 0.0f)
            continue;

        float invW = 1.0f / clip.w();
        float ndcX = clip.x() * invW;
        float ndcY = clip.y() * invW;
        float ndcZ = clip.z() * invW;

        if (ndcX < -1.0f || ndcX > 1.0f || ndcY < -1.0f || ndcY > 1.0f)
            continue;

        float screenX = (ndcX * 0.5f + 0.5f) * static_cast<float>(width());
        float screenY = (1.0f - (ndcY * 0.5f + 0.5f)) * static_cast<float>(height());

        float depth = ndcZ * 0.5f + 0.5f;
        int px = std::clamp(static_cast<int>(screenX), 0, width() - 1);
        int py = std::clamp(static_cast<int>(screenY), 0, height() - 1);
        float depthBuffer = 1.0f;
        glReadPixels(px, height() - 1 - py, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthBuffer);
        if (depth > depthBuffer + depthEpsilon)
            continue;

        float fade = 1.0f;
        if (dist > fadeStart && maxDistance > fadeStart) {
            fade = 1.0f - (dist - fadeStart) / (maxDistance - fadeStart);
            fade = std::clamp(fade, 0.0f, 1.0f);
        }

        QColor color = entry.color;
        color.setAlphaF(color.alphaF() * fade);
        drawList.append({QPointF(screenX, screenY), entry.text, color});
    }

    if (drawList.isEmpty())
        return;

    QPainter painter(this);
    painter.setRenderHint(QPainter::TextAntialiasing, true);
    painter.setFont(*textFont);

    for (const DrawText &entry : drawList) {
        painter.setPen(entry.color);
        painter.drawText(entry.screen, entry.text);
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

            RenderTransform t = getRenderTransform(p);
            square->notesBillboards.append(
                new Billboard(t.pos.x(), t.pos.y(), t.pos.z() + (ROOM_SIZE * 0.5f * t.scale), p->getNote(), color));
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

                RenderTransform tp = getRenderTransform(p);
                RenderTransform tr = getRenderTransform(r);
                double deltaX = (tr.pos.x() - tp.pos.x()) / 3.0;
                double deltaY = (tr.pos.y() - tp.pos.y()) / 3.0;
                double deltaZ = (tr.pos.z() - tp.pos.z()) / 3.0;
                double roomSize = ROOM_SIZE * tp.scale;

                double shiftX = 0;
                double shiftY = 0;
                double shiftZ = roomSize / 2.0;

                if (deltaX < 0) {
                    shiftY = roomSize / 2.0;
                } else {
                    shiftY = -roomSize / 2.0;
                }

                if (deltaY != 0) {
                    shiftX = -roomSize;
                }

                if (deltaZ < 0) {
                    shiftZ *= -1;
                }

                double x = tp.pos.x() + deltaX + shiftX;
                double y = tp.pos.y() + deltaY + shiftY;
                double z = tp.pos.z() + deltaZ + shiftZ;

                square->doorsBillboards.append(new Billboard(x, y, z, info, QColor(255, 255, 255, 255)));
            }
        }
    }

    square->rebuild_display_list = false;
}

void RendererWidget::appendRoomGeometry(CRoom *p)
{
    RenderTransform t = getRenderTransform(p);
    float dx = t.pos.x() - curx;
    float dy = t.pos.y() - cury;
    float dz = t.pos.z() - curz;
    float size = ROOM_SIZE * t.scale;
    float xMin = dx - size;
    float xMax = dx + size;
    float yMin = dy - size;
    float yMax = dy + size;
    float z0 = dz;
    float z1 = dz + WALL_HEIGHT * t.scale;
    float doorHalf = (DOOR_WIDTH * 0.5f) * t.scale;
    float wallThickness = WALL_THICKNESS * t.scale;

    if (p->getTerrain()) {
        GLuint texture = conf->sectors[p->getTerrain()].texture;
        QVector3D a(dx - size, dy + size, dz);
        QVector3D b(dx - size, dy - size, dz);
        QVector3D c(dx + size, dy - size, dz);
        QVector3D d(dx + size, dy + size, dz);
        if (texture != 0) {
            appendTexturedQuad(a, b, c, d, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
                               QVector2D(1.0f, 0.0f), QVector2D(1.0f, 1.0f), colour, texture);
        } else {
            GLfloat floorColor[4] = {0.35f, 0.35f, 0.35f, colour[3]};
            appendQuad(a, b, c, d, floorColor);
        }
    } else {
        GLfloat floorColor[4] = {0.35f, 0.35f, 0.35f, colour[3]};
        QVector3D a(dx - size, dy - size, dz);
        QVector3D b(dx + size, dy - size, dz);
        QVector3D c(dx + size, dy + size, dz);
        QVector3D d(dx - size, dy + size, dz);
        appendQuad(a, b, c, d, floorColor);
    }

    int roadMask = 0;
    if (p->getMMExitFlags(NORTH) & MM_EXIT_ROAD)
        roadMask |= 1;
    if (p->getMMExitFlags(EAST) & MM_EXIT_ROAD)
        roadMask |= 2;
    if (p->getMMExitFlags(SOUTH) & MM_EXIT_ROAD)
        roadMask |= 4;
    if (p->getMMExitFlags(WEST) & MM_EXIT_ROAD)
        roadMask |= 8;

    bool isRoadTerrain = false;
    int terrainIndex = p->getTerrain();
    if (terrainIndex >= 0 && terrainIndex < conf->sectors.size()) {
        QByteArray terrainDesc = conf->sectors[terrainIndex].desc;
        isRoadTerrain = (terrainDesc.toUpper() == "ROAD");
    }

    GLuint overlayTexture = 0;
    if (isRoadTerrain) {
        overlayTexture = road_textures[roadMask];
    } else if (roadMask != 0) {
        overlayTexture = trail_textures[roadMask];
    }

    if (overlayTexture != 0) {
        GLfloat overlayColor[4] = {1.0f, 1.0f, 1.0f, colour[3]};
        float overlayZ = dz + 0.02f * t.scale;
        QVector3D a(dx - size, dy + size, overlayZ);
        QVector3D b(dx - size, dy - size, overlayZ);
        QVector3D c(dx + size, dy - size, overlayZ);
        QVector3D d(dx + size, dy + size, overlayZ);
        appendTexturedQuad(a, b, c, d, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
                           QVector2D(1.0f, 0.0f), QVector2D(1.0f, 1.0f), overlayColor, overlayTexture);
    }

    QVector<GLuint> iconTextures;
    uint32_t mobFlags = p->getMobFlags();
    for (int i = 0; i < mobIconCount && i < MOB_FLAG_COUNT; ++i) {
        if (mobFlags & mobIconMap[i].flag) {
            GLuint tex = mob_textures[i];
            if (tex != 0)
                iconTextures.append(tex);
        }
    }
    uint32_t loadFlags = p->getLoadFlags();
    for (int i = 0; i < loadIconCount && i < LOAD_FLAG_COUNT; ++i) {
        if (loadFlags & loadIconMap[i].flag) {
            GLuint tex = load_textures[i];
            if (tex != 0)
                iconTextures.append(tex);
        }
    }
    if (p->getRidableType() == MM_RIDABLE_NOT_RIDABLE && no_ride_texture != 0) {
        iconTextures.append(no_ride_texture);
    }

    if (!iconTextures.isEmpty()) {
        GLfloat iconColor[4] = {1.0f, 1.0f, 1.0f, colour[3]};
        float iconZ = dz + 0.03f * t.scale;
        float inset = size * 0.15f;
        float available = (size * 2.0f) - (inset * 2.0f);
        int count = iconTextures.size();
        int cols = static_cast<int>(std::ceil(std::sqrt(static_cast<float>(count))));
        int rows = (count + cols - 1) / cols;
        float cellW = available / cols;
        float cellH = available / rows;
        float iconSize = std::min(cellW, cellH);
        float startX = dx - size + inset;
        float startY = dy + size - inset;
        for (int i = 0; i < count; ++i) {
            int row = i / cols;
            int col = i % cols;
            float x0 = startX + col * cellW + (cellW - iconSize) * 0.5f;
            float y1 = startY - row * cellH - (cellH - iconSize) * 0.5f;
            float x1 = x0 + iconSize;
            float y0 = y1 - iconSize;
            QVector3D a(x0, y1, iconZ);
            QVector3D b(x0, y0, iconZ);
            QVector3D c(x1, y0, iconZ);
            QVector3D d(x1, y1, iconZ);
            appendTexturedQuad(a, b, c, d, QVector2D(0.0f, 1.0f), QVector2D(0.0f, 0.0f),
                               QVector2D(1.0f, 0.0f), QVector2D(1.0f, 1.0f), iconColor, iconTextures[i]);
        }
    }

    const GLfloat wallColor[4] = {0.45f, 0.45f, 0.45f, colour[3]};
    const GLfloat doorColor[4] = {0.70f, 0.55f, 0.35f, colour[3]};

    for (int dir = NORTH; dir <= WEST; ++dir) {
        bool hasExit = p->isExitPresent(dir);
        bool hasDoor = !p->getDoor(dir).isEmpty() || (p->getMMExitFlags(dir) & MM_EXIT_DOOR) ||
                       (p->getMMDoorFlags(dir) != 0);

        if (dir == NORTH) {
            float wy0 = yMax - wallThickness;
            float wy1 = yMax;
            if (!hasExit) {
                appendWallPrism(xMin, xMax, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
            } else if (hasDoor) {
                appendWallPrism(xMin, dx - doorHalf, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(dx + doorHalf, xMax, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(dx - doorHalf, dx + doorHalf, wy0, wy1, z0, z1, doorColor, door_textures[dir]);
            }
        } else if (dir == SOUTH) {
            float wy0 = yMin;
            float wy1 = yMin + wallThickness;
            if (!hasExit) {
                appendWallPrism(xMin, xMax, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
            } else if (hasDoor) {
                appendWallPrism(xMin, dx - doorHalf, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(dx + doorHalf, xMax, wy0, wy1, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(dx - doorHalf, dx + doorHalf, wy0, wy1, z0, z1, doorColor, door_textures[dir]);
            }
        } else if (dir == EAST) {
            float wx0 = xMax - wallThickness;
            float wx1 = xMax;
            if (!hasExit) {
                appendWallPrism(wx0, wx1, yMin, yMax, z0, z1, wallColor, wall_textures[dir]);
            } else if (hasDoor) {
                appendWallPrism(wx0, wx1, yMin, dy - doorHalf, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(wx0, wx1, dy + doorHalf, yMax, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(wx0, wx1, dy - doorHalf, dy + doorHalf, z0, z1, doorColor, door_textures[dir]);
            }
        } else if (dir == WEST) {
            float wx0 = xMin;
            float wx1 = xMin + wallThickness;
            if (!hasExit) {
                appendWallPrism(wx0, wx1, yMin, yMax, z0, z1, wallColor, wall_textures[dir]);
            } else if (hasDoor) {
                appendWallPrism(wx0, wx1, yMin, dy - doorHalf, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(wx0, wx1, dy + doorHalf, yMax, z0, z1, wallColor, wall_textures[dir]);
                appendWallPrism(wx0, wx1, dy - doorHalf, dy + doorHalf, z0, z1, doorColor, door_textures[dir]);
            }
        }
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
            ky = +(size + 0.2f * t.scale);
            kz = 0;
            sx = 0;
            sy = +size;
        } else if (k == EAST) {
            kx = +(size + 0.2f * t.scale);
            ky = 0;
            kz = 0;
            sx = +size;
            sy = 0;
        } else if (k == SOUTH) {
            kx = 0;
            ky = -(size + 0.2f * t.scale);
            kz = 0;
            sx = 0;
            sy = -size;
        } else if (k == WEST) {
            kx = -(size + 0.2f * t.scale);
            ky = 0;
            kz = 0;
            sx = -size;
            sy = 0;
        } else if (k == UP) {
            kx = 0;
            ky = 0;
            kz = +(size + 0.2f * t.scale);
            sx = size / 2;
            sy = 0;
        } else {
            kx = 0;
            ky = 0;
            kz = -(size + 0.2f * t.scale);
            sx = 0;
            sy = size / 2;
        }

        if (p->isExitNormal(k)) {
            CRoom *r = p->exits[k];
            if (!r)
                continue;

            RenderTransform t2 = getRenderTransform(r);
            float dx2 = t2.pos.x() - curx;
            float dy2 = t2.pos.y() - cury;
            float dz2 = t2.pos.z() - curz;
            if (k <= WEST) {
                float rx = t2.pos.x() - t.pos.x();
                float ry = t2.pos.y() - t.pos.y();
                float rz = t2.pos.z() - t.pos.z();
                float dist = std::sqrt(rx * rx + ry * ry + rz * rz);
                float size2 = ROOM_SIZE * t2.scale;
                if (dist <= (size + size2 + 0.01f)) {
                    continue;
                }
            }

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
                dy2 = dy + 0.5f * t.scale;
            } else if (k == EAST) {
                dx2 = dx + 0.5f * t.scale;
            } else if (k == SOUTH) {
                dy2 = dy - 0.5f * t.scale;
            } else if (k == WEST) {
                dx2 = dx - 0.5f * t.scale;
            } else if (k == UP) {
                dz2 = dz + 0.5f * t.scale;
            } else if (k == DOWN) {
                dz2 = dz - 0.5f * t.scale;
            }

            if (k == NORTH) {
                ky = +(size);
            } else if (k == EAST) {
                kx = +(size);
            } else if (k == SOUTH) {
                ky = -(size);
            } else if (k == WEST) {
                kx = -(size);
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
                QVector3D da(dx2 + kx - size, dy2 + ky + size, dz2);
                QVector3D db(dx2 + kx - size, dy2 + ky - size, dz2);
                QVector3D dc(dx2 + kx + size, dy2 + ky - size, dz2);
                QVector3D dd(dx2 + kx + size, dy2 + ky + size, dz2);
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
        RenderTransform t = getRenderTransform(room);
        float dx = t.pos.x() - curx;
        float dy = t.pos.y() - cury;
        float dz = t.pos.z() - curz;
        if (!frustum.isPointInFrustum(dx, dy, dz))
            continue;
        appendRoomGeometry(room);

        if (!Map.selections.isEmpty() && Map.selections.isSelected(room->id) == true) {
            GLfloat highlight[4] = {0.20f, 0.20f, 0.80f, colour[3] - 0.1f};
            if (highlight[3] < 0.0f)
                highlight[3] = 0.0f;
            float size = ROOM_SIZE * 2.0f * t.scale;
            QVector3D a(dx - size, dy - size, dz);
            QVector3D b(dx + size, dy - size, dz);
            QVector3D c(dx + size, dy + size, dz);
            QVector3D d(dx - size, dy + size, dz);
            appendQuad(a, b, c, d, highlight);
        }

        if (conf->getDisplayRegionsRenderer()) {
            if (room->getRegion() && room->getRegion() == engine->get_users_region()) {
                GLfloat regionColor[4] = {0.50f, 0.50f, 0.50f, colour[3] - 0.2f};
                if (regionColor[3] < 0.0f)
                    regionColor[3] = 0.0f;
                float size = ROOM_SIZE * 1.5f * t.scale;
                QVector3D a(dx - size, dy - size, dz);
                QVector3D b(dx + size, dy - size, dz);
                QVector3D c(dx + size, dy + size, dz);
                QVector3D d(dx - size, dy + size, dz);
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
    float bestDistance, dist;
    float newX, newY, newZ;
    unsigned int i;

    print_debug(DEBUG_RENDERER, "calculating new Base coordinates");

    // in case we lost sync, stay at the last position
    if (stacker.amount() == 0)
        return;

    // initial unbeatably worst value for euclidean test
    bestDistance = 32000.0f * 32000.0f * 1000.0f;
    for (i = 0; i < stacker.amount(); i++) {
        p = stacker.get(i);
        RenderTransform t = getRenderTransform(p);
        newX = curx - t.pos.x();
        newY = cury - t.pos.y();
        newZ = curz - t.pos.z() + userLayerShift;
        dist = newX * newX + newY * newY + newZ * newZ;
        if (dist < bestDistance) {
            bestDistance = dist;
            newRoom = p;
        }
    }

    if (newRoom != nullptr) {
        RenderTransform t = getRenderTransform(newRoom);
        curx = t.pos.x();
        cury = t.pos.y();
        curz = t.pos.z() + userLayerShift;
        baseRoom = newRoom;

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

    if (!r)
        return;

    RenderTransform t = getRenderTransform(r);
    userX = (double)(curx - t.pos.x());
    userY = (double)(cury - t.pos.y());
    changeUserLayerShift(0 - static_cast<int>(std::lround(curz - t.pos.z())));

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

    Map.updateLocalSpaceBounds();
    Map.updateLocalSpaceBounds();
    setupNewBaseCoordinates();
    textBillboards.clear();

    frustum.calculateFrustum(curx, cury, curz);

    // calculate the lower and the upper borders
    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;

    int baseZ = baseRoom ? baseRoom->getZ() + userLayerShift : static_cast<int>(std::lround(curz));
    lowerZ = baseZ - (side * 2);
    upperZ = baseZ + (side * 2);
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

        z = plane->z - baseZ;
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

    Map.updateLocalSpaceBounds();
    setupNewBaseCoordinates();

    frustum.calculateFrustum(curx, cury, curz);

    // calculate the lower and the upper borders
    int visibleLayers = conf->getVisibleLayers();
    int side = visibleLayers >> 1;

    int baseZ = baseRoom ? baseRoom->getZ() + userLayerShift : static_cast<int>(std::lround(curz));
    lowerZ = baseZ - (side * 2);
    upperZ = baseZ + (side * 2);
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

        z = plane->z - baseZ;

        current_plane_z = plane->z;

        glDrawCSquare(plane->squares, GL_SELECT);
        plane = plane->next;
    }
    //    Map.unlock();
}

void RendererWidget::renderPickupRoom(CRoom *p)
{
    GLfloat dx, dy, dz;

    RenderTransform t = getRenderTransform(p);
    dx = t.pos.x() - curx;
    dy = t.pos.y() - cury;
    dz = (t.pos.z() - curz);

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

RendererWidget::RenderTransform RendererWidget::getRenderTransform(CRoom *room)
{
    RenderTransform t;
    t.pos = QVector3D(room->getX(), room->getY(), room->getZ());
    t.scale = 1.0f;

    CRegion *region = room->getRegion();
    if (!region)
        return t;
    int localSpaceId = region->getLocalSpaceId();
    if (localSpaceId <= 0)
        return t;
    LocalSpace *space = Map.getLocalSpace(localSpaceId);
    if (!space || !space->hasPortal || !space->hasBounds)
        return t;

    float localW = space->maxX - space->minX;
    float localH = space->maxY - space->minY;
    if (localW <= 0.0f || localH <= 0.0f)
        return t;

    float scale = std::min(space->portalW / localW, space->portalH / localH);
    if (scale <= 0.0f)
        return t;

    float localCx = (space->minX + space->maxX) * 0.5f;
    float localCy = (space->minY + space->maxY) * 0.5f;
    float localCz = (space->minZ + space->maxZ) * 0.5f;
    float portalCx = space->portalX;
    float portalCy = space->portalY;

    t.pos = QVector3D(portalCx + (room->getX() - localCx) * scale,
                      portalCy + (room->getY() - localCy) * scale, (room->getZ() - localCz) * scale);
    t.scale = scale;
    return t;
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
    int baseZ = baseRoom ? baseRoom->getZ() + userLayerShift : static_cast<int>(std::lround(curz));
    lowerZ = baseZ - (side * 2);
    upperZ = baseZ + (side * 2);
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

                RenderTransform t = getRenderTransform(room);
                float dx = t.pos.x() - curx;
                float dy = t.pos.y() - cury;
                float dz = t.pos.z() - curz;
                if (frustum.isPointInFrustum(dx, dy, dz) != true)
                    continue;

                GLfloat pickColor[4];
                encodeColor(room->id + 1, pickColor);
                float size = ROOM_SIZE * t.scale;
                QVector3D a(dx - size, dy - size, dz);
                QVector3D b(dx + size, dy - size, dz);
                QVector3D c(dx + size, dy + size, dz);
                QVector3D d(dx - size, dy + size, dz);
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
