/*
 *  Pandora MUME mapper - Unit Tests
 *
 *  Test runner main file
 */

#include <QTest>

#include "test_utils.h"
#include "test_room.h"

int main(int argc, char *argv[])
{
    int status = 0;

    // Run utility function tests
    {
        TestUtils testUtils;
        status |= QTest::qExec(&testUtils, argc, argv);
    }

    // Run room tests (placeholder for now)
    {
        TestRoom testRoom;
        status |= QTest::qExec(&testRoom, argc, argv);
    }

    return status;
}
