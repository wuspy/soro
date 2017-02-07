#include <QString>
#include <QtTest>

#include "libsoro/sensordatarecorder.h"

using namespace Soro;
using namespace Soro::MissionControl;
using namespace Soro::Rover;

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
    recorder.newData("+!123&", strlen("+!123&"));


    QVERIFY2(true, "Failure");
}

QTEST_APPLESS_MAIN(SoroTests)

#include "tst_sorotests.moc"
