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
#include <QXmlStreamReader>
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

#define XML_ROOMNAME (1 << 0)
#define XML_DESC (1 << 1)
#define XML_NOTE (1 << 2)
#define XML_CONTENTS (1 << 3)

void CRoomManager::loadMap(QString filename)
{
    QFile xmlFile(filename);

    if (xmlFile.exists() == false) {
        print_debug(DEBUG_XML, "ERROR: The database file %s does NOT exist!\r\n", qPrintable(filename));
        return;
    }
    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        print_debug(DEBUG_XML, "ERROR: Unable to open the database file %s.\r\n", qPrintable(filename));
        return;
    }

    // Most sensitive application :-)
    // lock for any writing/reading!

    // Map is only (almost!) changed in mainwindow thread, which is "sequential"
    // so, when we come here, we are sequentially blocking the RoomManager (Map)
    // when progressBar updates it's status, it lets the App Loop to call whatever it wants (depepnding on events)
    // and we want those events to, well, fail, since there is no such thing as waiting for your own thread :-/
    Map.setBlocked(true);

    unsigned int currentMaximum = 22000;
    QProgressDialog progress("Loading the database...", "Abort Loading", 0, currentMaximum, renderer_window);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.show();

    QXmlStreamReader reader(&xmlFile);
    StructureParser *handler = new StructureParser(&progress, currentMaximum, this);

    print_debug(DEBUG_XML, "reading xml ...");
    fflush(stdout);

    //  lockForWrite();
    handler->parse(reader);

    if (progress.wasCanceled()) {
        print_debug(DEBUG_XML, "Loading was canceled");
        reinit();
    } else {
        progress.setLabelText("Preparing the loaded database");
        progress.setMaximum(size());
        progress.setValue(0);

        for (unsigned int i = 0; i < size(); i++) {
            progress.setValue(i);

            if (progress.wasCanceled()) {
                reinit();
                break;
            }

            CRoom *r = rooms[i];
            for (int exit = 0; exit <= 5; exit++)
                if (r->exits[exit] != nullptr) {
                    r->exits[exit] = getRoom((unsigned long)r->exits[exit]);
                }
        }
        progress.setValue(size());

        print_debug(DEBUG_XML, "done.");
    }

    Map.setBlocked(false);

    delete handler;
    return;
}

StructureParser::StructureParser(QProgressDialog *progress, unsigned int &currentMaximum, CRoomManager *parent)
    : parent(parent), progress(progress), currentMaximum(currentMaximum)
{
    readingRegion = false;
    abortLoading = false;
}

bool StructureParser::parse(QXmlStreamReader &reader)
{
    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.isStartElement()) {
            startElement(reader.name().toString(), reader.attributes());
        } else if (reader.isEndElement()) {
            endElement(reader.name().toString());
        } else if (reader.isCharacters() && !reader.isWhitespace()) {
            characters(reader.text().toString());
        }

        if (abortLoading) {
            break;
        }
    }

    if (reader.hasError()) {
        print_debug(DEBUG_XML, "XML parse error: %s", qPrintable(reader.errorString()));
        return false;
    }

    return !abortLoading;
}

bool StructureParser::endElement(const QString &qName)
{
    if (qName == "room") {
        parent->addRoom(r); /* tada! */

        if (r->id > currentMaximum) {
            currentMaximum = r->id;
            progress->setMaximum(currentMaximum);
        }
        unsigned int size = parent->size();
        progress->setValue(size);
    }
    if (qName == "region" && readingRegion) {
        parent->addRegion(region);
        region = nullptr;
        readingRegion = false;
    }
    flag = 0;

    if (progress->wasCanceled())
        abortLoading = true;

    return true;
}

bool StructureParser::characters(const QString &ch)
{
    if (abortLoading)
        return true;

    if (ch.isEmpty())
        return true;

    if (flag == XML_ROOMNAME) {
        r->setName(ch.toLocal8Bit());
    } else if (flag == XML_DESC) {
        r->setDesc(ch.toLocal8Bit());
    } else if (flag == XML_NOTE) {
        r->setNote(ch.toLocal8Bit());
    } else if (flag == XML_CONTENTS) {
        r->setContents(ch.toLocal8Bit());
    }
    return true;
}

bool StructureParser::startElement(const QString &qName, const QXmlStreamAttributes &attributes)
{
    if (abortLoading)
        return true;

    if (readingRegion == true) {
        if (qName == "alias") {
            QByteArray alias, door;

            s = attributes.value("name").toString();
            alias = s.toLocal8Bit();

            s = attributes.value("door").toString();
            door = s.toLocal8Bit();

            if (door != "" && alias != "")
                region->addDoor(alias, door);
            return true;
        }
    }

    if (qName == "exit") {
        unsigned int dir;
        unsigned int to;

        /* special */
        if (attributes.size() < 3) {
            print_debug(DEBUG_XML, "Not enough exit attributes in XML file!");
            exit(1);
        }

        s = attributes.value("dir").toString();
        dir = numbydir(s.toLocal8Bit().at(0));

        s = attributes.value("to").toString();
        if (s == "DEATH") {
            r->setExitDeath(dir);
        } else if (s == "UNDEFINED") {
            r->setExitUndefined(dir);
        } else {
            i = 0;
            bool NoError = false;
            to = s.toInt(&NoError);
            r->exits[dir] = (CRoom *)to;
        }

        s = attributes.value("door").toString();
        r->setDoor(dir, s.toLocal8Bit());

        // Load MMapper exit flags (backward compatible - defaults to 0 if missing)
        s = attributes.value("exitflags").toString();
        if (!s.isEmpty())
            r->setMMExitFlags(dir, s.toUInt());

        s = attributes.value("doorflags").toString();
        if (!s.isEmpty())
            r->setMMDoorFlags(dir, s.toUInt());

    } else if (qName == "roomname") {
        flag = XML_ROOMNAME;
        return true;
    } else if (qName == "desc") {
        flag = XML_DESC;
        return true;
    } else if (qName == "note") {
        if (attributes.count() > 0) {
            r->setNoteColor(attributes.value("color").toString().toLocal8Bit());
        } else {
            // QColor color = QColor(242, 128, 3, 255);
            // printf("color: %s\n\r",(const char*)color.name().toLocal8Bit());
            // r->setNoteColor(color);
            r->setNoteColor("");
        }
        flag = XML_NOTE;
        return true;
    } else if (qName == "contents") {
        flag = XML_CONTENTS;
        return true;
    } else if (qName == "room") {
        r = new CRoom;

        s = attributes.value("id").toString();
        r->id = s.toInt();

        s = attributes.value("x").toString();
        r->setX(s.toInt());

        s = attributes.value("y").toString();
        r->setY(s.toInt());

        s = attributes.value("z").toString();
        r->simpleSetZ(s.toInt());

        s = attributes.value("terrain").toString();
        r->setSector(conf->getSectorByDesc(s.toLocal8Bit()));

        s = attributes.value("region").toString();
        r->setRegion(s.toLocal8Bit());

        // Load MMapper properties (backward compatible - defaults to 0/UNDEFINED if missing)
        s = attributes.value("light").toString();
        if (!s.isEmpty())
            r->setLightType(s.toUInt());

        s = attributes.value("align").toString();
        if (!s.isEmpty())
            r->setAlignType(s.toUInt());

        s = attributes.value("portable").toString();
        if (!s.isEmpty())
            r->setPortableType(s.toUInt());

        s = attributes.value("ridable").toString();
        if (!s.isEmpty())
            r->setRidableType(s.toUInt());

        s = attributes.value("sundeath").toString();
        if (!s.isEmpty())
            r->setSundeathType(s.toUInt());

        s = attributes.value("mobflags").toString();
        if (!s.isEmpty())
            r->setMobFlags(s.toUInt());

        s = attributes.value("loadflags").toString();
        if (!s.isEmpty())
            r->setLoadFlags(s.toUInt());
    } else if (qName == "region") {
        region = new CRegion;

        readingRegion = true;
        s = attributes.value("name").toString();
        region->setName(s.toLocal8Bit());
    } else if (qName == "map") {
        s = attributes.value("rooms").toString();
        if (s.isEmpty()) {
            progress->setMaximum(currentMaximum);
        } else {
            progress->setMaximum(s.toInt());
        }
    }

    return true;
}

bool StructureParser::isAborted() const
{
    return abortLoading;
}

/* plain text file alike writing */
void CRoomManager::saveMap(QString filename)
{
    FILE *f;
    CRoom *p;
    int i;
    QString tmp;
    unsigned int z;

    print_debug(DEBUG_XML, "in xml_writebase()");

    f = fopen(qPrintable(filename), "w");
    if (f == nullptr) {
        printf("XML: Error - can not open the file: %s.\r\n", qPrintable(filename));
        return;
    }

    Map.setBlocked(true);

    QProgressDialog progress("Saving the database...", "Abort Saving", 0, size(), renderer_window);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    fprintf(f, "<map rooms=\"%i\">\n", size());

    {
        // SAVE REGIONS DATA
        CRegion *region;
        QList<CRegion *> regions;
        QMap<QByteArray, QByteArray> doors;

        regions = getAllRegions();
        fprintf(f, "  <regions>\n");
        for (int i = 0; i < regions.size(); i++) {
            region = regions[i];
            if (region->getName() == "default")
                continue;  // skip the default region -> its always in memory as the first one anyway!
            fprintf(f, "    <region name=\"%s\">\n", (const char *)region->getName());

            doors = region->getAllDoors();
            QMapIterator<QByteArray, QByteArray> iter(doors);
            while (iter.hasNext()) {
                iter.next();
                fprintf(f, "      <alias name=\"%s\" door=\"%s\"/>\n", (const char *)iter.key(),
                        (const char *)iter.value());
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

        // Build room attributes including MMapper properties (only if non-default)
        QString roomAttrs = QString("  <room id=\"%1\" x=\"%2\" y=\"%3\" z=\"%4\" "
                                    "terrain=\"%5\" region=\"%6\"")
                                .arg(p->id)
                                .arg(p->getX())
                                .arg(p->getY())
                                .arg(p->getZ())
                                .arg((const char *)conf->sectors[p->getTerrain()].desc)
                                .arg((const char *)p->getRegionName());

        // Add MMapper properties only if they have non-default values
        if (p->getLightType() != 0)
            roomAttrs += QString(" light=\"%1\"").arg(p->getLightType());
        if (p->getAlignType() != 0)
            roomAttrs += QString(" align=\"%1\"").arg(p->getAlignType());
        if (p->getPortableType() != 0)
            roomAttrs += QString(" portable=\"%1\"").arg(p->getPortableType());
        if (p->getRidableType() != 0)
            roomAttrs += QString(" ridable=\"%1\"").arg(p->getRidableType());
        if (p->getSundeathType() != 0)
            roomAttrs += QString(" sundeath=\"%1\"").arg(p->getSundeathType());
        if (p->getMobFlags() != 0)
            roomAttrs += QString(" mobflags=\"%1\"").arg(p->getMobFlags());
        if (p->getLoadFlags() != 0)
            roomAttrs += QString(" loadflags=\"%1\"").arg(p->getLoadFlags());

        roomAttrs += ">\n";
        fprintf(f, "%s", qPrintable(roomAttrs));

        fprintf(f, "    <roomname>%s</roomname>\n", (const char *)p->getName());
        fprintf(f, "    <desc>%s</desc>\n", (const char *)p->getDesc());
        fprintf(f, "    <note color=\"%s\">%s</note>\n", (const char *)p->getNoteColor(), (const char *)p->getNote());

        // Save contents if present
        if (!p->getContents().isEmpty()) {
            fprintf(f, "    <contents>%s</contents>\n", (const char *)p->getContents());
        }

        fprintf(f, "    <exits>\n");

        for (i = 0; i <= 5; i++) {
            if (p->isExitPresent(i) == true) {
                if (p->isExitNormal(i) == true) {
                    tmp = QString::number(p->exits[i]->id);
                } else {
                    if (p->isExitUndefined(i) == true)
                        tmp = QStringLiteral("UNDEFINED");
                    else if (p->isExitDeath(i) == true)
                        tmp = QStringLiteral("DEATH");
                }

                // Build exit element with optional MMapper flags
                QString exitLine = QString("      <exit dir=\"%1\" to=\"%2\" door=\"%3\"")
                                       .arg(QChar(exitnames[i][0]))
                                       .arg(tmp)
                                       .arg((const char *)p->getDoor(i));

                // Add MMapper exit flags only if non-zero
                if (p->getMMExitFlags(i) != 0)
                    exitLine += QString(" exitflags=\"%1\"").arg(p->getMMExitFlags(i));
                if (p->getMMDoorFlags(i) != 0)
                    exitLine += QString(" doorflags=\"%1\"").arg(p->getMMDoorFlags(i));

                exitLine += "/>\n";
                fprintf(f, "%s", qPrintable(exitLine));
            }
        }

        fprintf(f, "    </exits>\n");
        fprintf(f, "  </room>\n");
    }
    progress.setValue(size());

    fprintf(f, "</map>\r\n");
    fflush(f);
    fclose(f);

    Map.setBlocked(false);

    print_debug(DEBUG_XML, "xml_writebase() is done.\r\n");
}
