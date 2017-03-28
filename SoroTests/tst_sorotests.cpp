#include <QString>
#include <QtTest>
#include <QSignalSpy>

#include "libsoro/sensordataparser.h"

using namespace Soro;

class SoroTests : public QObject
{
    Q_OBJECT

public:
    SoroTests();

private Q_SLOTS:
    void testSensorDataRecorder();
};

SoroTests::SoroTests()
{
}

void SoroTests::testSensorDataRecorder()
{
    SensorDataParser recorder;
    QSignalSpy spy(&recorder, &SensorDataParser::dataParsed);
    QSignalSpy spyErr(&recorder, &SensorDataParser::parseError);
    QList<QVariant> args;

    /* Test simple data
     */
    recorder.newData("A123B456C789", strlen("A123B456C789"));
    QVERIFY(spy.count() == 3);
    QVERIFY(spyErr.count() == 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'A');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'B');
    QVERIFY(args.at(1).toInt() == 456);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'C');
    QVERIFY(args.at(1).toInt() == 789);

    /* Test complex data
     */
    recorder.newData("X123Y456Z", strlen("X123Y456Z"));
    recorder.newData("789P1", strlen("789P1"));
    recorder.newData("23", strlen("23"));

    QVERIFY(spy.count() == 4);
    QVERIFY(spyErr.count() == 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'X');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'Y');
    QVERIFY(args.at(1).toInt() == 456);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'Z');
    QVERIFY(args.at(1).toInt() == 789);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'P');
    QVERIFY(args.at(1).toInt() == 123);

    /* Test invalid data
     */
    recorder.newData("fweanguewio", strlen("fweanguewio"));
    QVERIFY(spy.count() == 0);
    QVERIFY(spyErr.count() > 0);

    spyErr.clear();

    recorder.newData("X123456YZ45P789", strlen("X123456YZ45P789"));
    QVERIFY(spy.count() == 2);
    QVERIFY(spyErr.count() > 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'X');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0) == 'P');
    QVERIFY(args.at(1) == 789);

    spyErr.clear();
}

QTEST_GUILESS_MAIN(SoroTests)

#include "tst_sorotests.moc"
