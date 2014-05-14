
#include "mmapper_importer.h"
#include "defines.h"
#include "Map/CRoomManager.h"
#include <QFile>

//#include "coordinate.h"
#include "qtiocompressor.h"


#include "Utils/utils.h"

bool CRoomManager::loadMMapperMap(QString filename)
{
    MapBlocker blocker(*this);

    // clear current database
    reinit();


    //baseId = m_mapData.getMaxId() + 1;
//    Coordinate basePosition;
//    basePosition.y = 0;
//    basePosition.x = 0;
//    basePosition.z = 0;

    QFile m_file;

    m_file.setFileName(filename);
    if ( !m_file.open( QIODevice::ReadOnly ) )
        throw std::runtime_error( m_file.errorString().toStdString() );

    QtIOCompressor *m_compressor = new QtIOCompressor(&m_file);


    QDataStream stream(&m_file);

    //Room *room = NULL;
    //InfoMark *mark = NULL;

    quint32 magic;
    qint32 version;
    quint32 index = 0;
    quint32 roomsCount = 0;
    quint32 connectionsCount = 0;
    quint32 marksCount = 0;
    qint32  coord;


    // Read the version and magic

    stream >> magic;
    if ( magic != 0xFFB2AF01 )
        throw std::runtime_error("Does not look like a mmapper file");


    stream >> version;
    if ( version != 031 && version != 030 )
        throw std::runtime_error("This format version is not supported. Only newer versions of mmapper maps are supported.");

    // QtIOCompressor
    if (version >= 031) {
        printf("turning decompression on!\r\n");
        m_compressor->open(QIODevice::ReadOnly);
        stream.setDevice(m_compressor);
    }

    stream >> roomsCount;
    if (version < 020)
        stream >> connectionsCount;
    stream >> marksCount;

    //m_progressCounter->increaseTotalStepsBy( roomsCount + connectionsCount + marksCount );

    //read selected room x,y
    if (version < 020) // OLD VERSIONS SUPPORT CODE
    {
        stream >> (quint32 &)coord;
        stream >> (quint32 &)coord;
    }
    else
    {
        stream >> (qint32 &)coord;
        stream >> (qint32 &)coord;
        stream >> (qint32 &)coord;
    }


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

        CRoom * room = new CRoom(this);

        stream >> vqba;
        room->setName(vqba.toLocal8Bit());
        stream >> vqba;
        room->setDesc(vqba.toLocal8Bit());
        stream >> vqba;
        room->setDynamicDesc(vqba.toLocal8Bit());
        stream >> vquint32;
        room->setId(vquint32);

        stream >> vqba;
        room->setNote(vqba.toLocal8Bit());
        stream >> vquint8;
        room->setSector(static_cast<RoomTerrainType>(vquint8));
        stream >> vquint8;
        room->setLightType(static_cast<RoomLightType>(vquint8));
        stream >> vquint8;
        room->setAlignType(static_cast<RoomAlignType>(vquint8));
        stream >> vquint8;
        room->setPortableType(static_cast<RoomPortableType>(vquint8));
        if (version >= 030)
            stream >> vquint8;
        else
            vquint8 = 0;
        room->setRidableType(static_cast<RoomRidableType>(vquint8));

        stream >> vquint16; // MOBFLAGS
        // test bits and set appropriate flags in appropriate way
        if (IS_SET(vquint16, bit1))
            room->setMobFlag(RMF_RENT, true);
        if (IS_SET(vquint16, bit2))
            room->setMobFlag(RMF_SHOP, true);
        if (IS_SET(vquint16, bit3))
            room->setMobFlag(RMF_WEAPONSHOP, true);
        if (IS_SET(vquint16, bit4))
            room->setMobFlag(RMF_ARMOURSHOP, true);
        if (IS_SET(vquint16, bit5))
            room->setMobFlag(RMF_FOODSHOP, true);
        if (IS_SET(vquint16, bit6))
            room->setMobFlag(RMF_PETSHOP, true);
        if (IS_SET(vquint16, bit7))
            room->setMobFlag(RMF_GUILD, true);
        if (IS_SET(vquint16, bit8))
            room->setMobFlag(RMF_SCOUTGUILD, true);
        if (IS_SET(vquint16, bit9))
            room->setMobFlag(RMF_MAGEGUILD, true);
        if (IS_SET(vquint16, bit10))
            room->setMobFlag(RMF_CLERICGUILD, true);
        if (IS_SET(vquint16, bit11))
            room->setMobFlag(RMF_WARRIORGUILD, true);
        if (IS_SET(vquint16, bit12))
            room->setMobFlag(RMF_RANGERGUILD, true);
        if (IS_SET(vquint16, bit13))
            room->setMobFlag(RMF_SMOB, true);
        if (IS_SET(vquint16, bit14))
            room->setMobFlag(RMF_QUEST, true);
        if (IS_SET(vquint16, bit15))
            room->setMobFlag(RMF_ANY, true);


        stream >> vquint16; // loadflags
        if (IS_SET(vquint16, bit1))
            room->setLoadFlag(RLF_TREASURE, true);
        if (IS_SET(vquint16, bit2))
            room->setLoadFlag(RLF_ARMOUR, true);
        if (IS_SET(vquint16, bit3))
            room->setLoadFlag(RLF_WEAPON, true);
        if (IS_SET(vquint16, bit4))
            room->setLoadFlag(RLF_WATER, true);
        if (IS_SET(vquint16, bit5))
            room->setLoadFlag(RLF_FOOD, true);
        if (IS_SET(vquint16, bit6))
            room->setLoadFlag(RLF_HERB, true);
        if (IS_SET(vquint16, bit7))
            room->setLoadFlag(RLF_KEY, true);
        if (IS_SET(vquint16, bit8))
            room->setLoadFlag(RLF_MULE, true);
        if (IS_SET(vquint16, bit9))
            room->setLoadFlag(RLF_HORSE, true);
        if (IS_SET(vquint16, bit10))
            room->setLoadFlag(RLF_PACKHORSE, true);
        if (IS_SET(vquint16, bit11))
            room->setLoadFlag(RLF_TRAINEDHORSE, true);
        if (IS_SET(vquint16, bit12))
            room->setLoadFlag(RLF_ROHIRRIM, true);
        if (IS_SET(vquint16, bit13))
            room->setLoadFlag(RLF_WARG, true);
        if (IS_SET(vquint16, bit14))
            room->setLoadFlag(RLF_BOAT, true);
        if (IS_SET(vquint16, bit15))
            room->setLoadFlag(RLF_ATTENTION, true);
        if (IS_SET(vquint16, bit16))
            room->setLoadFlag(RLF_TOWER, true);

        stream >> vquint8; //roomUpdated ... ignored

        // coordinates
        stream >> (qint32 &)coord;
        room->setX( coord * 2);

        stream >> (qint32 &)coord;
        room->setY( coord * -2 );

        stream >> (qint32 &)coord;
        room->setZ( coord * 2);

        for (int i = 0; i < 7; ++i)
        {
            ExitDirection dir = static_cast<ExitDirection>(i);

            quint8 flags;
            stream >> flags; // exit flags

            if (IS_SET(flags, bit1))
                room->setExitUndefined(dir);

            if (IS_SET(flags, bit3))
                room->setExitFlag(dir, EF_ROAD, true);
            if (IS_SET(flags, bit4))
                room->setExitFlag(dir, EF_CLIMB, true);
            if (IS_SET(flags, bit5))
                room->setExitFlag(dir, EF_RANDOM, true);
            if (IS_SET(flags, bit6))
                room->setExitFlag(dir, EF_SPECIAL, true);
            if (IS_SET(flags, bit7))
                room->setExitFlag(dir, EF_NO_MATCH, true);

            stream >> flags; // Door flags
            // EF_HIDDEN is irrelevant
            if (IS_SET(flags, bit2))
                room->setDoorFlag(dir, DF_NEEDKEY, true);
            if (IS_SET(flags, bit3))
                room->setDoorFlag(dir, DF_NOBLOCK, true);
            if (IS_SET(flags, bit4))
                room->setDoorFlag(dir, DF_NOBREAK, true);
            if (IS_SET(flags, bit5))
                room->setDoorFlag(dir, DF_NOPICK, true);
            if (IS_SET(flags, bit6))
                room->setDoorFlag(dir, DF_DELAYED, true);


            QString dName;
            stream >> dName;
            room->setDoor(dir, dName.toLocal8Bit());

            quint32 connection;
            for (stream >> connection; connection != UINT_MAX; stream >> connection)
            {
                // incoming connections are simply ignored - they are outgoing in their own rooms!
                //e.addIn(connection+baseId);
            }
            for (stream >> connection; connection != UINT_MAX; stream >> connection)
            {
                // only the last outgoing connection makes the difference...
                room->setExitLeadsTo(dir, connection);
                //e.addOut(connection+baseId);
            }

        }

        addRoom(room);
        //m_mapData.insertPredefinedRoom(room);
    }

    //    MarkerList& markerList = m_mapData.getMarkersList();
    //    // create all pointers to items
    //    for(index = 0; index<marksCount; index++)
    //    {
    //        mark = new InfoMark();


    //        loadMark(mark, stream, version);

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

    //        markerList.append(mark);

    //        m_progressCounter->step();
    //    }


    if (version >= 031)
        m_compressor->close();


    return true;
}




