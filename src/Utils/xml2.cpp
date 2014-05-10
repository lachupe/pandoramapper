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


/*
  $Id: xml2.cpp,v 1.15 2006/08/06 13:50:53 porien Exp $
*/
#include <QApplication>
#include <QFile>
#include <QXmlDefaultHandler>
#include <QString>
#include <QProgressDialog>
#include <QMessageBox>

#include "defines.h"
#include "xml2.h"
#include "CConfigurator.h"
#include "utils.h"

#include "Map/CRoomManager.h"
#include "Proxy/CDispatcher.h"
#include "Gui/mainwindow.h"


#define XML_ROOMNAME    (1 << 0)
#define XML_DESC        (1 << 1)
#define XML_NOTE        (1 << 2)

void CRoomManager::loadXmlMap( QString filename)
{
  QFile xmlFile( filename);
  QXmlInputSource source( &xmlFile );

  QXmlSimpleReader reader;


  if (xmlFile.exists() == false) {
      print_debug(DEBUG_XML, "ERROR: The database file %s does NOT exist!\r\n", qPrintable(filename) );
      return;
  }

  // Most sensitive application :-)
  // lock for any writing/reading!

  // Map is only (almost!) changed in mainwindow thread, which is "sequential"
  // so, when we come here, we are sequentially blocking the RoomManager (Map)
  // when progressBar updates it's status, it lets the App Loop to call whatever it wants (depepnding on events)
  // and we want those events to, well, fail, since there is no such thing as waiting for your own thread :-/
  Map.setBlocked( true );
  
  unsigned int currentMaximum = 22000;
  QProgressDialog progress("Loading the database...", "Abort Loading", 0, currentMaximum, renderer_window);
  progress.setWindowModality(Qt::ApplicationModal);
  progress.show();


  StructureParser * handler = new StructureParser(&progress, currentMaximum, this);
  reader.setContentHandler( handler );
    
  print_debug(DEBUG_XML, "reading xml ...");
  fflush(stdout);

//  lockForWrite();
  reader.parse( source );

  if (progress.wasCanceled()) {
	  print_debug(DEBUG_XML, "Loading was canceled");
	  reinit();
  }

  Map.setBlocked( false );

  delete handler;
  return;
}




StructureParser::StructureParser(QProgressDialog *progress, unsigned int& currentMaximum, CRoomManager *parent): 
	QXmlDefaultHandler(), parent(parent), progress(progress),  currentMaximum(currentMaximum)
{
    readingRegion = false;
    abortLoading = false;
}


bool StructureParser::endElement( const QString& , const QString& , const QString& qName)
{
  if (qName == "room") {
      parent->addRoom(r);	/* tada! */
      
      if (r->getId() > currentMaximum) {
          currentMaximum = r->getId();
    	  progress->setMaximum(currentMaximum);
      }
	  unsigned int size = parent->size();
	  progress->setValue( size );
  }  
  if (qName == "region" && readingRegion)  {
      parent->addRegion( region );
      region = NULL;      
      readingRegion = false;
  }        
  flag = 0;    
    

  if (progress->wasCanceled())
	  abortLoading = true;
  
  return true;
}

bool StructureParser::characters( const QString& ch)
{
	if (abortLoading)
		return true;
	
    if (ch == NULL || ch == "")
    	return true;
    
    if (flag == XML_ROOMNAME) {
    	r->setName(ch.toLocal8Bit());
    } else if (flag == XML_DESC) {
    	r->setDesc(ch.toLocal8Bit());
    } else if (flag == XML_NOTE) {
    	r->setNote(ch.toLocal8Bit());
    }
    return true;
} 


bool StructureParser::startElement( const QString& , const QString& , 
                                    const QString& qName, 
                                    const QXmlAttributes& attributes)
{
	if (abortLoading)
		return true;
	
    if (readingRegion == true) {
        if (qName == "alias") {
            QByteArray alias, door;
            
            s = attributes.value("name");
            alias = s.toLocal8Bit();
            
            s = attributes.value("door");
            door = s.toLocal8Bit();
            
            if (door != "" && alias != "")
                region->addDoor(alias, door);
            return true;
        } 
   }
        
  if (qName == "exit") {
    ExitDirection dir;
    unsigned int to;
        
    /* special */
    if (attributes.length() < 3) {
      print_debug(DEBUG_XML, "Not enough exit attributes in XML file!");
      exit(1);
    }        
      
    s = attributes.value("dir");
    dir = numbydir(s.toLocal8Bit().at(0));
      
    s = attributes.value("to");
    if (s == "DEATH") {
        r->setExitDeath( dir );
    } else if (s == "UNDEFINED") {
        r->setExitUndefined( dir);
    } else {
    	i = 0;
    	bool NoError = false;
    	to = s.toInt(&NoError);
        r->setExit(dir, to);
    }

    s = attributes.value("door");
    r->setDoor(dir, s.toLocal8Bit());
    
  } else if (qName == "roomname") {
    flag = XML_ROOMNAME;
    return true;
  } else if (qName == "desc") {
    flag = XML_DESC;
    return true;
  } else if (qName == "note") {
      if(attributes.count() > 0) {
          r->setNoteColor(attributes.value("color").toLocal8Bit());
      } else {
          //QColor color = QColor(242, 128, 3, 255);
          //printf("color: %s\n\r",(const char*)color.name().toLocal8Bit());
          //r->setNoteColor(color);
          r->setNoteColor("");
      }
      flag = XML_NOTE;
    return true;
  } else if (qName == "room") {

      s = attributes.value("id");

      r = parent->createRoom( s.toInt() );
      
      s = attributes.value("x");
      r->setX( s.toInt() );

      s = attributes.value("y");
      r->setY( s.toInt() );

      s = attributes.value("z");
      r->simpleSetZ( s.toInt() );

      s = attributes.value("terrain");

      r->setSector( conf->getSectorByDesc(s.toLocal8Bit()) );

      
      s = attributes.value("region");
      r->setRegion(s.toLocal8Bit());
  } else if (qName == "region") {
     region = new CRegion;
            
     readingRegion = true;
     s = attributes.value("name");
     region->setName(s.toLocal8Bit());
  } else if (qName == "map") {
	  s = attributes.value("rooms");
	  if (s.isEmpty()) {
		  progress->setMaximum( currentMaximum );
	  } else {
		  progress->setMaximum( s.toInt() );
	  }
  }
  
  return true;
}

/* plain text file alike writing */
void CRoomManager::saveXmlMap(QString filename)
{
  FILE *f;
  CRoom *p;
  char tmp[4028];
  unsigned int z;
    
  print_debug(DEBUG_XML, "in xml_writebase()");
    
  f = fopen(qPrintable(filename), "w");
  if (f == NULL) {
    printf("XML: Error - can not open the file: %s.\r\n", qPrintable(filename));
    return;
  }


  Map.setBlocked( true );
  
  QProgressDialog progress("Saving the database...", "Abort Saving", 0, size(), renderer_window);
  progress.setWindowModality(Qt::WindowModal);
  progress.show();

  fprintf(f, "<map rooms=\"%i\">\n", size() );
  

  {
        // SAVE REGIONS DATA
        CRegion    *region;
        QList<CRegion *> regions;
        QMap<QByteArray, QByteArray> doors;        
        
        regions = getAllRegions();
        fprintf(f, "  <regions>\n");
        for (int i=0; i < regions.size(); i++) {
            region = regions[i];
            if (region->getName() == "default")
                continue;   // skip the default region -> its always in memory as the first one anyway!
            fprintf(f, "    <region name=\"%s\">\n", (const char *) region->getName() );
            
            doors = region->getAllDoors();
            QMapIterator<QByteArray, QByteArray> iter(doors);
            while (iter.hasNext()) {
                iter.next();
                fprintf(f, "      <alias name=\"%s\" door=\"%s\"/>\n",  (const char *)  iter.key(), (const char *) iter.value() );
            }
            fprintf(f, "    </region>\n");
        }
        fprintf(f, "  </regions>\n");
   }
  
  
    for (z = 0; z < size(); z++) {
    	
  	  	progress.setValue(z);

        if (progress.wasCanceled()) 
             break;

        p = rooms[z];
    
        fprintf(f,  "  <room id=\"%i\" x=\"%i\" y=\"%i\" z=\"%i\" "
                "terrain=\"%s\" region=\"%s\">\n",
                p->getId(), p->getX(), p->getY(), p->getZ(),
                (const char *) conf->sectors[ p->getTerrain() ].desc, 
                (const char *) p->getRegionName());
            
            
        fprintf(f, "    <roomname>%s</roomname>\n", (const char *) p->getName());
        fprintf(f, "    <desc>%s</desc>\n",  (const char *) p->getDesc() );
        fprintf(f, "    <note color=\"%s\">%s</note>\n", 
                (const char *) p->getNoteColor(), 
                (const char *) p->getNote() );
            
        fprintf(f, "    <exits>\n");
    
    
            
        for (int ind = 0; ind <= 5; ind++) {
            ExitDirection i = static_cast<ExitDirection>(ind);
            if (p->isExitPresent(i) == true) {
    
                if (p->isExitNormal(i) == true) {
                    sprintf(tmp, "%d", p->getExitLeadsTo(i));
                } else {
                    if (p->isExitUndefined(i) == true)
                        sprintf(tmp, "%s", "UNDEFINED");
                    else if (p->isExitDeath(i) == true)
                        sprintf(tmp, "%s", "DEATH");
                }
                                    
                fprintf(f, "      <exit dir=\"%c\" to=\"%s\" door=\"%s\"/>\n",
                        exitnames[i][0], tmp, (const char *) p->getDoor(i) ); 
            }   
    
        }
            
        fprintf(f, "    </exits>\n");
        fprintf(f,  "  </room>\n");
 
  }
  progress.setValue(size());

  fprintf(f, "</map>\r\n");
  fflush(f);
  fclose(f);
    
  Map.setBlocked( false );

  print_debug(DEBUG_XML, "xml_writebase() is done.\r\n");
}
