#pragma once

#include <QAbstractListModel>
#include <qqmlintegration.h>

#include "device.h"

class DeviceListModel : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT

public:
    enum Roles {
        DeviceRole = Qt::UserRole + 1,
    };

    explicit DeviceListModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    Q_INVOKABLE Device *deviceAt(int index) const;

private:
    QList<Device *> m_devices;
};
