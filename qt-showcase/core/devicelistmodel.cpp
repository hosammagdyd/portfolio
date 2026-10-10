#include "devicelistmodel.h"

namespace {

struct DeviceSeed
{
    QString id;
    QString name;
    double temperature;
    int signalStrength;
};

// Status is derived automatically by each Device's internal state machine
// based on these starting temperatures (see Device::setupStateMachine).
const QList<DeviceSeed> kSeeds = {
    {"vent-01", "PulmoFlow 700 — Unit 01", 36.4, 92},
    {"vent-02", "PulmoFlow 700 — Unit 02", 58.1, 74},
    {"vent-03", "PulmoFlow 700 — Unit 03", 29.8, 98},
    {"vent-04", "PulmoFlow 700 — Unit 04", 81.6, 41},
    {"vent-05", "PulmoFlow 700 — Unit 05", 34.9, 88},
};

} // namespace

DeviceListModel::DeviceListModel(QObject *parent)
    : QAbstractListModel(parent)
{
    m_devices.reserve(kSeeds.size());
    for (const auto &seed : kSeeds) {
        auto *device = new Device(seed.id, seed.name, this);
        device->setTemperature(seed.temperature);
        device->setSignalStrength(seed.signalStrength);
        m_devices.append(device);
    }
}

int DeviceListModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_devices.size();
}

QVariant DeviceListModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0 || index.row() >= m_devices.size())
        return {};

    if (role == DeviceRole)
        return QVariant::fromValue(static_cast<QObject *>(m_devices.at(index.row())));

    return {};
}

QHash<int, QByteArray> DeviceListModel::roleNames() const
{
    return {{DeviceRole, "device"}};
}

Device *DeviceListModel::deviceAt(int index) const
{
    if (index < 0 || index >= m_devices.size())
        return nullptr;
    return m_devices.at(index);
}
