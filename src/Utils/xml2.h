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

#ifndef XML2_H
#define XML2_H

#include <QString>
#include <QXmlStreamReader>

class QProgressDialog;
class CRoomManager;
class CRoom;
class CRegion;

/**
 * XML Parser for PandoraMapper map files.
 *
 * Parses the XML format and builds the room database.
 * Uses a two-pass approach:
 *   1. Parse all rooms (storing exit target IDs temporarily)
 *   2. Resolve exit pointers after all rooms are loaded
 */
class StructureParser
{
public:
    StructureParser(QProgressDialog *progress, unsigned int &currentMaximum, CRoomManager *parent);

    bool parse(QXmlStreamReader &reader);
    bool isAborted() const;

    // Error information
    bool hasError() const { return parseError; }
    QString errorMessage() const { return errorMsg; }
    int errorLine() const { return errorLineNumber; }
    int errorColumn() const { return errorColumnNumber; }

private:
    bool startElement(const QString &qName, const QXmlStreamAttributes &attributes);
    bool endElement(const QString &qName);
    bool characters(const QString &ch);

    // Parent and progress
    CRoomManager *parent;
    QProgressDialog *progress;
    unsigned int &currentMaximum;

    // Parser state
    int flag;                   // Current text content type being parsed
    bool readingRegion;         // Inside a <region> element
    bool abortLoading;          // User canceled loading

    // Current objects being parsed
    CRoom *currentRoom;         // Room currently being parsed
    CRegion *currentRegion;     // Region currently being parsed
    int currentRoomId;          // ID of room being parsed (for error messages)
    QString textBuffer;         // Accumulated text content

    // Error state
    bool parseError;
    QString errorMsg;
    int errorLineNumber;
    int errorColumnNumber;
};

#endif // XML2_H
