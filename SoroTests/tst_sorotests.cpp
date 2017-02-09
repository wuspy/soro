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
    qRegisterMetaType<SensorDataRecorder::DataTag>("SensorDataRecorder::DataTag");
}

void SoroTests::testSensorDataRecorder()
{
    SensorDataRecorder recorder;
    QSignalSpy spy(&recorder, &SensorDataRecorder::newData);

    /* Test simple data
     */
    recorder.newData("+!123&", strlen("+!123&"));
    if (spy.count() < 1) {
        QVERIFY2(spy.wait(1000), "dataParsed signal was not emitted");
    }

    QList<QVariant> args = spy.takeFirst();
    qint32 arg1 = args.at(0).toInt();
    float arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_IMUDATA_1_X,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 123, "Wrong value was emitted");

    /* Test complex data
     */
    recorder.newData("@123&~#3421&+!124", strlen("@123&~#3421&+!124"));
    recorder.newData("4&^1235", strlen("4&!1235"));
    recorder.newData("&", strlen("&"));
    while (spy.count() < 4) {
        QVERIFY2(spy.wait(1000), "dataParsed signal was not emitted");
    }

    args = spy.takeFirst();
    arg1 = args.at(0).toInt();
    arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_WHEELDATA_2,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 123, "Wrong value was emitted");

    args = spy.takeFirst();
    arg1 = args.at(0).toInt();
    arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_IMUDATA_2_Z,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 3421, "Wrong value was emitted");

    args = spy.takeFirst();
    arg1 = args.at(0).toInt();
    arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_IMUDATA_1_X,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 1244, "Wrong value was emitted");

    args = spy.takeFirst();
    arg1 = args.at(0).toInt();
    arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_WHEELDATA_6,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 1235, "Wrong value was emitted");

    /* Test invalid data
     */
    recorder.newData("(123&~#34A21$45&+!124&", strlen("(123&~#34A21$45&+!124&"));
    if (spy.count() < 1) {
        QVERIFY2(spy.wait(1000), "dataParsed signal was not emitted");
    }
    args = spy.takeFirst();
    arg1 = args.at(0).toInt();
    arg2 = args.at(1).toFloat();
    QVERIFY2(reinterpret_cast<const SensorDataRecorder::DataTag&>(arg1) == SensorDataRecorder::DATATAG_IMUDATA_1_X,
             "Data tag is incorrect");
    QVERIFY2(arg2 == 124, "Wrong value was emitted");
}

QTEST_GUILESS_MAIN(SoroTests)

#include "tst_sorotests.moc"
