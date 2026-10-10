#include "devicesimulatorworker.h"

#include <QRandomGenerator>
#include <cmath>

namespace {
constexpr double kWaveformBaseline = 50.0;

// Generic Assisted Volume Control (VC-A) breath-cycle shape — see
// DeviceSimulatorWorker::waveformTick() for the full explanation.
constexpr double kInspiratoryFraction = 0.33;
constexpr double kPeep = 5.0;
}

DeviceSimulatorWorker::DeviceSimulatorWorker(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_waveformTimer(new QTimer(this))
{
    m_timer->setInterval(1500);
    connect(m_timer, &QTimer::timeout, this, &DeviceSimulatorWorker::tick);

    // Runs much faster than the telemetry tick so the waveform reads as a
    // genuinely live, smoothly scrolling trace rather than a jagged stairstep.
    m_waveformTimer->setInterval(80);
    connect(m_waveformTimer, &QTimer::timeout, this, &DeviceSimulatorWorker::waveformTick);
}

void DeviceSimulatorWorker::initialize(QList<double> initialTemperatures)
{
    m_states.clear();
    m_states.reserve(initialTemperatures.size());
    for (double temperature : std::as_const(initialTemperatures))
        m_states.append(DeviceState{temperature, 90});
}

void DeviceSimulatorWorker::start()
{
    m_timer->start();
    m_waveformTimer->start();
}

void DeviceSimulatorWorker::stop()
{
    m_timer->stop();
    m_waveformTimer->stop();
}

void DeviceSimulatorWorker::tick()
{
    auto *random = QRandomGenerator::global();

    for (int row = 0; row < m_states.size(); ++row) {
        DeviceState &state = m_states[row];

        const double temperatureDelta = (random->generateDouble() - 0.5) * 3.0;
        state.temperature = qBound(10.0, state.temperature + temperatureDelta, 95.0);

        const int signalDelta = random->bounded(-4, 5);
        state.signalStrength = qBound(5, state.signalStrength + signalDelta, 100);

        emit telemetryReady(row, state.temperature, state.signalStrength);
    }
}

void DeviceSimulatorWorker::waveformTick()
{
    ++m_elapsedTicks;

    auto *random = QRandomGenerator::global();

    for (int row = 0; row < m_states.size(); ++row) {
        const DeviceState &state = m_states.at(row);

        // Each device gets a slightly different respiratory rate so the
        // traces don't all breathe in lockstep, and the amplitude breathes
        // with the device's current temperature so the charts visibly react
        // to state.
        const int cycleTicks = 50 + row * 4;
        const int tickWithinCycle = static_cast<int>(m_elapsedTicks % cycleTicks);
        const double t = double(tickWithinCycle) / cycleTicks;

        const double severity = (state.temperature - kWaveformBaseline) * 0.1;
        const double peakFlow = 40.0 + severity;          // L/min
        const double peakVolume = 420.0 + severity * 4.0; // mL
        const double peakPressure = 20.0 + severity;      // cmH2O

        double flow;
        double volume;
        double pressure;

        if (t < kInspiratoryFraction) {
            // Inspiration: constant ("square") flow delivers a linear volume
            // ramp, while airway pressure rises with the lung's compliance.
            const double u = t / kInspiratoryFraction;
            flow = peakFlow;
            volume = peakVolume * u;
            pressure = kPeep + (peakPressure - kPeep) * std::pow(u, 0.6);
        } else {
            // Expiration: passive recoil — flow and volume decay
            // exponentially back toward baseline, pressure drops rapidly to PEEP.
            const double v = (t - kInspiratoryFraction) / (1.0 - kInspiratoryFraction);
            flow = -peakFlow * 1.2 * std::exp(-v * 4.0);
            volume = peakVolume * std::exp(-v * 4.0);
            pressure = kPeep + (peakPressure - kPeep) * std::exp(-v * 20.0);
        }

        const double noise = (random->generateDouble() - 0.5) * 0.8;
        emit waveformReady(row, pressure + noise * 0.3, flow + noise, volume + noise * 2.0);
    }
}
