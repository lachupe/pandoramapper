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

#ifndef CONFIGURATOR_H
#define CONFIGURATOR_H
/* configurator */

#include <QXmlDefaultHandler>
#include <QGLWidget>
#include <QTime>

#include <vector>
#include "CRoom.h"

using namespace std;

struct roomSectorsData {
  QByteArray desc;             /* name of this flag */
  QByteArray filename;         /* appropriate texture's filename */
  char   pattern;           /* appropriate pattern */
  GLuint texture;          /* and texture handler for renderer */
  GLuint gllist;            /* OpenGL display list */
};


typedef struct {
    QByteArray  name;           /* spells name */
    QByteArray  up_mes;         /* up message/pattern */
    QByteArray  down_mes;       /* down message */
    QByteArray  refresh_mes;    /* refresh message */
    QTime       timer;          /* timer */
    bool        addon;          /* if this spell has to be added after the "Affected by:" line */
    bool        up;             /* is this spell currently up ? */
    bool		silently_up;	/* this spell is up, but time wasn't set for ome reason (reconnect) */
								/* this option is required for better GroupManager functioning */
} TSpell;

class Cconfigurator : public QObject {
    Q_OBJECT

    /* general */
    bool        configModified;       /* if the config was modified */
    QByteArray  configFile;
    QByteArray  configPath;

    /* patterns/regexps */
    QByteArray exitsPattern;
    QRegExp     exitsExpr;

    /* data */
    QByteArray  databasePath;
    QByteArray  baseFile;
    int         localPort;
    QByteArray  remoteHost;
    int         remotePort;
    bool        databaseModified;


    bool autorefresh;             /* automatic room desc refresh */
    bool automerge;               /* automatic twins merging based on roomdesc */
    bool angrylinker;             /* automatic linking based on coordinates */
    bool duallinker;              /* auto-link to the room you came from */

    bool exitsCheck;             /* apply exits check to stacks */
    bool terrainCheck;           /* apply terrain check to stacks */
    bool briefMode;
    bool alwaysOnTop;           /* keep Pandora window on top of others */

    bool regionsAutoSet;
    bool regionsAutoReplace;
    bool displayRegionsRenderer;
    bool showRegionsInfo;
    bool showNotesRenderer;
    bool multisampling;

    int startupMode; /* 0 for select, 1 for move */
    QByteArray noteColor;

    int textureVisibilityRange;
    int detailsVisibilityRange;


    int descQuote;        /* quote for description - in percents */
    int nameQuote;        /* quote for roomname - in percents */

//    void parse_line(char *line);

    void resetCurrentConfig();

    // user window size/posiiton
    QRect userWindowRect;

    // renderer user defined angle's and shift
    float angleX, angleY, angleZ;
    float userX, userY, userZ;

    int visibleLayers;

    bool selectOAnyLayer;

    // groupManager settings
    int  groupManagerState;
    int	 groupManagerLocalPort;
    int	 groupManagerRemotePort;
    QByteArray groupManagerHost;
    QByteArray groupManagerCharName;
    bool groupManagerShowManager;
    bool groupManagerShowSelf;
    QRect groupManagerRect;
    QColor groupManagerColor;
    bool 	groupManagerNotifyArmour;
    bool 	groupManagerNotifySanc;

    QByteArray scorePattern;
    QByteArray scorePatternShort;

public:

    /* movement patterns */
    QList<QByteArray> moveForcePatterns;
    QList<QByteArray> moveCancelPatterns;


    /* spells */
    std::vector<TSpell>  spells;
    QByteArray      spells_pattern;
    void addSpell(QByteArray spellname, QByteArray up, QByteArray down, QByteArray refresh, bool addon);
    void addSpell(TSpell s);
    QString spellUpFor(unsigned int p);
    void resetSpells();



    // group Manager
    int getGroupManagerState() { return groupManagerState; }
    int getGroupManagerLocalPort() { return groupManagerLocalPort; }
    int getGroupManagerRemotePort() { return groupManagerRemotePort; }
    QByteArray getGroupManagerHost() { return groupManagerHost; }
    QByteArray getGroupManagerCharName() { return groupManagerCharName; }
    bool getGroupManagerShowManager() { return groupManagerShowManager; }
    bool getGroupManagerShowSelf() { return groupManagerShowSelf; }
    QRect getGroupManagerRect() { return groupManagerRect; }
    QColor getGroupManagerColor() { return groupManagerColor; }
    bool getGroupManagerNotifySanc() { return groupManagerNotifySanc; }
    bool getGroupManagerNotifyArmour() { return groupManagerNotifyArmour; }


    void setGroupManagerState(int val) { groupManagerState = val; setConfigModified(true); }
    void setGroupManagerLocalPort(int val) { groupManagerLocalPort = val; setConfigModified(true); }
    void setGroupManagerRemotePort(int val) { groupManagerRemotePort = val; setConfigModified(true); }
    void setGroupManagerHost(QByteArray val) { groupManagerHost = val; setConfigModified(true); }
    void setGroupManagerCharName(QByteArray val) { groupManagerCharName = val; setConfigModified(true); }
    void setGroupManagerShowManager(bool b) { groupManagerShowManager = b; }
    void setGroupManagerShowSelf(bool b) { groupManagerShowSelf = b; setConfigModified(true); }
    void setGroupManagerRect(QRect r) { groupManagerRect = r; setConfigModified(true); }
    void setGroupManagerColor(QColor c) { groupManagerColor = c; setConfigModified(true); }
    void setGroupManagerNotifySanc(bool b) { groupManagerNotifySanc = b; setConfigModified(true); }
    void setGroupManagerNotifyArmour(bool b) { groupManagerNotifyArmour = b; setConfigModified(true); }

    /* texture and sectors stuff */
    vector<struct roomSectorsData> sectors;
    int getSectorByDesc(QByteArray desc);
    int getSectorByPattern(char pattern);

    int loadTexture(struct roomSectorsData *p);
    char getPatternByRoom(CRoom *r);
    GLuint getTextureByDesc(QByteArray desc);
    void addTexture(QByteArray desc, QByteArray filename, char pattern);

    /* */

    int loadEngineConfig(QByteArray path, QByteArray filename);

    Cconfigurator();


    int loadConfig(QByteArray path, QByteArray filename);
    int saveConfigAs(QByteArray path, QByteArray filename);
    int saveConfig() { return saveConfigAs(configPath, configFile); }


    void setEndCol(QByteArray str);
    /* patterns */
    QByteArray getExitsPattern() { return exitsPattern; }
    void setExitsPattern(QByteArray str);
    QByteArray getScorePattern() { return scorePattern; }
    QByteArray getShortScorePattern() { return scorePatternShort; }
    void setScorePattern(QByteArray str) { scorePattern = str; setConfigModified(true); }
    void setShortScorePattern(QByteArray str) { scorePatternShort = str; setConfigModified(true); }

    QRegExp getExitsExpr() { return exitsExpr; }

    /* data / global flags */
    void setBaseFile(QByteArray str);
    void setDatabaseModified(bool b) { databaseModified = b; }
    void setRemoteHost(QByteArray str);
    void setRemotePort(int i);
    void setLocalPort(int i);
    void setConfigModified(bool b) { configModified = b; emit configurationChanged(); }

    void setConfigFile(QByteArray p, QByteArray f) { configFile = f; configPath = p; }

    void setAutorefresh(bool b);
    void setAutomerge(bool b);
    void setAngrylinker(bool b);
    void setExitsCheck(bool b);
    void setTerrainCheck(bool b);
    void setDetailsVisibility(int i);
    void setTextureVisibility(int i);
    void setBriefMode(bool b);
    void setAlwaysOnTop(bool b);
    void setDescQuote(int i);
    void setNameQuote(int i);

    void setRegionsAutoSet(bool b);
    void setRegionsAutoReplace(bool b);
    void setDisplayRegionsRenderer(bool b);
    void setShowRegionsInfo(bool b);
    void setMultisampling(bool b) { multisampling = b; setConfigModified(true); }


    void setStartupMode(int i);
    int getStartupMode();
    void setNoteColor(QByteArray c);
    QByteArray getNoteColor();

    void setShowNotesRenderer(bool b);
    /*--*/
    bool isDatabaseModified() { return databaseModified; }
    QByteArray getBaseFile() { return baseFile; }
    QByteArray getRemoteHost() { return remoteHost;}
    int getRemotePort() {return remotePort;}
    int getLocalPort() {return localPort;}
    bool isConfigModified() { return configModified; }

    bool getAutorefresh() { return autorefresh; }
    bool getAutomerge()  { return automerge; }
    bool getAngrylinker() { return angrylinker; }
    bool getExitsCheck() { return exitsCheck; }
    bool getTerrainCheck() { return terrainCheck; }
    bool getBriefMode()     {return briefMode;}
    bool getAlwaysOnTop()     {return alwaysOnTop;}

    int getDetailsVisibility() { return detailsVisibilityRange; }
    int getTextureVisibility() { return textureVisibilityRange; }

    bool getRegionsAutoSet();
    bool getRegionsAutoReplace();
    bool getDisplayRegionsRenderer();
    bool getShowRegionsInfo();
    bool getMultisampling() { return multisampling; }

    bool getShowNotesRenderer() { return showNotesRenderer; }

    // App Window size
    void setWindowRect(QRect rect) { userWindowRect = rect; setConfigModified(true);  }
    void setWindowRect(int x, int y, int width, int height)
        { userWindowRect.setRect(x, y, width, height); setConfigModified(true);  }
    QRect getWindowRect() {return userWindowRect; }

    // renderer settings
    void setRendererAngles(float x, float y, float z)
       { angleX = x; angleY = y; angleZ = z; setConfigModified(true);  }
    void setRendererPosition(float x, float y, float z)
       { userX = x; userY = y; userZ = z; setConfigModified(true);  }
    float getRendererAngleX() {return angleX;}   // this is ONLY what was read from CONFIG
    float getRendererAngleY() {return angleY;}
    float getRendererAngleZ() {return angleZ;}

    float getRendererPositionX() {return userX; }
    float getRendererPositionY() {return userY; }
    float getRendererPositionZ() {return userZ; }

    void setDuallinker(bool b);
    bool getDuallinker();

    void setSelectOAnyLayer(bool b) { selectOAnyLayer=b; setConfigModified(true); }
    bool getSelectOAnyLayer() { return selectOAnyLayer; }

    void setVisibleLayers(int i) { visibleLayers = i; setConfigModified(true); }
    int getVisibleLayers() { return visibleLayers; }


    int getDescQuote() { return descQuote; }
    int getNameQuote() { return nameQuote; }

signals:
    void configurationChanged();
};

extern class Cconfigurator *conf;

#endif
