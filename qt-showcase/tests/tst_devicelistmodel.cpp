#include <QtTest>

#include "device.h"
#include "devicelistmodel.h"

class TestDeviceListModel : public QObject
{
    Q_OBJECT

private slots:
    void seedsSampleDevices();
    void exposesDeviceRole();
    void statusFollowsTemperatureThresholds();
};

void TestDeviceListModel::seedsSampleDevices()
{
    DeviceListModel model;
    QVERIFY(model.rowCount() > 0);
}

void TestDeviceListModel::exposesDeviceRole()
{
    DeviceListModel model;
    const QModelIndex index = model.index(0, 0);
    const QVariant value = model.data(index, DeviceListModel::DeviceRole);

    auto *device = qobject_cast<Device *>(qvariant_cast<QObject *>(value));
    QVERIFY(device != nullptr);
    QVERIFY(!device->name().isEmpty());
}

void TestDeviceListModel::statusFollowsTemperatureThresholds()
{
    DeviceListModel model;
    Device *device = model.deviceAt(0);
    QVERIFY(device != nullptr);

    device->setTemperature(30.0);
    QTest::qWait(50);
    QCOMPARE(device->statusText(), QStringLiteral("normal"));

    device->setTemperature(60.0);
    QTest::qWait(50);
    QCOMPARE(device->statusText(), QStringLiteral("warning"));

    device->setTemperature(90.0);
    QTest::qWait(50);
    QCOMPARE(device->statusText(), QStringLiteral("critical"));

    device->setTemperature(40.0);
    QTest::qWait(50);
    QCOMPARE(device->statusText(), QStringLiteral("normal"));
}

QTEST_MAIN(TestDeviceListModel)
#include "tst_devicelistmodel.moc"
