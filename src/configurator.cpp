

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
    base_file = "";
    local_port = 0;
    remote_host = "";
    remote_port = 0;
    db_modified = false;
    set_conf_mod(false);
    
    userWindowRect.setRect(0, 0, 0, 0);  // means autodetect
    anglex = 0;
    angley = 0;
    anglez = 0;
    userx = 0;
    usery = 0;
    userz = 0;

    visibleLayers = 5;

    set_autorefresh(true);          /* default values */ 
    set_automerge(true);
    set_angrylinker(true);
    set_exits_check(false);
    set_terrain_check(true);
    set_always_on_top(true);
    
    set_details_vis(500);
    set_texture_vis(300);
    
    set_name_quote(10);
    set_desc_quote(10);
    
    set_regions_auto_replace( false );
    set_regions_auto_set( false );
    
    set_show_regions_info( true );
    set_display_regions_renderer( true );
    set_show_notes_renderer( true );

    setSelectOAnyLayer( true );
    
    /* colours */
    
    /* patterns */
    set_exits_pat("Exits:");
    
    struct room_sectors_data first;
        
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

    set_startup_mode(0);
    set_note_color("#F28003");
}


void Cconfigurator::reset_current_config()
{
    sectors.clear();
    spells.clear();
    
    struct room_sectors_data first;
        
    first.pattern = 0;
    first.desc = "NONE";
    first.texture = 1;
    first.gllist = 1;
    sectors.push_back(first);
}

/* ---------------- PATTERNS and REGEXPS GENERATION --------------- */
void Cconfigurator::set_exits_pat(QByteArray str) 
{ 
    exits_pat = str; 
    exits_exp.setPattern(QRegExp::escape(str) );
    
    set_conf_mod(true);
}

/* --------------------------------------- spells ----------------------------------------- */
void Cconfigurator::add_spell(QByteArray spellname, QByteArray up, QByteArray down, QByteArray refresh, bool addon)
{
    TSpell spell;
    
    spell.name = spellname;
    spell.up_mes = up;
    spell.down_mes = down;
    spell.refresh_mes = refresh;
    spell.addon = addon;
    spell.up = false;
    
    spells.push_back(spell);
    set_conf_mod(true);
}

void Cconfigurator::add_spell(TSpell spell)
{
    spells.push_back(spell);
    set_conf_mod(true);
}

QString Cconfigurator::spell_up_for(unsigned int p)
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
char Cconfigurator::get_pattern_by_room(CRoom *r)
{
    return sectors.at(r->getTerrain()).pattern;
}

int Cconfigurator::get_sector_by_desc(QByteArray desc)
{
    unsigned int i;
    for (i = 0; i < sectors.size(); ++i) {
        if (sectors[i].desc == desc)
            return i;
    }
    return 0;
}

GLuint Cconfigurator::get_texture_by_desc(QByteArray desc)
{
    int i;
    i = get_sector_by_desc(desc);
    if (i == -1)
        return 0;
    return sectors[i].texture;
}


void Cconfigurator::add_texture(QByteArray desc, QByteArray filename, char pattern)
{
    struct room_sectors_data s;
        
    s.desc = desc;
    s.filename = filename;
    s.pattern = pattern;

    sectors.push_back(s);
//    printf("added texture with pattern %c.\r\n", pattern);
}

int Cconfigurator::get_sector_by_pattern(char pattern)
{
    unsigned int i;
    for (i = 0; i < sectors.size(); ++i) {
        if (sectors[i].pattern == pattern)
            return i;
    }
    return 0;
}


void Cconfigurator::set_base_file(QByteArray str)
{
    base_file = str;
    set_conf_mod(true);
}

void Cconfigurator::set_display_regions_renderer(bool b)
{
    display_regions_renderer = b;
    set_conf_mod(true);
}

void Cconfigurator::set_show_regions_info(bool b)
{
    show_regions_info = b;
    set_conf_mod(true);
}

bool Cconfigurator::get_display_regions_renderer()
{
    return display_regions_renderer;
}

bool Cconfigurator::get_show_regions_info()
{
    return show_regions_info;
}

bool Cconfigurator::get_regions_auto_set()
{
    return regions_auto_set;
}

bool Cconfigurator::get_regions_auto_replace()
{
    return regions_auto_replace;
}

void Cconfigurator::set_regions_auto_set(bool b)
{
    regions_auto_set = b;
}

void Cconfigurator::set_regions_auto_replace(bool b)
{
    regions_auto_replace = b;
    set_conf_mod(true);
}

void Cconfigurator::set_show_notes_renderer(bool b)
{
    show_notes_renderer = b;
    set_conf_mod(true);
}



void Cconfigurator::set_remote_host(QByteArray str)
{
    remote_host = str;
    set_conf_mod(true);
}

void Cconfigurator::set_remote_port(int i)
{
    remote_port = i;
    set_conf_mod(true);
}

void Cconfigurator::set_local_port(int i)
{
    local_port = i;
    set_conf_mod(true);
}

void Cconfigurator::set_autorefresh(bool b)
{
    autorefresh = b;
    set_conf_mod(true);
}

void Cconfigurator::set_automerge(bool b)
{
    automerge = b;
    set_conf_mod(true);
}

void Cconfigurator::set_duallinker(bool b)  
{ 
    duallinker = b; 
    set_conf_mod(true); 
}

bool Cconfigurator::get_duallinker() 
{ 
    return duallinker; 
}


void Cconfigurator::set_angrylinker(bool b)
{
    angrylinker = b;
    set_conf_mod(true);
}

void Cconfigurator::set_exits_check(bool b)
{
    exits_check = b;
//    set_conf_mod(true);       /* this option changes repeatedly when you turn */
                                /* mapping on and off */
}

void Cconfigurator::set_terrain_check(bool b)
{
    terrain_check = b;
    set_conf_mod(true);
}

void Cconfigurator::set_details_vis(int i)
{
    details_visibility_range = i;
    set_conf_mod(true);
}

void Cconfigurator::set_texture_vis(int i)
{
    texture_visibilit_range = i;
    set_conf_mod(true);
}

void Cconfigurator::set_brief_mode(bool b)
{
    brief_mode = b;
    set_conf_mod(true);
}

void Cconfigurator::set_always_on_top(bool b)
{
    always_on_top = b;
    set_conf_mod(true);
}

void Cconfigurator::set_name_quote(int i)
{
    name_quote = i;
    set_conf_mod(true);
}

void Cconfigurator::set_desc_quote(int i)
{
    desc_quote = i;
    set_conf_mod(true);
}

void Cconfigurator::set_startup_mode(int i)
{
    startup_mode = i;
    set_conf_mod(true);
}

int Cconfigurator::get_startup_mode()
{
    return startup_mode;
}

// default color
void Cconfigurator::set_note_color(QByteArray c)
{
    note_color = c;
    set_conf_mod(true);
}

QByteArray Cconfigurator::get_note_color()
{
    return note_color;
}


int Cconfigurator::load_texture(struct room_sectors_data *p)
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


int Cconfigurator::load_config(QByteArray path, QByteArray filename)
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
    
  reset_current_config();
	
    
  print_debug(DEBUG_CONFIG, "Reading the config file %s", (const char *) (path+filename));
  fflush(stdout);
  reader.parse( source );
  print_debug(DEBUG_CONFIG, "done.");
  set_conf_mod(false);


  config_path = path;
  config_file = filename;
  return 1;
}

int Cconfigurator::save_config_as(QByteArray path, QByteArray filename)
{
  FILE *f;
  unsigned int i;

  config_file = filename;
  config_path = path;

  f = fopen((const char *) path + filename, "w");
  if (f == NULL) {
    print_debug(DEBUG_XML, "XML: Error - can not open the file: %s.", (const char *) filename);
    return -1;
  }    
  
  fprintf(f, "<config>\r\n");
  fprintf(f, "  <localport port=\"%i\">\r\n", get_local_port());
  fprintf(f, "  <remotehost hostname=\"%s\" port=\"%i\">\r\n", 
                  (const char *) get_remote_host(), 
                  get_remote_port() );
  fprintf(f, "  <basefile filename=\"%s\">\r\n", 
                  (const char *) get_base_file() );
  
  fprintf(f, "  <GLvisibility textures=\"%i\" details=\"%i\" shownotes=\"%s\" layers=\"%i\">\r\n", 
                  get_texture_vis(),  get_details_vis(), ON_OFF(get_show_notes_renderer()),  
                  getVisibleLayers() );
  
  fprintf(f, "  <analyzers exits=\"%s\"  terrain=\"%s\">\r\n", 
                  ON_OFF(get_exits_check() ), ON_OFF(get_terrain_check() ) );

  fprintf(f, "  <engineflags briefmode=\"%s\" automerge=\"%s\"  angrylinker=\"%s\" duallinker=\"%s\">\r\n", 
                  ON_OFF(get_brief_mode()), 
                  ON_OFF(get_automerge() ), ON_OFF( get_angrylinker()), 
                  ON_OFF(get_duallinker() ) );
                  
              
  fprintf(f, "  <regionsflags displayinrenderer=\"%s\" showinfo=\"%s\">\r\n", 
                  ON_OFF(get_display_regions_renderer()), 
                  ON_OFF(get_show_regions_info()) );
                  

  fprintf(f, "  <guisettings always_on_top=\"%s\">\r\n", 
                  ON_OFF(get_always_on_top()) );

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
  fprintf(f, "  <groupManager state=\"%s\" host=\"%s\" charName=\"%s\" localPort=\"%i\" remotePort=\"%i\">\r\n", 
                  (const char *) grpManager.toAscii(), (const char *) getGroupManagerHost(),
                  (const char *) getGroupManagerCharName(), 
                  getGroupManagerLocalPort(), getGroupManagerRemotePort() );

  fprintf(f, "  <groupManagerGUI show=\"%s\">\r\n", ON_OFF( getShowGroupManager() ) );

  
  QRect window = renderer_window->geometry(); 
  fprintf(f, "  <window x=\"%i\" y=\"%i\" height=\"%i\" width=\"%i\">\r\n",
            window.x(), window.y(), window.height(), window.width() );                 


  anglex = renderer_window->renderer->anglex;
  angley = renderer_window->renderer->angley;
  anglez = renderer_window->renderer->anglez;
  fprintf(f, "  <rendererangles anglex=\"%f\" angley=\"%f\" anglez=\"%f\">\r\n", 
                anglex, angley, anglez); 

  userx = renderer_window->renderer->userx;
  usery = renderer_window->renderer->usery;
  userz = renderer_window->renderer->userz;
  fprintf(f, "  <rendererpositions userx=\"%f\" usery=\"%f\" userz=\"%f\">\r\n",
        userx, usery, userz);


  fprintf(f, "  <refresh auto=\"%s\" roomnamequote=\"%i\" descquote=\"%i\">\r\n",
                  ON_OFF( get_autorefresh() ), get_name_quote(), get_desc_quote() );
  
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
          get_startup_mode(), (const char*)get_note_color());

  /* PUT ENGINE CONFIG SAVING THERE ! */
  
  set_conf_mod(false);
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
        conf->set_local_port(s.toInt() );
//        printf("Using local port %i. \r\n", conf->get_local_port() );

        return TRUE;
    } else if (qName == "remotehost") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(remotehost token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("hostname");
        conf->set_remote_host(s.toAscii() );
        
        s = attributes.value("port");
        conf->set_remote_port(s.toInt() );
//        printf("Using remote host %s:%i\r\n", (const char *)conf->get_remote_host(), 
//                                            conf->get_remote_port() );

        return TRUE;
    } else if (qName == "basefile") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(basefile token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("filename");
        conf->set_base_file(s.toAscii() );
//        printf("Using the database file: %s\r\n", qPrintable(s) );
        
        return TRUE;
    } else if (qName == "GLvisibility") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(GLvisibility token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("textures");
        conf->set_texture_vis(s.toInt() );
        s = attributes.value("details");
        conf->set_details_vis(s.toInt() );
        
        s = attributes.value("shownotes");
        s = s.toLower();
        if (s == "on") 
            conf->set_show_notes_renderer( true);
        else 
            conf->set_show_notes_renderer( false);

        s = attributes.value("layers");
        conf->setVisibleLayers(s.toInt() );
        
        
        print_debug(DEBUG_CONFIG, "OpenGL visibility ranges set to %i (texture) and %i (details).",
                    conf->get_texture_vis(), conf->get_details_vis() );
        
        return TRUE;
    } else if (qName == "analyzers") {
        if (attributes.length() < 2) {
            print_debug(DEBUG_CONFIG, "(analyzers token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("exits");
        s = s.toLower();
        if (s == "on") 
            conf->set_exits_check(true);
        else 
            conf->set_exits_check(false);
        
        s = attributes.value("terrain");
        s = s.toLower();
        if (s == "on") 
            conf->set_terrain_check(true);
        else 
            conf->set_terrain_check(false);
        
        
        print_debug(DEBUG_CONFIG, "Analyzers: desc ON, exits %s, terrain %s.",
                    ON_OFF(conf->get_exits_check() ), ON_OFF(conf->get_terrain_check()) );
        
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


    } else if (qName == "guisettings") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(guisettings token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("always_on_top");
        s = s.toLower();
        if (s == "on") 
            conf->set_always_on_top(true);
        else 
            conf->set_always_on_top(false);

        print_debug(DEBUG_CONFIG, "GUI settings: always_on_top %s.", ON_OFF(conf->get_always_on_top()) );

        return TRUE;
    } else if (qName == "regionsflags") {
        if (attributes.length() < 1) {
            print_debug(DEBUG_CONFIG, "(guisettings token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("displayinrenderer");
        s = s.toLower();
        if (s == "on") 
            conf->set_display_regions_renderer(true);
        else 
            conf->set_display_regions_renderer(false);
            
        s = attributes.value("showinfo");
        s = s.toLower();
        if (s == "on") 
            conf->set_show_regions_info(true);
        else 
            conf->set_show_regions_info(false);
            
            

        print_debug(DEBUG_CONFIG, "GUI settings: always_on_top %s.", ON_OFF(conf->get_always_on_top()) );

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
            conf->set_brief_mode(true);
        else 
            conf->set_brief_mode(false);
        
        s = attributes.value("automerge");
        s = s.toLower();
        if (s == "on") 
            conf->set_automerge(true);
        else 
            conf->set_automerge(false);

        s = attributes.value("angrylinker");
        s = s.toLower();
        if (s == "on") 
            conf->set_angrylinker(true);
        else 
            conf->set_angrylinker(false);

        s = attributes.value("duallinker");
        s = s.toLower();
        if (s == "on") 
            conf->set_duallinker(true);
        else 
            conf->set_duallinker(false);



        print_debug(DEBUG_CONFIG, "Engine flags: briefmode %s, automerge %s, angrylinker %s.",
               ON_OFF(conf->get_brief_mode()), ON_OFF(conf->get_automerge()), 
                ON_OFF(conf->get_angrylinker()), ON_OFF(conf->get_duallinker()) );
        
        return TRUE;
    } else if (qName == "refresh") {
        if (attributes.length() < 3) {
            print_debug(DEBUG_CONFIG, "(refresh token) Not enough attributes in XML file!");
            exit(1);
        }        
        
        s = attributes.value("auto");
        s = s.toLower();
        if (s == "on") 
            conf->set_autorefresh(true);
        else 
            conf->set_autorefresh(false);
        
        s = attributes.value("roomnamequote");
        conf->set_name_quote(s.toInt());
        s = attributes.value("descquote");
        conf->set_desc_quote(s.toInt());

        print_debug(DEBUG_CONFIG, "Autorefresh settings: automatic refresh %s, roomname quote %i, desc quote %i.",
                ON_OFF(conf->get_autorefresh()), conf->get_name_quote(), 
                conf->get_desc_quote() );
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

        conf->add_texture(texture.desc, texture.filename, texture.pattern);
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

        conf->add_spell(spell);
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

        conf->set_window_rect(x, y, width, height);
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
        
        conf->set_renderer_angles(x,y,z);
            
        print_debug(DEBUG_CONFIG, "Loaded renderer angles : x %f, y %f, z %f",x, y, z);
        return TRUE;
    } else if (qName == "rendererpositions") {
        float x = 0;
        float y = 0;
        float z = 0;

        x = attributes.value("userx").toFloat();
        y = attributes.value("usery").toFloat();
        z = attributes.value("userz").toFloat();
        
        conf->set_renderer_position(x,y,z);
            
        print_debug(DEBUG_CONFIG, "Loaded renderer shift : x %f, y %f, z %f",x, y, z);
        return TRUE;
    
    } else if (qName == "misc") {
        int startup_mode = 0;
        startup_mode = attributes.value("startupmode").toInt();
        conf->set_startup_mode(startup_mode);
        print_debug(DEBUG_CONFIG, "Set startup mode: %d", startup_mode);
        
        QByteArray note_color = (QByteArray)attributes.value("notecolor").toAscii();
        conf->set_note_color(note_color);
        print_debug(DEBUG_CONFIG, "Set note color: %d", (const char*)note_color);

        return TRUE;
    }

  return TRUE;
}

