#include <QString>
#include <QtTest>
#include <QSignalSpy>

#include "libsoro/sensordatarecorder.h"

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
    SensorDataRecorder recorder;
    QSignalSpy spy(&recorder, &SensorDataRecorder::dataParsed);
    QSignalSpy spyErr(&recorder, &SensorDataRecorder::parseError);
    QList<QVariant> args;

    /* Test simple data
     */
    recorder.newData("a123b456c789", strlen("a123b456c789"));
    QVERIFY(spy.count() == 3);
    QVERIFY(spyErr.count() == 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'a');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'b');
    QVERIFY(args.at(1).toInt() == 456);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'c');
    QVERIFY(args.at(1).toInt() == 789);

    /* Test complex data
     */
    recorder.newData("x123y456z", strlen("x123y456z"));
    recorder.newData("789p1", strlen("789p1"));
    recorder.newData("23", strlen("23"));

    QVERIFY(spy.count() == 4);
    QVERIFY(spyErr.count() == 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'x');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'y');
    QVERIFY(args.at(1).toInt() == 456);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'z');
    QVERIFY(args.at(1).toInt() == 789);
    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'p');
    QVERIFY(args.at(1).toInt() == 123);

    /* Test invalid data
     */
    recorder.newData("fweanguewio", strlen("fweanguewio"));
    QVERIFY(spy.count() == 0);
    QVERIFY(spyErr.count() > 0);

    spyErr.clear();

    recorder.newData("x123456yz45p789", strlen("x123456yz45p789"));
    QVERIFY(spy.count() == 2);
    QVERIFY(spyErr.count() > 0);

    args = spy.takeFirst();
    QVERIFY(args.at(0).toChar() == 'x');
    QVERIFY(args.at(1).toInt() == 123);
    args = spy.takeFirst();
    QVERIFY(args.at(0) == 'p');
    QVERIFY(args.at(1) == 789);

    spyErr.clear();
}

QTEST_GUILESS_MAIN(SoroTests)

#include "tst_sorotests.moc"
