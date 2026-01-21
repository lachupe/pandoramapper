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

#ifndef MMAPPERIMPORT_H
#define MMAPPERIMPORT_H

#include <QString>

class CRoomManager;
class QProgressDialog;
class QWidget;

// MMapper .mm2 file importer
// Supports schema versions 34+ (qCompress compression)
// Older zlib-compressed files (v25-33) are not supported to avoid external dependency

class MMapperImport
{
  public:
    // Import an MMapper .mm2 file into the room manager
    // Returns true on success, false on failure
    // parentWidget is used for progress dialog
    static bool importFile(const QString &filename, CRoomManager *roomManager, QWidget *parentWidget = nullptr);

    // Check if a file is a valid MMapper file
    // Returns the schema version or 0 if invalid
    static uint32_t checkFile(const QString &filename);

    // Get error message from last operation
    static QString lastError();

  private:
    static QString s_lastError;

    // Schema version constants
    static constexpr int32_t MMAPPER_MAGIC = static_cast<int32_t>(0xFFB2AF01u);
    static constexpr uint32_t SCHEMA_QCOMPRESS = 34;       // First version with qCompress
    static constexpr uint32_t SCHEMA_NEW_COORDS = 36;     // ENU coordinate system
    static constexpr uint32_t SCHEMA_NO_INBOUND = 38;     // No inbound links
    static constexpr uint32_t SCHEMA_NO_UPTODATE = 39;    // Removed upToDate field
    static constexpr uint32_t SCHEMA_SERVER_ID = 40;      // Has server_id
    static constexpr uint32_t SCHEMA_DEATH_FLAG = 41;     // Death is a room flag, not terrain
    static constexpr uint32_t SCHEMA_AREA = 42;           // Has area field
    static constexpr uint32_t SCHEMA_CURRENT = 42;

    // MMapper terrain types (enum order 0-14)
    enum MMapperTerrain
    {
        TERRAIN_UNDEFINED = 0,
        TERRAIN_INDOORS,
        TERRAIN_CITY,
        TERRAIN_FIELD,
        TERRAIN_FOREST,
        TERRAIN_HILLS,
        TERRAIN_MOUNTAINS,
        TERRAIN_SHALLOW,
        TERRAIN_WATER,
        TERRAIN_RAPIDS,
        TERRAIN_UNDERWATER,
        TERRAIN_ROAD,
        TERRAIN_BRUSH,
        TERRAIN_TUNNEL,
        TERRAIN_CAVERN
    };

    // MMapper direction enum (order differs from PandoraMapper)
    enum MMapperDirection
    {
        DIR_NORTH = 0,
        DIR_SOUTH = 1,
        DIR_EAST = 2,
        DIR_WEST = 3,
        DIR_UP = 4,
        DIR_DOWN = 5,
        DIR_UNKNOWN = 6
    };

    // Convert MMapper direction to PandoraMapper direction
    static int convertDirection(int mmapperDir);

    // Convert MMapper terrain to PandoraMapper sector
    static char convertTerrain(uint8_t mmapperTerrain);
};

#endif // MMAPPERIMPORT_H
