/*
 *  Pandora MUME mapper - Unit Tests
 *
 *  Tests for utility functions
 */

#include "test_utils.h"
#include "utils.h"

void TestUtils::testIsSet()
{
    unsigned int flags = 0;
    const unsigned int BIT_A = (1 << 0);
    const unsigned int BIT_B = (1 << 1);
    const unsigned int BIT_C = (1 << 2);

    // Initially no bits set
    QVERIFY(!isSet(flags, BIT_A));
    QVERIFY(!isSet(flags, BIT_B));
    QVERIFY(!isSet(flags, BIT_C));

    // Set some bits and verify
    flags = BIT_A | BIT_C;
    QVERIFY(isSet(flags, BIT_A));
    QVERIFY(!isSet(flags, BIT_B));
    QVERIFY(isSet(flags, BIT_C));
}

void TestUtils::testSetBit()
{
    unsigned int flags = 0;
    const unsigned int BIT_A = (1 << 0);
    const unsigned int BIT_B = (1 << 1);

    setBit(flags, BIT_A);
    QCOMPARE(flags, BIT_A);

    setBit(flags, BIT_B);
    QCOMPARE(flags, BIT_A | BIT_B);

    // Setting already set bit should have no effect
    setBit(flags, BIT_A);
    QCOMPARE(flags, BIT_A | BIT_B);
}

void TestUtils::testRemoveBit()
{
    const unsigned int BIT_A = (1 << 0);
    const unsigned int BIT_B = (1 << 1);
    unsigned int flags = BIT_A | BIT_B;

    removeBit(flags, BIT_A);
    QCOMPARE(flags, BIT_B);

    removeBit(flags, BIT_B);
    QCOMPARE(flags, 0u);

    // Removing already removed bit should have no effect
    removeBit(flags, BIT_A);
    QCOMPARE(flags, 0u);
}

void TestUtils::testToggleBit()
{
    const unsigned int BIT_A = (1 << 0);
    unsigned int flags = 0;

    toggleBit(flags, BIT_A);
    QCOMPARE(flags, BIT_A);

    toggleBit(flags, BIT_A);
    QCOMPARE(flags, 0u);

    toggleBit(flags, BIT_A);
    QCOMPARE(flags, BIT_A);
}

void TestUtils::testToLower()
{
    QCOMPARE(toLower('A'), 'a');
    QCOMPARE(toLower('Z'), 'z');
    QCOMPARE(toLower('a'), 'a');
    QCOMPARE(toLower('z'), 'z');
    QCOMPARE(toLower('0'), '0');
    QCOMPARE(toLower(' '), ' ');
}

void TestUtils::testToUpper()
{
    QCOMPARE(toUpper('a'), 'A');
    QCOMPARE(toUpper('z'), 'Z');
    QCOMPARE(toUpper('A'), 'A');
    QCOMPARE(toUpper('Z'), 'Z');
    QCOMPARE(toUpper('0'), '0');
    QCOMPARE(toUpper(' '), ' ');
}

void TestUtils::testOnOff()
{
    QCOMPARE(QString(onOff(true)), QString("ON"));
    QCOMPARE(QString(onOff(false)), QString("OFF"));
}

void TestUtils::testYesNo()
{
    QCOMPARE(QString(yesNo(true)), QString("YES"));
    QCOMPARE(QString(yesNo(false)), QString("NO"));
}

void TestUtils::testIsInteger()
{
    char positive[] = "123";
    char negative[] = "-456";
    char invalid[] = "12a3";
    char empty[] = "";

    QCOMPARE(is_integer(positive), 1);
    QCOMPARE(is_integer(negative), 1);
    QCOMPARE(is_integer(invalid), 0);
    QCOMPARE(is_integer(empty), 0);
}

void TestUtils::testSkipSpaces()
{
    const char *str1 = "  hello";
    const char *str2 = "hello";
    const char *str3 = "   ";

    QCOMPARE(QString(skip_spaces(str1)), QString("hello"));
    QCOMPARE(QString(skip_spaces(str2)), QString("hello"));
    QCOMPARE(QString(skip_spaces(str3)), QString(""));
}

void TestUtils::testIsAbbrev()
{
    QCOMPARE(is_abbrev("n", "north"), 1);
    QCOMPARE(is_abbrev("no", "north"), 1);
    QCOMPARE(is_abbrev("nor", "north"), 1);
    QCOMPARE(is_abbrev("north", "north"), 1);
    QCOMPARE(is_abbrev("northe", "north"), 0);
    QCOMPARE(is_abbrev("s", "north"), 0);
}

void TestUtils::testReverseNum()
{
    // North <-> South
    QCOMPARE(reversenum(0), 2);  // NORTH -> SOUTH
    QCOMPARE(reversenum(2), 0);  // SOUTH -> NORTH
    // East <-> West
    QCOMPARE(reversenum(1), 3);  // EAST -> WEST
    QCOMPARE(reversenum(3), 1);  // WEST -> EAST
    // Up <-> Down
    QCOMPARE(reversenum(4), 5);  // UP -> DOWN
    QCOMPARE(reversenum(5), 4);  // DOWN -> UP
}

void TestUtils::testDirByNum()
{
    QCOMPARE(dirbynum(0), 'n');  // NORTH
    QCOMPARE(dirbynum(1), 'e');  // EAST
    QCOMPARE(dirbynum(2), 's');  // SOUTH
    QCOMPARE(dirbynum(3), 'w');  // WEST
    QCOMPARE(dirbynum(4), 'u');  // UP
    QCOMPARE(dirbynum(5), 'd');  // DOWN
}

void TestUtils::testNumByDir()
{
    QCOMPARE(numbydir('n'), 0);  // NORTH
    QCOMPARE(numbydir('e'), 1);  // EAST
    QCOMPARE(numbydir('s'), 2);  // SOUTH
    QCOMPARE(numbydir('w'), 3);  // WEST
    QCOMPARE(numbydir('u'), 4);  // UP
    QCOMPARE(numbydir('d'), 5);  // DOWN
}
