/*
 *  Pandora MUME mapper - Unit Tests
 *
 *  Tests for utility functions
 */

#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include <QObject>
#include <QTest>

class TestUtils : public QObject
{
    Q_OBJECT

private slots:
    // Test bit manipulation functions
    void testIsSet();
    void testSetBit();
    void testRemoveBit();
    void testToggleBit();

    // Test character conversion functions
    void testToLower();
    void testToUpper();

    // Test string helper functions
    void testOnOff();
    void testYesNo();

    // Test utility functions
    void testIsInteger();
    void testSkipSpaces();
    void testIsAbbrev();
    void testReverseNum();
    void testDirByNum();
    void testNumByDir();
};

#endif // TEST_UTILS_H
