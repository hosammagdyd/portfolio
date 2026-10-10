#pragma once

#include <QList>
#include <QObject>
#include <QThread>
#include <qqmlintegration.h>

class DeviceListModel;
class DeviceSimulatorWorker;

// Main-thread controller. Where the platform supports real threads, it owns
// a QThread and moves DeviceSimulatorWorker onto it, so all data-generation
// work happens off the UI thread; every crossing between this object and the
// worker goes through a queued signal/slot connection (Qt resolves
// AutoConnection to Queued here since the two objects live on different
// threads). On platforms without thread support (Qt for WebAssembly's
// single-threaded configuration, used deliberately in this project to avoid
// COOP/COEP headers), the same worker runs cooperatively on the main thread
// instead — see devicesimulator.cpp. Either way, this object is the only
// thing allowed to touch Device/DeviceListModel, and only from slots that
// Qt guarantees run on the main thread.
class DeviceSimulator : public QObject
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(DeviceListModel *model READ model WRITE setModel NOTIFY modelChanged)
    Q_PROPERTY(bool running READ isRunning WRITE setRunning NOTIFY runningChanged)

public:
    explicit DeviceSimulator(QObject *parent = nullptr);
    ~DeviceSimulator() override;

    DeviceListModel *model() const;
    void setModel(DeviceListModel *model);

    bool isRunning() const;
    void setRunning(bool running);

signals:
    void modelChanged();
    void runningChanged();

    // Crosses to the worker thread.
    void requestInitialize(QList<double> initialTemperatures);
    void requestStart();
    void requestStop();

private slots:
    // Invoked (via queued connection) on the main thread when the worker
    // has new data — safe to apply directly to Device here.
    void applyTelemetry(int row, double temperature, int signalStrength);
    void applyWaveform(int row, double pressure, double flow, double volume);

private:
    DeviceListModel *m_model = nullptr;
    QThread m_workerThread;
    DeviceSimulatorWorker *m_worker = nullptr;
    bool m_running = false;
};
