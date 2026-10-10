#pragma once

#include <QList>
#include <QObject>
#include <QTimer>
#include <QVector>

// Runs entirely on a worker thread (see DeviceSimulator, which owns a
// QThread and moves this object onto it). Pure computation only — it never
// touches a Device/DeviceListModel directly, since those are QObjects that
// live on the main thread. Results cross back via queued signals, and
// inputs arrive the same way, so there is no shared mutable state between
// threads at all.
class DeviceSimulatorWorker : public QObject
{
    Q_OBJECT

public:
    explicit DeviceSimulatorWorker(QObject *parent = nullptr);

public slots:
    void initialize(QList<double> initialTemperatures);
    void start();
    void stop();

signals:
    void telemetryReady(int row, double temperature, int signalStrength);
    void waveformReady(int row, double pressure, double flow, double volume);

private slots:
    void tick();
    void waveformTick();

private:
    struct DeviceState
    {
        double temperature = 36.0;
        int signalStrength = 90;
    };

    QTimer *m_timer;
    QTimer *m_waveformTimer;
    QVector<DeviceState> m_states;
    qint64 m_elapsedTicks = 0;
};
