/*
 *  Pandora MUME mapper
 *
 *  Copyright (C) 2000-2009  Azazello
 *  Copyright (C) 2025 PandoraMapper Contributors
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

#include "MMapperImport.h"

#include <QBuffer>
#include <QDataStream>
#include <QFile>
#include <QProgressDialog>
#include <QApplication>

#include "defines.h"
#include "CConfigurator.h"
#include "utils.h"

#include "Map/CRoom.h"
#include "Map/CRoomManager.h"

QString MMapperImport::s_lastError;

QString MMapperImport::lastError()
{
    return s_lastError;
}

uint32_t MMapperImport::checkFile(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        s_lastError = QString("Cannot open file: %1").arg(filename);
        return 0;
    }

    QDataStream stream(&file);

    int32_t magic;
    stream >> magic;
    if (magic != MMAPPER_MAGIC) {
        s_lastError = "Not an MMapper file (invalid magic number)";
        return 0;
    }

    uint32_t version;
    stream >> version;

    file.close();
    return version;
}

int MMapperImport::convertDirection(int mmapperDir)
{
    // MMapper: NORTH=0, SOUTH=1, EAST=2, WEST=3, UP=4, DOWN=5, UNKNOWN=6
    // Pandora: NORTH=0, EAST=1, SOUTH=2, WEST=3, UP=4, DOWN=5
    switch (mmapperDir) {
    case DIR_NORTH:
        return NORTH;
    case DIR_SOUTH:
        return SOUTH;
    case DIR_EAST:
        return EAST;
    case DIR_WEST:
        return WEST;
    case DIR_UP:
        return UP;
    case DIR_DOWN:
        return DOWN;
    default:
        return -1;  // Unknown direction - skip
    }
}

char MMapperImport::convertTerrain(uint8_t mmapperTerrain)
{
    // Map MMapper terrain enum to PandoraMapper sector description
    // These descriptions should match the sector names in the config
    const char *terrainNames[] = {"UNDEFINED", "INDOORS", "CITY",   "FIELD",      "FOREST",
                                  "HILLS",     "MOUNT",   "SHALLW", "WATER",      "RAPID",
                                  "UNDERWAT",  "ROAD",    "BRUSH",  "TUNNEL",     "CAVERN"};

    if (mmapperTerrain >= 15) {
        mmapperTerrain = 0;  // Default to UNDEFINED
    }

    // Try to find matching sector in configurator
    int sectorIndex = conf->getSectorByDesc(QByteArray(terrainNames[mmapperTerrain]));
    if (sectorIndex > 0) {
        return static_cast<char>(sectorIndex);
    }

    // Fallback: try common alternative names
    switch (mmapperTerrain) {
    case TERRAIN_INDOORS:
        sectorIndex = conf->getSectorByDesc("INSIDE");
        break;
    case TERRAIN_MOUNTAINS:
        sectorIndex = conf->getSectorByDesc("MOUNTAINS");
        break;
    case TERRAIN_SHALLOW:
        sectorIndex = conf->getSectorByDesc("SHALLOW");
        break;
    case TERRAIN_RAPIDS:
        sectorIndex = conf->getSectorByDesc("RAPIDS");
        break;
    case TERRAIN_UNDERWATER:
        sectorIndex = conf->getSectorByDesc("UNDERWATER");
        break;
    default:
        break;
    }

    return (sectorIndex > 0) ? static_cast<char>(sectorIndex) : 0;
}

bool MMapperImport::importFile(const QString &filename, CRoomManager *roomManager, QWidget *parentWidget)
{
    s_lastError.clear();

    // Check file
    uint32_t version = checkFile(filename);
    if (version == 0) {
        return false;
    }

    // Check version support
    if (version < SCHEMA_QCOMPRESS) {
        s_lastError = QString("MMapper file version %1 is too old. Only version %2+ (qCompress) is supported.\n"
                              "Please open this file in MMapper and re-save it to update the format.")
                          .arg(version)
                          .arg(SCHEMA_QCOMPRESS);
        return false;
    }

    if (version > SCHEMA_CURRENT) {
        s_lastError = QString("MMapper file version %1 is newer than supported (%2).\n"
                              "Please update PandoraMapper or use an older MMapper map file.")
                          .arg(version)
                          .arg(SCHEMA_CURRENT);
        return false;
    }

    // Open file
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly)) {
        s_lastError = QString("Cannot open file: %1").arg(filename);
        return false;
    }

    QDataStream stream(&file);

    // Skip magic and version (already read)
    int32_t magic;
    uint32_t ver;
    stream >> magic >> ver;

    // Set Qt4.8 compatibility mode (MMapper uses this)
    stream.setVersion(QDataStream::Qt_4_8);

    // Read remaining compressed data
    QByteArray compressedData = stream.device()->readAll();
    file.close();

    // Decompress
    QByteArray uncompressedData = qUncompress(compressedData);
    if (uncompressedData.isEmpty()) {
        s_lastError = "Failed to decompress MMapper file data";
        return false;
    }

    print_debug(DEBUG_XML, "MMapper file decompressed: %d -> %d bytes", compressedData.size(), uncompressedData.size());

    // Parse decompressed data
    QBuffer buffer(&uncompressedData);
    buffer.open(QIODevice::ReadOnly);
    QDataStream dataStream(&buffer);
    dataStream.setVersion(QDataStream::Qt_4_8);

    // Read header
    uint32_t roomsCount, marksCount;
    int32_t posX, posY, posZ;

    dataStream >> roomsCount >> marksCount;
    dataStream >> posX >> posY >> posZ;

    print_debug(DEBUG_XML, "MMapper file: %u rooms, %u markers, schema v%u", roomsCount, marksCount, version);

    // Check room count limit
    if (roomsCount >= MAX_ROOMS) {
        s_lastError = QString("MMapper file has %1 rooms, but PandoraMapper only supports %2 rooms.\n"
                              "The map is too large to import.")
                          .arg(roomsCount)
                          .arg(MAX_ROOMS);
        return false;
    }

    // Setup progress dialog
    QProgressDialog progress("Importing MMapper database...", "Abort", 0, roomsCount, parentWidget);
    progress.setWindowModality(Qt::ApplicationModal);
    progress.show();

    // Mapping from MMapper room IDs to new sequential IDs (1-based)
    QHash<uint32_t, uint32_t> idMapping;

    // Temporary storage for room data (IDs -> exits mapping)
    struct RoomExitData
    {
        CRoom *room;
        uint32_t mmapperExitIds[6];  // Original MMapper IDs for exits
        bool exitPresent[6];
    };
    QVector<RoomExitData> roomData;
    roomData.reserve(roomsCount);

    // Read rooms
    for (uint32_t i = 0; i < roomsCount; ++i) {
        if (progress.wasCanceled()) {
            s_lastError = "Import canceled by user";
            return false;
        }
        progress.setValue(i);
        QApplication::processEvents();

        CRoom *room = new CRoom();
        RoomExitData exitData;
        exitData.room = room;
        for (int j = 0; j < 6; j++) {
            exitData.mmapperExitIds[j] = 0;
            exitData.exitPresent[j] = false;
        }

        // Read room data based on schema version
        QString area, name, desc, contents, note;

        if (version >= SCHEMA_AREA) {
            dataStream >> area;
        }
        dataStream >> name >> desc >> contents;

        uint32_t mmapperRoomId;
        dataStream >> mmapperRoomId;

        // Assign new sequential ID (1-based, 0 is reserved)
        uint32_t newId = i + 1;
        room->id = newId;
        idMapping[mmapperRoomId] = newId;

        if (version >= SCHEMA_SERVER_ID) {
            uint32_t serverId;
            dataStream >> serverId;
            // serverId is MMapper-specific, we ignore it
        }

        dataStream >> note;

        // Read terrain
        uint8_t terrainType;
        dataStream >> terrainType;

        // Handle death terrain in older schemas
        if (version < SCHEMA_DEATH_FLAG && terrainType == 15) {
            terrainType = TERRAIN_INDOORS;  // Death was terrain 15, now it's a load flag
        }

        // Read MMapper room properties and store them
        uint8_t lightType, alignType, portableType, ridableType, sundeathType;
        dataStream >> lightType >> alignType >> portableType >> ridableType >> sundeathType;

        uint32_t mobFlags, loadFlags;
        dataStream >> mobFlags >> loadFlags;

        // Store MMapper properties in the room
        room->setLightType(lightType);
        room->setAlignType(alignType);
        room->setPortableType(portableType);
        room->setRidableType(ridableType);
        room->setSundeathType(sundeathType);
        room->setMobFlags(mobFlags);
        room->setLoadFlags(loadFlags);

        // Store contents
        room->setContents(contents.toLocal8Bit());

        // Check for deathtrap in loadFlags (bit 24 = DEATHTRAP in newer schema)
        // We'll also mark this in the note for visibility
        bool isDeathtrap = (loadFlags & (1 << 24)) != 0;

        // Read upToDate field (removed in schema v39)
        if (version < SCHEMA_NO_UPTODATE) {
            uint8_t upToDate;
            dataStream >> upToDate;
            // Ignored - MMapper-specific field
        }

        // Read coordinates
        int32_t x, y, z;
        dataStream >> x >> y >> z;

        // Stretch the map to make it look pandora-familiar
        x = x * 2;
        y = y * 2;
        z = z * 2;

        // Convert coordinates (older schemas used ESU, newer use ENU)
        if (version < SCHEMA_NEW_COORDS) {
            y = -y;  // ESU to ENU conversion
        }

        // Set room properties
        room->setName(name.toLocal8Bit());
        room->setDesc(desc.toLocal8Bit());

        QString finalNote = note;
        if (isDeathtrap) {
            if (!finalNote.isEmpty())
                finalNote += " ";
            finalNote += "[DEATHTRAP]";
        }
        room->setNote(finalNote.toLocal8Bit());

        room->setSector(convertTerrain(terrainType));
        room->setX(x);
        room->setY(y);
        room->simpleSetZ(z);

        // Set region - use area if available, otherwise "default"
        // If the region doesn't exist, create it
        QByteArray regionName = area.isEmpty() ? QByteArray("default") : area.toLocal8Bit();
        if (roomManager->getRegionByName(regionName) == nullptr && !regionName.isEmpty() && regionName != "default") {
            roomManager->addRegion(regionName);
        }
        room->setRegion(regionName);

        // Read exits (7 directions in MMapper: NSEWUD + UNKNOWN)
        for (int dir = 0; dir < 7; ++dir) {
            uint16_t exitFlags;
            uint16_t doorFlags;
            QString doorName;

            dataStream >> exitFlags >> doorFlags >> doorName;

            // Read inbound links (older versions)
            if (version < SCHEMA_NO_INBOUND) {
                uint32_t inbound;
                dataStream >> inbound;
                while (inbound != 0xFFFFFFFF) {
                    dataStream >> inbound;
                }
            }

            // Read outbound links
            uint32_t outbound;
            dataStream >> outbound;

            int pandoraDir = convertDirection(dir);
            if (pandoraDir >= 0 && pandoraDir < 6) {
                // Store MMapper exit and door flags
                room->setMMExitFlags(pandoraDir, exitFlags);
                room->setMMDoorFlags(pandoraDir, doorFlags);

                // Store first outbound connection (MMapper ID - will be remapped later)
                if (outbound != 0xFFFFFFFF) {
                    exitData.mmapperExitIds[pandoraDir] = outbound;
                    exitData.exitPresent[pandoraDir] = true;

                    // Set door if present
                    if (!doorName.isEmpty()) {
                        room->setDoor(pandoraDir, doorName.toLocal8Bit());
                    }
                }

                // Skip remaining outbound connections (MMapper supports multiple, Pandora doesn't)
                while (outbound != 0xFFFFFFFF) {
                    dataStream >> outbound;
                }
            } else {
                // Skip outbound for UNKNOWN direction
                while (outbound != 0xFFFFFFFF) {
                    dataStream >> outbound;
                }
            }
        }

        // Check for stream errors
        if (dataStream.status() != QDataStream::Ok) {
            s_lastError = QString("Stream error while reading room %1: status=%2")
                              .arg(i)
                              .arg(dataStream.status());
            // Clean up
            delete room;
            for (int j = 0; j < roomData.size(); ++j) {
                delete roomData[j].room;
            }
            return false;
        }

        roomData.append(exitData);
    }

    buffer.close();

    // Add rooms to manager and resolve exit connections
    progress.setLabelText("Adding rooms to database...");
    progress.setMaximum(roomData.size() * 2);

    for (int i = 0; i < roomData.size(); ++i) {
        progress.setValue(roomsCount + i);
        if (progress.wasCanceled()) {
            s_lastError = "Import canceled by user";
            // Clean up rooms not yet added
            for (int j = i; j < roomData.size(); ++j) {
                delete roomData[j].room;
            }
            return false;
        }

        roomManager->addRoom(roomData[i].room);
    }

    // Resolve exit connections using ID mapping
    progress.setLabelText("Resolving exit connections...");
    for (int i = 0; i < roomData.size(); ++i) {
        progress.setValue(roomsCount + roomData.size() + i);
        QApplication::processEvents();

        CRoom *room = roomData[i].room;
        for (int dir = 0; dir < 6; ++dir) {
            if (roomData[i].exitPresent[dir]) {
                uint32_t mmapperTargetId = roomData[i].mmapperExitIds[dir];
                // Look up the remapped ID
                if (idMapping.contains(mmapperTargetId)) {
                    uint32_t newTargetId = idMapping[mmapperTargetId];
                    CRoom *target = roomManager->getRoom(newTargetId);
                    if (target != nullptr) {
                        room->setExit(dir, target);
                    } else {
                        // Target room not found (shouldn't happen), mark as undefined
                        room->setExitUndefined(dir);
                    }
                } else {
                    // Target room ID not in our mapping (external reference), mark as undefined
                    room->setExitUndefined(dir);
                }
            }
        }
    }

    progress.setValue(progress.maximum());

    print_debug(DEBUG_XML, "MMapper import complete: %d rooms imported", roomData.size());

    return true;
}
