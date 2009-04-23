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



/* configuration reader/saver and handler */
#include <QFile>
#include <QImage>
#include <QXmlDefaultHandler>
#include <QGLWidget>
#include <QMutex>

#include "defines.h"
#include "CRoom.h"
#include "CRoomManager.h"
#include "mainwindow.h"

#include "CConfigurator.h"
#include "utils.h"
#include "CEngine.h"
#include "CGroup.h"
//#include "renderer.h"

class Cconfigurator *conf;

Cconfigurator::Cconfigurator()
{
    /* here we set the default configuration */

    print_debug(DEBUG_CONFIG, "in configurator constructor");
    
    /* data */
    baseFile = "";
    localPort = 0;
    remoteHost = "";
    remotePort = 0;
    databaseModified = false;
    setConfigModified(false);
    
    userWindowRect.setRect(0, 0, 0, 0);  // means autodetect
    angleX = 0;
    angleY = 0;
    angleZ = 0;
    userX = 0;
    userY = 0;
    userZ = 0;

    visibleLayers = 5;

    setAutorefresh(true);          /* default values */ 
    setAutomerge(true);
    setAngrylinker(true);
    setExitsCheck(false);
    setTerrainCheck(true);
    setAlwaysOnTop(true);
    
    setDetailsVisibility(500);
    setTextureVisibility(300);
    
    setNameQuote(10);
    setDescQuote(10);
    
    setRegionsAutoReplace( false );
    setRegionsAutoSet( false );
    
    setShowRegionsInfo( true );
    setDisplayRegionsRenderer( true );
    setShowNotesRenderer( true );

    setSelectOAnyLayer( true );
    
    /* colours */
    
    /* patterns */
    setExitsPattern("Exits:");
    
    struct roomSectorsData first;
        
    first.pattern = 0;
    first.desc = "NONE";
    first.texture = 1;
    first.gllist = 1;
    sectors.push_back(first);
    
    spells_pattern = "Affected by:";
    
    // group Manager
    groupManagerState = CGroupCommunicator::Off;
    groupManagerLocalPort = 4243;
    groupManagerRemotePort = 4243;
    groupManagerHost = "localhost";
    groupManagerCharName = "Aza";
    showGroupManager = false;
    groupManagerRect.setRect(0, 0, 0, 0);  // means autodetect
    groupManagerColor = QColor("#F28003");

    setStartupMode(0);
    setNoteColor("#F28003");
}


void Cconfigurator::resetCurrentConfig()
{
    sectors.clear();
    spells.clear();
    
    struct roomSectorsData first;
        
    first.pattern = 0;
    first.desc = "NONE";
    first.texture = 1;
    first.gllist = 1;
    sectors.push_back(first);
}

/* ---------------- PATTERNS and REGEXPS GENERATION --------------- */
void Cconfigurator::setExitsPattern(QByteArray str) 
{ 
    exitsPattern = str; 
    exitsExpr.setPattern(QRegExp::escape(str) );
    
    setConfigModified(true);
}

/* --------------------------------------- spells ----------------------------------------- */
void Cconfigurator::addSpell(QByteArray spellname, QByteArray up, QByteArray down, QByteArray refresh, bool addon)
{
    TSpell spell;
    
    spell.name = spellname;
    spell.up_mes = up;
    spell.down_mes = down;
    spell.refresh_mes = refresh;
    spell.addon = addon;
    spell.up = false;
    
    spells.push_back(spell);
    setConfigModified(true);
}

void Cconfigurator::addSpell(TSpell spell)
{
    spells.push_back(spell);
    setConfigModified(true);
}

QString Cconfigurator::spellUpFor(unsigned int p)
{
    QString s;
    int min;
    int sec;
    
    if (p > spells.size())
        return "";
        
    sec = spells[p].timer.elapsed() / (1000);
    min = sec / 60;
    sec = sec % 60;    
        
    s = QString("- %1%2:%3%4")
            .arg( min / 10 )
            .arg( min % 10 )
            .arg( sec / 10 )
            .arg( sec % 10 );

    return s;
}


/* ----------------- REGULAR EXPRESSIONS SECTION ---------------- */
/* ------------------- DATA ------------------- */
char Cconfigurator::getPatternByRoom(CRoom *r)
{
    return sectors.at(r->getTerrain()).pattern;
}

int Cconfigurator::getSectorByDesc(QByteArray desc)
{
    unsigned int i;
    for (i = 0; i < sectors.size(); ++i) {
        if (sectors[i].desc == desc)
            return i;
    }
    return 0;
}

GLuint Cconfigurator::getTextureByDesc(QByteArray desc)
{
    int i;
    i = getSectorByDesc(desc);
    if (i == -1)
        return 0;
    return sectors[i].texture;
}


void Cconfigurator::addTexture(QByteArray desc, QByteArray filename, char pattern)
{
    struct roomSectorsData s;
        
    s.desc = desc;
    s.filename = filename;
    s.pattern = pattern;

    sectors.push_back(s);
//    printf("added texture with pattern %c.\r\n", pattern);
}

int Cconfigurator::getSectorByPattern(char pattern)
{
    unsigned int i;
    for (i = 0; i < sectors.size(); ++i) {
        if (sectors[i].pattern == pattern)
            return i;
    }
    return 0;
}


void Cconfigurator::setBaseFile(QByteArray str)
{
    baseFile = str;
    setConfigModified(true);
}

void Cconfigurator::setDisplayRegionsRenderer(bool b)
{
    displayRegionsRenderer = b;
    setConfigModified(true);
}

void Cconfigurator::setShowRegionsInfo(bool b)
{
    showRegionsInfo = b;
    setConfigModified(true);
}

bool Cconfigurator::getDisplayRegionsRenderer()
{
    return displayRegionsRenderer;
}

bool Cconfigurator::getShowRegionsInfo()
{
    return showRegionsInfo;
}

bool Cconfigurator::getRegionsAutoSet()
{
    return regionsAutoSet;
}

bool Cconfigurator::getRegionsAutoReplace()
{
    return regionsAutoReplace;
}

void Cconfigurator::setRegionsAutoSet(bool b)
{
    regionsAutoSet = b;
}

void Cconfigurator::setRegionsAutoReplace(bool b)
{
    regionsAutoReplace = b;
    setConfigModified(true);
}

void Cconfigurator::setShowNotesRenderer(bool b)
{
    showNotesRenderer = b;
    setConfigModified(true);
}



void Cconfigurator::setRemoteHost(QByteArray str)
{
    remoteHost = str;
    setConfigModified(true);
}

void Cconfigurator::setRemotePort(int i)
{
    remotePort = i;
    setConfigModified(true);
}

void Cconfigurator::setLocalPort(int i)
{
    localPort = i;
    setConfigModified(true);
}

void Cconfigurator::setAutorefresh(bool b)
{
    autorefresh = b;
    setConfigModified(true);
}

void Cconfigurator::setAutomerge(bool b)
{
    automerge = b;
    setConfigModified(true);
}

void Cconfigurator::setDuallinker(bool b)  
{ 
    duallinker = b; 
    setConfigModified(true); 
}

bool Cconfigurator::getDuallinker() 
{ 
    return duallinker; 
}


void Cconfigurator::setAngrylinker(bool b)
{
    angrylinker = b;
    setConfigModified(true);
}

void Cconfigurator::setExitsCheck(bool b)
{
    exitsCheck = b;
//    set_conf_mod(true);       /* this option changes repeatedly when you turn */
                                /* mapping on and off */
}

void Cconfigurator::setTerrainCheck(bool b)
{
    terrainCheck = b;
    setConfigModified(true);
}

void Cconfigurator::setDetailsVisibility(int i)
{
    detailsVisibilityRange = i;
    setConfigModified(true);
}

void Cconfigurator::setTextureVisibility(int i)
{
    textureVisibilityRange = i;
    setConfigModified(true);
}

void Cconfigurator::setBriefMode(bool b)
{
    briefMode = b;
    setConfigModified(true);
}

void Cconfigurator::setAlwaysOnTop(bool b)
{
    alwaysOnTop = b;
    setConfigModified(true);
}

void Cconfigurator::setNameQuote(int i)
{
    nameQuote = i;
    setConfigModified(true);
}

void Cconfigurator::setDescQuote(int i)
{
    descQuote = i;
    setConfigModified(true);
}

void Cconfigurator::setStartupMode(int i)
{
    startupMode = i;
    setConfigModified(true);
}

int Cconfigurator::getStartupMode()
{
    return startupMode;
}

// default color
void Cconfigurator::setNoteColor(QByteArray c)
{
    noteColor = c;
    setConfigModified(true);
}

QByteArray Cconfigurator::getNoteColor()
{
    return noteColor;
}


int Cconfigurator::loadTexture(struct roomSectorsData *p)
{
    QImage tex1, buf1;

    glGenTextures(1, &p->texture);
    print_debug(DEBUG_RENDERER, "loading texture %s", (const char *) p->filename);
    if (p->filename == "")
        return -1;
    if (!buf1.load( p->filename )) {
        print_debug(DEBUG_CONFIG, "Failed to load the %s!", (const char *) p->filename);
        return -1;
    }
    tex1 = QGLWidget::convertToGLFormat( buf1 );
    glGenTextures(1, &p->texture );
    glBindTexture(GL_TEXTURE_2D, p->texture );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tex1.width(), tex1.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, tex1.bits() );
    
    
    
    p->gllist = glGenLists(1);
    if (p->gllist != 0) {
        glNewList(p->gllist, GL_COMPILE);
        
        glEnable(GL_TEXTURE_2D);
        
        glBindTexture(GL_TEXTURE_2D, p->texture);
        
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
        
        
        
        glEndList();
    }
    return 1;
}


int Cconfigurator::loadConfig(QByteArray path, QByteArray filename)
{
  QFile xmlFile(path+filename);
  QXmlInputSource source( &xmlFile );

  if (xmlFile.exists() == false) {
      print_debug(DEBUG_CONFIG, "ERROR: The config file %s does NOT exist!", (const char*) (path+filename) );
      return 0;
  }

  QXmlSimpleReader reader;

  ConfigParser * handler = new ConfigParser();
  reader.setContentHandler( handler );
    
  resetCurrentConfig();
	
    
  print_debug(DEBUG_CONFIG, "Reading the config file %s", (const char *) (path+filename));
  fflush(stdout);
  reader.parse( source );
  print_debug(DEBUG_CONFIG, "done.");
  setConfigModified(false);


  configPath = path;
  configFile = filename;
  return 1;
}

int Cconfigurator::saveConfigAs(QByteArray path, QByteArray filename)
{
  FILE *f;
  unsigned int i;
  QRect window;

  configFile = filename;
  configPath = path;

  f = fopen((const char *) path + filename, "w");
  if (f == NULL) {
    print_debug(DEBUG_XML, "XML: Error - can not open the file: %s.", (const char *) filename);
    return -1;
  }    
  
  fprintf(f, "<config>\r\n");
  fprintf(f, "  <localport port=\"%i\">\r\n", getLocalPort());
  fprintf(f, "  <remotehost hostname=\"%s\" port=\"%i\">\r\n", 
                  (const char *) getRemoteHost(), 
                  getRemotePort() );
  fprintf(f, "  <basefile filename=\"%s\">\r\n", 
                  (const char *) getBaseFile() );
  
  fprintf(f, "  <GLvisibility textures=\"%i\" details=\"%i\" shownotes=\"%s\" layers=\"%i\">\r\n", 
                  getTextureVisibility(),  getDetailsVisibility(), ON_OFF(getShowNotesRenderer()),  
                  getVisibleLayers() );
  
  fprintf(f, "  <analyzers exits=\"%s\"  terrain=\"%s\">\r\n", 
                  ON_OFF(getExitsCheck() ), ON_OFF(getTerrainCheck() ) );

  fprintf(f, "  <engineflags briefmode=\"%s\" automerge=\"%s\"  angrylinker=\"%s\" duallinker=\"%s\">\r\n", 
                  ON_OFF(getBriefMode()), 
                  ON_OFF(getAutomerge() ), ON_OFF( getAngrylinker()), 
                  ON_OFF(getDuallinker() ) );
                  
              
  fprintf(f, "  <regionsflags displayinrenderer=\"%s\" showinfo=\"%s\">\r\n", 
                  ON_OFF(getDisplayRegionsRenderer()), 
                  ON_OFF(getShowRegionsInfo()) );
                  

  fprintf(f, "  <guisettings always_on_top=\"%s\">\r\n", 
                  ON_OFF(getAlwaysOnTop()) );

  QString grpManager;
  switch (getGroupManagerState()) {
	case CGroupCommunicator::Client :
		grpManager += "Client";
		break;
	case CGroupCommunicator::Server :
		grpManager += "Server";
		break;
	default:
		grpManager += "Off";
		break;
  }
  fprintf(f, "  <groupManager state=\"%s\" host=\"%s\" charName=\"%s\" charColor=\"%s\" localPort=\"%i\" remotePort=\"%i\">\r\n", 
                  (const char *) grpManager.toAscii(), (const char *) getGroupManagerHost(),
                  (const char *) getGroupManagerCharName(), 
                  (const char *) getGroupManagerColor().name().toAscii(), 
                  getGroupManagerLocalPort(), getGroupManagerRemotePort() );

  window = renderer_window->getGroupManagerRect(); 
  fprintf(f, "  <groupManagerGUI show=\"%s\"  x=\"%i\" y=\"%i\" height=\"%i\" width=\"%i\">\r\n", 
		  ON_OFF( getShowGroupManager() ), window.x(), window.y(), window.height(), window.width() );
                        
  window = renderer_window->geometry(); 
  fprintf(f, "  <window x=\"%i\" y=\"%i\" height=\"%i\" width=\"%i\">\r\n",
            window.x(), window.y(), window.height(), window.width() );                 


  angleX = renderer_window->renderer->angleX;
  angleY = renderer_window->renderer->angleY;
  angleZ = renderer_window->renderer->angleZ;
  fprintf(f, "  <rendererangles anglex=\"%f\" angley=\"%f\" anglez=\"%f\">\r\n", 
                angleX, angleY, angleZ); 

  userX = renderer_window->renderer->userX;
  userY = renderer_window->renderer->userY;
  userZ = renderer_window->renderer->userZ;
  fprintf(f, "  <rendererpositions userx=\"%f\" usery=\"%f\" userz=\"%f\">\r\n",
        userX, userY, userZ);


  fprintf(f, "  <refresh auto=\"%s\" roomnamequote=\"%i\" descquote=\"%i\">\r\n",
                  ON_OFF( getAutorefresh() ), getNameQuote(), getDescQuote() );
  
  QString ch;
  for (i = 1; i < sectors.size(); i++) {
      if (sectors[i].pattern == '<')
          ch = "&lt;";
      else if (sectors[i].pattern == '>')
          ch = "&gt;";
      else if (sectors[i].pattern == '&')
          ch = "&amp;";
      else if (sectors[i].pattern == '\'')
          ch = "&apos;";
      else if (sectors[i].pattern == '"')
          ch = "&quot";
      else
          ch = sectors[i].pattern;
      
      fprintf(f, "  <texture handle=\"%s\" file=\"%s\" pattern=\"%s\">\r\n",
                  (const char *) sectors[i].desc, 
                  (const char *) sectors[i].filename, qPrintable(ch));
  
  }
  
  for (i = 0; i < spells.size(); i++) {
        fprintf(f, "  <spell addon=\"%s\" name=\"%s\" up=\"%s\" refresh=\"%s\" down=\"%s\">\r\n",
                    YES_NO(spells[i].addon), 
                    (const char *) spells[i].name,
                    (const char *) spells[i].up_mes,
                    (const char *) spells[i].refresh_mes,
                    (const char *) spells[i].down_mes);
  }

  
  i = 0;
  while (debug_data[i].name) {
      fprintf(f, "  <debug name=\"%s\"  state=\"%s\">\r\n", debug_data[i].name, ON_OFF(debug_data[i].state));
      i++;
  }

  fprintf(f, "  <misc startupmode=\"%d\" notecolor=\"%s\">\r\n", 
          getStartupMode(), (const char*)getNoteColor());

  /* PUT ENGINE CONFIG SAVING THERE ! */
  
  setConfigModified(false);
  fprintf(f, "</config>\r\n");
  fflush(f);
  fclose(f);
  return 1;
}


/* --------------- HERE COMES THE XML READER FOR CONFIG FILES -------------- */


ConfigParser::ConfigParser()
  : QXmlDefaultHandler()
{
}


/*bool ConfigParser::endElement( const QString& , const QString& , const QString&)
{
    return TRUE;
}
*/
/*bool ConfigParser::characters( const QString& ch)
{
  return TRUE;
} 
*/

bool ConfigParser::startElement( const QString& , const QString& , 
                                    const QString& qName, 
                                    const QXmlAttributes& attributes)
{
    if (qName == "localport") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(localport token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("port");
        conf->setLocalPort(s.toInt() );
//        printf("Using local port %i. \r\n", conf->get_local_port() );

        return TRUE;
    } else if (qName == "remotehost") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(remotehost token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("hostname");
        conf->setRemoteHost(s.toAscii() );
        
        s = attributes.value("port");
        conf->setRemotePort(s.toInt() );
//        printf("Using remote host %s:%i\r\n", (const char *)conf->get_remote_host(), 
//                                            conf->get_remote_port() );

        return TRUE;
    } else if (qName == "basefile") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(basefile token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("filename");
        conf->setBaseFile(s.toAscii() );
//        printf("Using the database file: %s\r\n", qPrintable(s) );
        
        return TRUE;
    } else if (qName == "GLvisibility") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(GLvisibility token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("textures");
        conf->setTextureVisibility(s.toInt() );
        s = attributes.value("details");
        conf->setDetailsVisibility(s.toInt() );
        
        s = attributes.value("shownotes");
        s = s.toLower();
        if (s == "on") 
            conf->setShowNotesRenderer( true);
        else 
            conf->setShowNotesRenderer( false);

        s = attributes.value("layers");
        conf->setVisibleLayers(s.toInt() );
        
        
        print_debug(DEBUG_CONFIG, "OpenGL visibility ranges set to %i (texture) and %i (details).",
                    conf->getTextureVisibility(), conf->getDetailsVisibility() );
        
        return TRUE;
    } else if (qName == "analyzers") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(analyzers token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("exits");
        s = s.toLower();
        if (s == "on") 
            conf->setExitsCheck(true);
        else 
            conf->setExitsCheck(false);
        
        s = attributes.value("terrain");
        s = s.toLower();
        if (s == "on") 
            conf->setTerrainCheck(true);
        else 
            conf->setTerrainCheck(false);
        
        
        print_debug(DEBUG_CONFIG, "Analyzers: desc ON, exits %s, terrain %s.",
                    ON_OFF(conf->getExitsCheck() ), ON_OFF(conf->getTerrainCheck()) );
        
        return TRUE;
    } else if (qName == "groupManager") {
    	s = attributes.value("state");
    	if (s == "Server") {
    		conf->setGroupManagerState(CGroupCommunicator::Server);
    	} else if (s == "Client") {
    		conf->setGroupManagerState(CGroupCommunicator::Client);
    	} else 
    		conf->setGroupManagerState(CGroupCommunicator::Off);
        
    	s = attributes.value("host");
        conf->setGroupManagerHost(s.toAscii());

        
    	s = attributes.value("charColor");
        conf->setGroupManagerColor(QColor(s));
        
    	s = attributes.value("charName");
        conf->setGroupManagerCharName(s.toAscii());

        s = attributes.value("localPort");
        conf->setGroupManagerLocalPort(s.toInt() );

        s = attributes.value("remotePort");
        conf->setGroupManagerRemotePort(s.toInt() );


    } else if (qName == "groupManagerGUI") {

   		// BURP
        s = attributes.value("show");
        s = s.toLower();
        if (s == "on") 
            conf->setShowGroupManager(true);
        else 
            conf->setShowGroupManager(false);
        
        int x = 0;
        int y = 0;
        int height = 0;
        int width = 0;
        x = attributes.value("x").toInt();
        y = attributes.value("y").toInt();
        height = attributes.value("height").toInt();
        width = attributes.value("width").toInt();

        conf->setGroupManagerRect(  QRect(x, y, width, height) );
        print_debug(DEBUG_CONFIG, "Loaded groupManager window settins: x %i, y %i, height %i, width %i",
                        x, y, height, width);

    } else if (qName == "guisettings") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(guisettings token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("always_on_top");
        s = s.toLower();
        if (s == "on") 
            conf->setAlwaysOnTop(true);
        else 
            conf->setAlwaysOnTop(false);

        print_debug(DEBUG_CONFIG, "GUI settings: always_on_top %s.", ON_OFF(conf->getAlwaysOnTop()) );

        return TRUE;
    } else if (qName == "regionsflags") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(guisettings token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("displayinrenderer");
        s = s.toLower();
        if (s == "on") 
            conf->setDisplayRegionsRenderer(true);
        else 
            conf->setDisplayRegionsRenderer(false);
            
        s = attributes.value("showinfo");
        s = s.toLower();
        if (s == "on") 
            conf->setShowRegionsInfo(true);
        else 
            conf->setShowRegionsInfo(false);
            
            

        print_debug(DEBUG_CONFIG, "GUI settings: always_on_top %s.", ON_OFF(conf->getAlwaysOnTop()) );

        return TRUE;
    } else if (qName == "engineflags") {
        if (attributes.length() < 3) {
            print_debug(DEBUG_CONFIG, "(engineflags token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("briefmode");
        s = s.toLower();
//        printf("The brief mode setting : %s\r\n", qPrintable(s) );
        if (s == "on") 
            conf->setBriefMode(true);
        else 
            conf->setBriefMode(false);
        
        s = attributes.value("automerge");
        s = s.toLower();
        if (s == "on") 
            conf->setAutomerge(true);
        else 
            conf->setAutomerge(false);

        s = attributes.value("angrylinker");
        s = s.toLower();
        if (s == "on") 
            conf->setAngrylinker(true);
        else 
            conf->setAngrylinker(false);

        s = attributes.value("duallinker");
        s = s.toLower();
        if (s == "on") 
            conf->setDuallinker(true);
        else 
            conf->setDuallinker(false);



        print_debug(DEBUG_CONFIG, "Engine flags: briefmode %s, automerge %s, angrylinker %s.",
               ON_OFF(conf->getBriefMode()), ON_OFF(conf->getAutomerge()), 
                ON_OFF(conf->getAngrylinker()), ON_OFF(conf->getDuallinker()) );
        
        return TRUE;
    } else if (qName == "refresh") {
        if (attributes.length() < 3) {
            print_debug(DEBUG_CONFIG, "(refresh token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("auto");
        s = s.toLower();
        if (s == "on") 
            conf->setAutorefresh(true);
        else 
            conf->setAutorefresh(false);
        
        s = attributes.value("roomnamequote");
        conf->setNameQuote(s.toInt());
        s = attributes.value("descquote");
        conf->setDescQuote(s.toInt());

        print_debug(DEBUG_CONFIG, "Autorefresh settings: automatic refresh %s, roomname quote %i, desc quote %i.",
                ON_OFF(conf->getAutorefresh()), conf->getNameQuote(), 
                conf->getDescQuote() );
        return TRUE;
    } else if (qName == "texture") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(texture token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("handle");
        s.toUpper();
        texture.desc = s.toAscii();
        
        s = attributes.value("file");
        texture.filename = s.toAscii();

        s = attributes.value("pattern");
        texture.pattern = s[0].toAscii();

        conf->addTexture(texture.desc, texture.filename, texture.pattern);
//        printf("Added texture: desc %s, file %s, pattern %c.\r\n", 
//              (const char *) texture.desc, (const char *) texture.filename, texture.pattern);

        
//        flag = TEXTURE;         /* get the inner data ! */
        return TRUE;
    } else if (qName == "spell") {
        if (attributes.length() < 4) {
            print_debug(DEBUG_CONFIG, "(pattern token) Not enough attributes in XML file!");
            exit(1);
        }        

        spell.up = false;
        s = attributes.value("addon");
        s = s.toLower();
        spell.addon = false;
        if (s == "yes") 
          spell.addon = true;
        
        
        s = attributes.value("name");
        spell.name = s.toAscii();
        
        s = attributes.value("up");
        spell.up_mes = s.toAscii();

        s = attributes.value("refresh");
        spell.refresh_mes = s.toAscii();

        s = attributes.value("down");
        spell.down_mes = s.toAscii();

        conf->addSpell(spell);
        return TRUE;
    } else if (qName == "debug") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(texture token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("name");
        unsigned int i = 0;
        while (debug_data[i].name != NULL) {
            if (debug_data[i].name == s) 
                break;
            i++;
        }
        if (debug_data[i].name == NULL) {
            print_debug(DEBUG_CONFIG, "Warning, %s is a wrong debug descriptor/name!", qPrintable(s));
            return TRUE;
        }
        
        s = attributes.value("state");
        s = s.toLower();
        if (s == "on") 
            debug_data[i].state = 1;
        else 
            debug_data[i].state = 0;
        
//        printf("Debug option %s is now %s.\r\n", debug_data[i].name, ON_OFF(debug_data[i].state) );
        return TRUE;
    } else if (qName == "window") {
        int x = 0;
        int y = 0;
        int height = 0;
        int width = 0;

        x = attributes.value("x").toInt();
        y = attributes.value("y").toInt();
        height = attributes.value("height").toInt();
        width = attributes.value("width").toInt();

        conf->setWindowRect(x, y, width, height);
        print_debug(DEBUG_CONFIG, "Loaded window settins: x %i, y %i, height %i, width %i",
                        x, y, height, width);
                        
        return TRUE;
    } else if (qName == "rendererangles") {
        float x = 0;
        float y = 0;
        float z = 0;

        x = attributes.value("anglex").toFloat();
        y = attributes.value("angley").toFloat();
        z = attributes.value("anglez").toFloat();
        
        conf->setRendererAngles(x,y,z);
            
        print_debug(DEBUG_CONFIG, "Loaded renderer angles : x %f, y %f, z %f",x, y, z);
        return TRUE;
    } else if (qName == "rendererpositions") {
        float x = 0;
        float y = 0;
        float z = 0;

        x = attributes.value("userx").toFloat();
        y = attributes.value("usery").toFloat();
        z = attributes.value("userz").toFloat();
        
        conf->setRendererPosition(x,y,z);
            
        print_debug(DEBUG_CONFIG, "Loaded renderer shift : x %f, y %f, z %f",x, y, z);
        return TRUE;
    
    } else if (qName == "misc") {
        int startup_mode = 0;
        startup_mode = attributes.value("startupmode").toInt();
        conf->setStartupMode(startup_mode);
        print_debug(DEBUG_CONFIG, "Set startup mode: %d", startup_mode);
        
        QByteArray note_color = (QByteArray)attributes.value("notecolor").toAscii();
        conf->setNoteColor(note_color);
        print_debug(DEBUG_CONFIG, "Set note color: %d", (const char*)note_color);

        return TRUE;
    }

  return TRUE;
}

