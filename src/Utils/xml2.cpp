/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *  Copyright (C) 2025       PandoraMapper Contributors
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

#include <QApplication>
#include <QBuffer>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QSaveFile>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "defines.h"
#include "xml2.h"
#include "CConfigurator.h"
#include "utils.h"

#include "Map/CRoomManager.h"
#include "Proxy/CDispatcher.h"
#include "Proxy/proxy.h"
#include "Gui/mainwindow.h"
#include "Engine/CStacksManager.h"
#include "Engine/CEngine.h"
#include "Renderer/renderer.h"

// Current file format version
static const int MAP_FILE_VERSION = 2;

// Flags for text content parsing
#define XML_ROOMNAME (1 << 0)
#define XML_DESC     (1 << 1)
#define XML_NOTE     (1 << 2)
#define XML_CONTENTS (1 << 3)

// Filter invalid XML characters (control chars except tab, newline, carriage return)
static QByteArray filterInvalidXmlChars(const QByteArray &input, int *strippedCount = nullptr)
{
    QByteArray output;
    output.reserve(input.size());
    int stripped = 0;

    for (int i = 0; i < input.size(); i++) {
        unsigned char ch = static_cast<unsigned char>(input.at(i));
        // Valid XML chars: #x9 | #xA | #xD | [#x20-#xD7FF]
        if (ch == 0x9 || ch == 0xA || ch == 0xD || ch >= 0x20) {
            output.append(static_cast<char>(ch));
        } else {
            stripped++;
        }
    }

    if (strippedCount)
        *strippedCount = stripped;
    return output;
}

// ============================================================================
// LOADING
// ============================================================================

void CRoomManager::loadMap(QString filename)
{
    QFile xmlFile(filename);

    if (!xmlFile.exists()) {
        print_debug(DEBUG_XML, "ERROR: The database file %s does NOT exist!", qPrintable(filename));
        send_to_user("--[ Map load failed: file not found (%s)\r\n", qPrintable(filename));
        return;
    }

    if (!xmlFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        print_debug(DEBUG_XML, "ERROR: Unable to open the database file %s.", qPrintable(filename));
        send_to_user("--[ Map load failed: unable to open (%s)\r\n", qPrintable(filename));
        return;
    }

    QByteArray rawXml = xmlFile.readAll();
    xmlFile.close();

    // Filter invalid control characters that break XML parsing
    int stripped = 0;
    QByteArray filteredXml = filterInvalidXmlChars(rawXml, &stripped);
    if (stripped > 0) {
        print_debug(DEBUG_XML, "WARNING: Stripped %d invalid control characters from XML.", stripped);
        send_to_user("--[ Map load: stripped %d invalid characters\r\n", stripped);
    }

    Map.setBlocked(true);
    send_to_user("--[ Loading map: %s\r\n", qPrintable(filename));

    // Clear references to rooms BEFORE reinit deletes them
    stacker.reset();
    selections.resetSelection();
    if (engine)
        engine->resetAddedRoomVar();

    // Clear existing map data
    print_debug(DEBUG_XML, "Clearing existing map data...");
    reinit();

    unsigned int currentMaximum = 22000;
    QProgressDialog progress("Loading the database...", "Abort Loading", 0, currentMaximum, renderer_window);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.show();

    QBuffer buffer(&filteredXml);
    buffer.open(QIODevice::ReadOnly);
    QXmlStreamReader reader(&buffer);
    StructureParser handler(&progress, currentMaximum, this);

    print_debug(DEBUG_XML, "Parsing XML...");
    bool parseOk = handler.parse(reader);

    if (!parseOk && handler.hasError()) {
        QString msg = QString("XML parse error: %1 (line %2, col %3)")
                          .arg(handler.errorMessage())
                          .arg(handler.errorLine())
                          .arg(handler.errorColumn());
        print_debug(DEBUG_XML, "%s", qPrintable(msg));
        send_to_user("--[ %s\r\n", qPrintable(msg));
        if (renderer_window) {
            QMessageBox::warning(renderer_window, "XML Load Error", msg);
        }
    }

    if (progress.wasCanceled()) {
        print_debug(DEBUG_XML, "Loading was canceled");
        send_to_user("--[ Map load canceled\r\n");
        reinit();
    } else {
        print_debug(DEBUG_XML, "Parsed %d rooms, resolving exits...", size());
        progress.setLabelText("Resolving exit connections...");
        progress.setMaximum(size());
        progress.setValue(0);

        // Second pass: resolve exit pointers
        unsigned int resolvedExits = 0;
        unsigned int failedExits = 0;

        for (unsigned int i = 0; i < size(); i++) {
            progress.setValue(i);
            if (progress.wasCanceled()) {
                reinit();
                break;
            }

            CRoom *room = rooms[i];
            for (int dir = 0; dir <= 5; dir++) {
                if (room->exits[dir] != nullptr) {
                    // Exit pointer temporarily holds the target room ID (cast as pointer)
                    unsigned int targetId = static_cast<unsigned int>(reinterpret_cast<uintptr_t>(room->exits[dir]));
                    CRoom *target = getRoom(targetId);

                    if (target != nullptr) {
                        room->exits[dir] = target;
                        resolvedExits++;
                    } else {
                        // Target room not found - mark as undefined
                        room->exits[dir] = nullptr;
                        room->setExitUndefined(dir);
                        failedExits++;
                        print_debug(DEBUG_XML, "Room %d exit %d: target %d not found", room->id, dir, targetId);
                    }
                }
            }
        }

        progress.setValue(size());
        print_debug(DEBUG_XML, "Exit resolution: %d resolved, %d failed", resolvedExits, failedExits);
        send_to_user("--[ Map loaded: %d rooms (%d exits resolved, %d failed)\r\n", size(), resolvedExits, failedExits);

        // Focus view on room 1 or first available room
        CRoom *focusRoom = getRoom(1);
        if (focusRoom == nullptr && size() > 0) {
            focusRoom = rooms[0];
        }
        if (focusRoom != nullptr) {
            stacker.reset();
            stacker.put(focusRoom);
            stacker.swap();
            if (renderer_window && renderer_window->renderer) {
                renderer_window->renderer->setUserX(0, true);
                renderer_window->renderer->setUserY(0, true);
            }
        }
    }

    Map.setBlocked(false);
}

// ============================================================================
// XML Parser Implementation
// ============================================================================

StructureParser::StructureParser(QProgressDialog *progress, unsigned int &currentMaximum, CRoomManager *parent)
    : parent(parent), progress(progress), currentMaximum(currentMaximum)
{
    flag = 0;
    readingRegion = false;
    abortLoading = false;
    parseError = false;
    currentRoomId = 0;
    errorLineNumber = 0;
    errorColumnNumber = 0;
    currentRoom = nullptr;
    currentRegion = nullptr;
}

bool StructureParser::parse(QXmlStreamReader &reader)
{
    while (!reader.atEnd() && !abortLoading) {
        reader.readNext();

        if (reader.isStartElement()) {
            startElement(reader.name().toString(), reader.attributes());
        } else if (reader.isEndElement()) {
            endElement(reader.name().toString());
        } else if (reader.isCharacters() && !reader.isWhitespace()) {
            characters(reader.text().toString());
        }
    }

    if (reader.hasError()) {
        parseError = true;
        errorMsg = reader.errorString();
        errorLineNumber = reader.lineNumber();
        errorColumnNumber = reader.columnNumber();
        print_debug(DEBUG_XML, "XML parse error at line %lld: %s", reader.lineNumber(), qPrintable(errorMsg));

        // Clean up partially parsed room
        if (currentRoom != nullptr) {
            delete currentRoom;
            currentRoom = nullptr;
        }
        return false;
    }

    return !abortLoading;
}

bool StructureParser::startElement(const QString &qName, const QXmlStreamAttributes &attributes)
{
    if (abortLoading)
        return false;

    // Handle region aliases
    if (readingRegion && qName == "alias") {
        QString aliasName = attributes.value("name").toString();
        QString doorName = attributes.value("door").toString();
        if (!aliasName.isEmpty() && !doorName.isEmpty() && currentRegion) {
            currentRegion->addDoor(aliasName.toUtf8(), doorName.toUtf8());
        }
        return true;
    }

    if (qName == "room") {
        currentRoom = new CRoom();

        QString idStr = attributes.value("id").toString();
        bool ok = false;
        int id = idStr.toInt(&ok);
        if (!ok || id < 0 || id >= MAX_ROOMS) {
            print_debug(DEBUG_XML, "Invalid room ID: %s", qPrintable(idStr));
            delete currentRoom;
            currentRoom = nullptr;
            return true;
        }
        currentRoom->id = id;
        currentRoomId = id;

        currentRoom->setX(attributes.value("x").toString().toInt());
        currentRoom->setY(attributes.value("y").toString().toInt());
        currentRoom->simpleSetZ(attributes.value("z").toString().toInt());

        QString terrain = attributes.value("terrain").toString();
        currentRoom->setSector(conf->getSectorByDesc(terrain.toUtf8()));

        QString region = attributes.value("region").toString();
        currentRoom->setRegion(region.toUtf8());

        // MMapper properties (optional, backward compatible)
        QString val;
        val = attributes.value("light").toString();
        if (!val.isEmpty()) currentRoom->setLightType(val.toUInt());

        val = attributes.value("align").toString();
        if (!val.isEmpty()) currentRoom->setAlignType(val.toUInt());

        val = attributes.value("portable").toString();
        if (!val.isEmpty()) currentRoom->setPortableType(val.toUInt());

        val = attributes.value("ridable").toString();
        if (!val.isEmpty()) currentRoom->setRidableType(val.toUInt());

        val = attributes.value("sundeath").toString();
        if (!val.isEmpty()) currentRoom->setSundeathType(val.toUInt());

        val = attributes.value("mobflags").toString();
        if (!val.isEmpty()) currentRoom->setMobFlags(val.toUInt());

        val = attributes.value("loadflags").toString();
        if (!val.isEmpty()) currentRoom->setLoadFlags(val.toUInt());

    } else if (qName == "exit" && currentRoom) {
        QString dirStr = attributes.value("dir").toString();
        if (dirStr.isEmpty()) {
            print_debug(DEBUG_XML, "Exit missing 'dir' attribute in room %d", currentRoomId);
            return true;
        }

        int dir = numbydir(dirStr.at(0).toLatin1());
        if (dir < 0 || dir > 5) {
            print_debug(DEBUG_XML, "Invalid exit direction '%s' in room %d", qPrintable(dirStr), currentRoomId);
            return true;
        }

        QString toStr = attributes.value("to").toString();
        if (toStr == "DEATH") {
            currentRoom->setExitDeath(dir);
        } else if (toStr == "UNDEFINED") {
            currentRoom->setExitUndefined(dir);
        } else {
            bool ok = false;
            unsigned int targetId = toStr.toUInt(&ok);
            if (ok) {
                // Store target ID temporarily in the pointer field (resolved later)
                currentRoom->exits[dir] = reinterpret_cast<CRoom *>(static_cast<uintptr_t>(targetId));
            } else {
                print_debug(DEBUG_XML, "Invalid exit target '%s' in room %d", qPrintable(toStr), currentRoomId);
                currentRoom->setExitUndefined(dir);
            }
        }

        QString door = attributes.value("door").toString();
        currentRoom->setDoor(dir, door.toUtf8());

        // MMapper exit flags (optional)
        QString exitFlags = attributes.value("exitflags").toString();
        if (!exitFlags.isEmpty()) currentRoom->setMMExitFlags(dir, exitFlags.toUInt());

        QString doorFlags = attributes.value("doorflags").toString();
        if (!doorFlags.isEmpty()) currentRoom->setMMDoorFlags(dir, doorFlags.toUInt());

    } else if (qName == "roomname") {
        flag = XML_ROOMNAME;
        textBuffer.clear();

    } else if (qName == "desc") {
        flag = XML_DESC;
        textBuffer.clear();

    } else if (qName == "note") {
        if (currentRoom) {
            QString color = attributes.value("color").toString();
            currentRoom->setNoteColor(color.toUtf8());
        }
        flag = XML_NOTE;
        textBuffer.clear();

    } else if (qName == "contents") {
        flag = XML_CONTENTS;
        textBuffer.clear();

    } else if (qName == "region") {
        currentRegion = new CRegion();
        readingRegion = true;

        QString name = attributes.value("name").toString();
        currentRegion->setName(name.toUtf8());

        QString localspace = attributes.value("localspace").toString();
        if (!localspace.isEmpty()) {
            currentRegion->setLocalSpaceId(localspace.toInt());
        }

    } else if (qName == "map") {
        QString roomsStr = attributes.value("rooms").toString();
        if (!roomsStr.isEmpty()) {
            progress->setMaximum(roomsStr.toInt());
        }
        // Check version if present
        QString version = attributes.value("version").toString();
        if (!version.isEmpty()) {
            int ver = version.toInt();
            print_debug(DEBUG_XML, "Map file version: %d", ver);
        }

    } else if (qName == "localspace") {
        int parsedId = attributes.value("id").toString().toInt();
        QString name = attributes.value("name").toString();

        int id = (parsedId > 0) ? parent->addLocalSpaceWithId(name.toUtf8(), parsedId)
                                : parent->addLocalSpace(name.toUtf8());

        LocalSpace *space = parent->getLocalSpace(id);
        if (space) {
            space->portalX = attributes.value("x").toString().toFloat();
            space->portalY = attributes.value("y").toString().toFloat();
            space->portalZ = attributes.value("z").toString().toFloat();
            space->portalW = attributes.value("w").toString().toFloat();
            space->portalH = attributes.value("h").toString().toFloat();
            space->hasPortal = true;
        }
    }

    return true;
}

bool StructureParser::endElement(const QString &qName)
{
    // Process accumulated text content
    if (currentRoom) {
        if (flag == XML_ROOMNAME) {
            currentRoom->setName(textBuffer.toUtf8());
        } else if (flag == XML_DESC) {
            currentRoom->setDesc(textBuffer.toUtf8());
        } else if (flag == XML_NOTE) {
            currentRoom->setNote(textBuffer.toUtf8());
        } else if (flag == XML_CONTENTS) {
            currentRoom->setContents(textBuffer.toUtf8());
        }
    }
    textBuffer.clear();
    flag = 0;

    if (qName == "room" && currentRoom) {
        if (currentRoom->id == 0) {
            print_debug(DEBUG_XML, "WARNING: Room with ID 0 - this may cause issues");
        }

        parent->addRoom(currentRoom);
        currentRoom = nullptr;  // Ownership transferred to parent

        unsigned int count = parent->size();
        if (count > currentMaximum) {
            currentMaximum = count;
            progress->setMaximum(currentMaximum);
        }
        progress->setValue(count);

        if (count % 1000 == 0) {
            print_debug(DEBUG_XML, "Loaded %d rooms...", count);
        }

    } else if (qName == "region" && readingRegion && currentRegion) {
        parent->addRegion(currentRegion);
        currentRegion = nullptr;  // Ownership transferred
        readingRegion = false;
    }

    if (progress->wasCanceled()) {
        abortLoading = true;
        // Clean up
        if (currentRoom) {
            delete currentRoom;
            currentRoom = nullptr;
        }
        if (currentRegion) {
            delete currentRegion;
            currentRegion = nullptr;
        }
    }

    return true;
}

bool StructureParser::characters(const QString &ch)
{
    if (abortLoading || ch.isEmpty())
        return true;

    // Accumulate text content
    if (flag != 0) {
        textBuffer += ch;
    }
    return true;
}

bool StructureParser::isAborted() const
{
    return abortLoading;
}

// ============================================================================
// SAVING
// ============================================================================

void CRoomManager::saveMap(QString filename)
{
    print_debug(DEBUG_XML, "Saving map to: %s", qPrintable(filename));

    // Use QSaveFile for atomic writes (writes to temp file, then renames)
    // This prevents corruption if save is interrupted
    QSaveFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        print_debug(DEBUG_XML, "ERROR: Cannot open file for writing: %s", qPrintable(filename));
        send_to_user("--[ Map save failed: cannot open file\r\n");
        return;
    }

    Map.setBlocked(true);
    send_to_user("--[ Saving map: %s\r\n", qPrintable(filename));

    QProgressDialog progress("Saving the database...", "Abort Saving", 0, size(), renderer_window);
    progress.setWindowModality(Qt::WindowModal);
    progress.show();

    QXmlStreamWriter xml(&file);
    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);

    // XML declaration
    xml.writeStartDocument("1.0");

    // Root element with metadata
    xml.writeStartElement("map");
    xml.writeAttribute("version", QString::number(MAP_FILE_VERSION));
    xml.writeAttribute("rooms", QString::number(size()));

    // Local spaces
    xml.writeStartElement("localspaces");
    QVector<LocalSpace *> spaces = getLocalSpaces();
    for (LocalSpace *space : spaces) {
        if (!space) continue;

        xml.writeStartElement("localspace");
        xml.writeAttribute("id", QString::number(space->id));
        xml.writeAttribute("name", QString::fromUtf8(space->name));
        xml.writeAttribute("x", QString::number(space->portalX));
        xml.writeAttribute("y", QString::number(space->portalY));
        xml.writeAttribute("z", QString::number(space->portalZ));
        xml.writeAttribute("w", QString::number(space->portalW));
        xml.writeAttribute("h", QString::number(space->portalH));
        xml.writeEndElement();  // localspace
    }
    xml.writeEndElement();  // localspaces

    // Regions
    xml.writeStartElement("regions");
    QList<CRegion *> regionList = getAllRegions();
    for (CRegion *region : regionList) {
        if (region->getName() == "default")
            continue;  // Skip default region

        xml.writeStartElement("region");
        xml.writeAttribute("name", QString::fromUtf8(region->getName()));
        if (region->getLocalSpaceId() > 0) {
            xml.writeAttribute("localspace", QString::number(region->getLocalSpaceId()));
        }

        // Door aliases
        QMap<QByteArray, QByteArray> doors = region->getAllDoors();
        QMapIterator<QByteArray, QByteArray> iter(doors);
        while (iter.hasNext()) {
            iter.next();
            xml.writeStartElement("alias");
            xml.writeAttribute("name", QString::fromUtf8(iter.key()));
            xml.writeAttribute("door", QString::fromUtf8(iter.value()));
            xml.writeEndElement();  // alias
        }

        xml.writeEndElement();  // region
    }
    xml.writeEndElement();  // regions

    // Rooms
    bool aborted = false;
    for (unsigned int i = 0; i < size(); i++) {
        progress.setValue(i);
        QApplication::processEvents();

        if (progress.wasCanceled()) {
            aborted = true;
            break;
        }

        CRoom *room = rooms[i];

        xml.writeStartElement("room");
        xml.writeAttribute("id", QString::number(room->id));
        xml.writeAttribute("x", QString::number(room->getX()));
        xml.writeAttribute("y", QString::number(room->getY()));
        xml.writeAttribute("z", QString::number(room->getZ()));

        // Terrain with bounds check
        int terrain = room->getTerrain();
        if (terrain >= 0 && terrain < static_cast<int>(conf->sectors.size())) {
            xml.writeAttribute("terrain", QString::fromUtf8(conf->sectors[terrain].desc));
        } else {
            xml.writeAttribute("terrain", "UNDEFINED");
        }

        xml.writeAttribute("region", QString::fromUtf8(room->getRegionName()));

        // MMapper properties (only if non-default)
        if (room->getLightType() != 0)
            xml.writeAttribute("light", QString::number(room->getLightType()));
        if (room->getAlignType() != 0)
            xml.writeAttribute("align", QString::number(room->getAlignType()));
        if (room->getPortableType() != 0)
            xml.writeAttribute("portable", QString::number(room->getPortableType()));
        if (room->getRidableType() != 0)
            xml.writeAttribute("ridable", QString::number(room->getRidableType()));
        if (room->getSundeathType() != 0)
            xml.writeAttribute("sundeath", QString::number(room->getSundeathType()));
        if (room->getMobFlags() != 0)
            xml.writeAttribute("mobflags", QString::number(room->getMobFlags()));
        if (room->getLoadFlags() != 0)
            xml.writeAttribute("loadflags", QString::number(room->getLoadFlags()));

        // Room name (filter invalid chars)
        xml.writeTextElement("roomname", QString::fromUtf8(filterInvalidXmlChars(room->getName())));

        // Description
        xml.writeTextElement("desc", QString::fromUtf8(filterInvalidXmlChars(room->getDesc())));

        // Note with color
        xml.writeStartElement("note");
        if (!room->getNoteColor().isEmpty()) {
            xml.writeAttribute("color", QString::fromUtf8(room->getNoteColor()));
        }
        xml.writeCharacters(QString::fromUtf8(filterInvalidXmlChars(room->getNote())));
        xml.writeEndElement();  // note

        // Contents (optional)
        if (!room->getContents().isEmpty()) {
            xml.writeTextElement("contents", QString::fromUtf8(filterInvalidXmlChars(room->getContents())));
        }

        // Exits
        xml.writeStartElement("exits");
        for (int dir = 0; dir <= 5; dir++) {
            if (!room->isExitPresent(dir))
                continue;

            xml.writeStartElement("exit");
            xml.writeAttribute("dir", QString(QChar(exitnames[dir][0])));

            // Determine exit target
            QString target;
            if (room->isExitDeath(dir)) {
                target = "DEATH";
            } else if (room->isExitUndefined(dir)) {
                target = "UNDEFINED";
            } else if (room->isExitNormal(dir) && room->exits[dir] != nullptr) {
                target = QString::number(room->exits[dir]->id);
            } else {
                // Fallback for any other case
                target = "UNDEFINED";
                print_debug(DEBUG_XML, "Room %d exit %d: unexpected state, marking UNDEFINED", room->id, dir);
            }
            xml.writeAttribute("to", target);

            xml.writeAttribute("door", QString::fromUtf8(room->getDoor(dir)));

            // MMapper exit flags (only if non-zero)
            if (room->getMMExitFlags(dir) != 0)
                xml.writeAttribute("exitflags", QString::number(room->getMMExitFlags(dir)));
            if (room->getMMDoorFlags(dir) != 0)
                xml.writeAttribute("doorflags", QString::number(room->getMMDoorFlags(dir)));

            xml.writeEndElement();  // exit
        }
        xml.writeEndElement();  // exits

        xml.writeEndElement();  // room
    }

    progress.setValue(size());

    xml.writeEndElement();  // map
    xml.writeEndDocument();

    Map.setBlocked(false);

    if (aborted) {
        file.cancelWriting();
        print_debug(DEBUG_XML, "Save aborted by user");
        send_to_user("--[ Map save aborted\r\n");
    } else if (!file.commit()) {
        print_debug(DEBUG_XML, "ERROR: Failed to commit save file: %s", qPrintable(file.errorString()));
        send_to_user("--[ Map save failed: %s\r\n", qPrintable(file.errorString()));
    } else {
        print_debug(DEBUG_XML, "Saved %d rooms to %s", size(), qPrintable(filename));
        send_to_user("--[ Map saved: %d rooms\r\n", size());
    }
}
