
#include "mmapper_importer.h"
#include "defines.h"
#include "Map/CRoomManager.h"
#include <QFile>

#include "coordinate.h"
#include "qtiocompressor.h"


bool CRoomManager::loadMMapperMap(QString filename)
{

    MapBlocker blocker(*this);

    // clear current database
    reinit();


    //baseId = m_mapData.getMaxId() + 1;
    Coordinate basePosition;
    basePosition.y = 0;
    basePosition.x = 0;
    basePosition.z = 0;

    QFile m_file;

    m_file.setFileName(filename);
    m_file.open(QIODevice::ReadOnly);
    if ( !m_file.open( QFile::ReadOnly ) )
        throw std::runtime_error( m_file.errorString().toStdString() );

    QDataStream stream(&m_file);

    //Room *room = NULL;
    //InfoMark *mark = NULL;

    quint32 magic;
    qint32 version;
    quint32 index = 0;
    quint32 roomsCount = 0;
    quint32 connectionsCount = 0;
    quint32 marksCount = 0;

    //m_mapData.setDataChanged();

    // Read the version and magic
    stream >> magic;
    if ( magic != 0xFFB2AF01 )
        return false;

    stream >> version;
    if ( version != 031 && version != 030 &&
         version != 020 && version != 021 && version != 007 )
        return false;

    // QtIOCompressor
    if (version >= 031) {
        QtIOCompressor *m_compressor = new QtIOCompressor(&m_file);
        stream.setDevice(m_compressor);
    }

    stream >> roomsCount;
    if (version < 020) stream >> connectionsCount;
    stream >> marksCount;

    //m_progressCounter->increaseTotalStepsBy( roomsCount + connectionsCount + marksCount );

    Coordinate pos;

    //read selected room x,y
    if (version < 020) // OLD VERSIONS SUPPORT CODE
    {
        stream >> (quint32 &)pos.x;
        stream >> (quint32 &)pos.y;
    }
    else
    {
        stream >> (qint32 &)pos.x;
        stream >> (qint32 &)pos.y;
        stream >> (qint32 &)pos.z;
    }

    pos += basePosition;

    //m_mapData.setPosition(pos);
    //emit log ("MapStorage", QString("Number of rooms: %1").arg(roomsCount) );

    //    ConnectionList connectionList;
    //    // create all pointers to connections
    //    for(index = 0; index<connectionsCount; index++)
    //    {
    //        Connection * connection = new Connection;
    //        connectionList.append(connection);
    //    }

    for (uint i = 0; i < roomsCount; ++i)
    {
        QString vqba;
        quint32 vquint32;
        quint8 vquint8;
        quint16 vquint16;

        Room * room = factory.createRoom();
        room->setPermanent();

        stream >> vqba;
        room->replace(R_NAME, vqba);
        stream >> vqba;
        room->replace(R_DESC, vqba);
        stream >> vqba;
        room->replace(R_DYNAMICDESC, vqba);
        stream >> vquint32; room->setId(vquint32+baseId);
        stream >> vqba;
        room->replace(R_NOTE, vqba);
        stream >> vquint8;
        room->replace(R_TERRAINTYPE, vquint8);
        stream >> vquint8; room->replace(R_LIGHTTYPE, vquint8);
        stream >> vquint8; room->replace(R_ALIGNTYPE, vquint8);
        stream >> vquint8;
        room->replace(R_PORTABLETYPE, vquint8);
        if (version >= 030)
            stream >> vquint8;
        else
            vquint8 = 0;
        room->replace(R_RIDABLETYPE, vquint8);
        stream >> vquint16; room->replace(R_MOBFLAGS, vquint16);
        stream >> vquint16; room->replace(R_LOADFLAGS, vquint16);

        stream >> vquint8; //roomUpdated
        if (vquint8)
        {
            room->setUpToDate();
        }
        Coordinate c;
        stream >> (qint32 &)c.x;
        stream >> (qint32 &)c.y;
        stream >> (qint32 &)c.z;

        room->setPosition(c + basePosition);
        ExitsList & eList = room->getExitsList();
        for (int i = 0; i < 7; ++i)
        {
            Exit & e = eList[i];

            ExitFlags flags;
            stream >> flags;
            if (ISSET(flags, EF_DOOR)) SET(flags, EF_EXIT);
            e[E_FLAGS] = flags;

            DoorFlags dFlags;
            stream >> dFlags;
            e[E_DOORFLAGS] = dFlags;

            DoorName dName;
            stream >> dName;
            e[E_DOORNAME] = dName;

            quint32 connection;
            for (stream >> connection; connection != UINT_MAX; stream >> connection)
            {
                e.addIn(connection+baseId);
            }
            for (stream >> connection; connection != UINT_MAX; stream >> connection)
            {
                e.addOut(connection+baseId);
            }
        }
        //m_mapData.insertPredefinedRoom(room);
    }

    //    MarkerList& markerList = m_mapData.getMarkersList();
    //    // create all pointers to items
    //    for(index = 0; index<marksCount; index++)
    //    {
    //        mark = new InfoMark();
    //        loadMark(mark, stream, version);
    //        markerList.append(mark);

    //        m_progressCounter->step();
    //    }


    if (version >= 031)
        m_compressor->close();

    return true;
}




void MapStorage::loadMark(InfoMark * mark, QDataStream & stream, qint32 version)
{
    //    QString vqstr;
    //    quint16 vquint16;
    //    quint32 vquint32;
    //    qint32  vqint32;

    //    qint32 postfix = basePosition.x + basePosition.y + basePosition.z;

    //    if (version < 020) // OLD VERSIONS SUPPORT CODE
    //    {
    //        stream >> vqstr;
    //        if (postfix != 0 && postfix != 1)
    //        {
    //            vqstr += QString("_m%1").arg(postfix);
    //        }
    //        mark->setName(vqstr);
    //        stream >> vqstr; mark->setText(vqstr);
    //        stream >> vquint16; mark->setType((InfoMarkType)vquint16);

    //        Coordinate pos;
    //        stream >> vquint32; pos.x = (qint32) vquint32;
    //        pos.x = pos.x*100/48-40;
    //        stream >> vquint32; pos.y = (qint32) vquint32;
    //        pos.y = pos.y*100/48-55;
    //        //pos += basePosition;
    //        pos.x += basePosition.x*100;
    //        pos.y += basePosition.y*100;
    //        pos.z += basePosition.z;
    //        mark->setPosition1(pos);

    //        stream >> vquint32; pos.x = (qint32) vquint32;
    //        pos.x = pos.x*100/48-40;
    //        stream >> vquint32; pos.y = (qint32) vquint32;
    //        pos.y = pos.y*100/48-55;
    //        //pos += basePosition;
    //        pos.x += basePosition.x*100;
    //        pos.y += basePosition.y*100;
    //        pos.z += basePosition.z;
    //        mark->setPosition2(pos);
    //    }
    //    else
    //    {
    //        QString vqba;
    //        QDateTime vdatetime;
    //        quint8 vquint8;

    //        stream >> vqba;
    //        if (postfix != 0 && postfix != 1)
    //        {
    //            vqba += QString("_m%1").arg(postfix).toAscii();
    //        }
    //        mark->setName(vqba);
    //        stream >> vqba; mark->setText(vqba);
    //        stream >> vdatetime; mark->setTimeStamp(vdatetime);
    //        stream >> vquint8; mark->setType((InfoMarkType)vquint8);

    //        Coordinate pos;
    //        stream >> vqint32; pos.x = vqint32/*-40*/;
    //        stream >> vqint32; pos.y = vqint32/*-55*/;
    //        stream >> vqint32; pos.z = vqint32;
    //        pos.x += basePosition.x*100;
    //        pos.y += basePosition.y*100;
    //        pos.z += basePosition.z;
    //        //pos += basePosition;
    //        mark->setPosition1(pos);

    //        stream >> vqint32; pos.x = vqint32/*-40*/;
    //        stream >> vqint32; pos.y = vqint32/*-55*/;
    //        stream >> vqint32; pos.z = vqint32;
    //        pos.x += basePosition.x*100;
    //        pos.y += basePosition.y*100;
    //        pos.z += basePosition.z;
    //        //pos += basePosition;
    //        mark->setPosition2(pos);
    //    }
}
