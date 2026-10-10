#include "devicesimulator.h"

#include <QtCore/qglobal.h>

#include "device.h"
#include "devicelistmodel.h"
#include "devicesimulatorworker.h"

DeviceSimulator::DeviceSimulator(QObject *parent)
    : QObject(parent)
{
#if QT_CONFIG(thread)
    m_worker = new DeviceSimulatorWorker;
    m_worker->moveToThread(&m_workerThread);
    connect(&m_workerThread, &QThread::finished, m_worker, &QObject::deleteLater);
#else
    // Real threads aren't available on this platform — notably Qt for
    // WebAssembly's single-threaded configuration, which this project uses
    // deliberately to avoid the COOP/COEP response headers real WASM
    // threads require (see the hosting setup). The worker still runs the
    // identical code, just cooperatively on the main thread instead of a
    // dedicated one, so behavior is correct everywhere — only the actual
    // parallelism is unavailable where the platform can't provide it.
    m_worker = new DeviceSimulatorWorker(this);
#endif

    connect(this, &DeviceSimulator::requestInitialize, m_worker, &DeviceSimulatorWorker::initialize);
    connect(this, &DeviceSimulator::requestStart, m_worker, &DeviceSimulatorWorker::start);
    connect(this, &DeviceSimulator::requestStop, m_worker, &DeviceSimulatorWorker::stop);

    connect(m_worker, &DeviceSimulatorWorker::telemetryReady, this, &DeviceSimulator::applyTelemetry);
    connect(m_worker, &DeviceSimulatorWorker::waveformReady, this, &DeviceSimulator::applyWaveform);

#if QT_CONFIG(thread)
    m_workerThread.start();
#endif
}

DeviceSimulator::~DeviceSimulator()
{
#if QT_CONFIG(thread)
    m_workerThread.quit();
    m_workerThread.wait();
#endif
}

DeviceListModel *DeviceSimulator::model() const
{
    return m_model;
}

void DeviceSimulator::setModel(DeviceListModel *model)
{
    if (m_model == model)
        return;
    m_model = model;
    emit modelChanged();

    if (!m_model)
        return;

    QList<double> initialTemperatures;
    initialTemperatures.reserve(m_model->rowCount());
    for (int row = 0; row < m_model->rowCount(); ++row) {
        if (Device *device = m_model->deviceAt(row))
            initialTemperatures.append(device->temperature());
    }
    emit requestInitialize(initialTemperatures);
}

bool DeviceSimulator::isRunning() const
{
    return m_running;
}

void DeviceSimulator::setRunning(bool running)
{
    if (m_running == running)
        return;
    m_running = running;

    if (running)
        emit requestStart();
    else
        emit requestStop();

    emit runningChanged();
}

void DeviceSimulator::applyTelemetry(int row, double temperature, int signalStrength)
{
    if (!m_model)
        return;
    if (Device *device = m_model->deviceAt(row)) {
        device->setTemperature(temperature);
        device->setSignalStrength(signalStrength);
    }
}

void DeviceSimulator::applyWaveform(int row, double pressure, double flow, double volume)
{
    if (!m_model)
        return;
    if (Device *device = m_model->deviceAt(row))
        device->pushWaveformSample(pressure, flow, volume);
}
