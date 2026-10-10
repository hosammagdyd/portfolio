#include "device.h"

#include <QSignalTransition>
#include <QState>
#include <QStateMachine>

namespace {

constexpr double kWarningThreshold = 55.0;
constexpr double kCriticalThreshold = 75.0;
constexpr double kUnbounded = 1000.0;

// Fires on Device::temperatureChanged, but only completes the transition
// when the new temperature actually falls within [lowerBound, upperBound).
class TemperatureRangeTransition : public QSignalTransition
{
public:
    TemperatureRangeTransition(Device *device, double lowerBound, double upperBound)
        : QSignalTransition(device, &Device::temperatureChanged)
        , m_device(device)
        , m_lowerBound(lowerBound)
        , m_upperBound(upperBound)
    {
    }

protected:
    bool eventTest(QEvent *event) override
    {
        if (!QSignalTransition::eventTest(event))
            return false;

        const double temperature = m_device->temperature();
        return temperature >= m_lowerBound && temperature < m_upperBound;
    }

private:
    Device *m_device;
    double m_lowerBound;
    double m_upperBound;
};

} // namespace

Device::Device(QString id, QString name, QObject *parent)
    : QObject(parent)
    , m_id(std::move(id))
    , m_name(std::move(name))
{
    m_pressureWaveform.fill(0.0, kWaveformCapacity);
    m_flowWaveform.fill(0.0, kWaveformCapacity);
    m_volumeWaveform.fill(0.0, kWaveformCapacity);
    setupStateMachine();
}

void Device::setupStateMachine()
{
    m_machine = new QStateMachine(this);
    m_normalState = new QState(m_machine);
    m_warningState = new QState(m_machine);
    m_criticalState = new QState(m_machine);

    connect(m_normalState, &QState::entered, this, [this] { setStatus(Normal); });
    connect(m_warningState, &QState::entered, this, [this] { setStatus(Warning); });
    connect(m_criticalState, &QState::entered, this, [this] { setStatus(Critical); });

    auto addTransition = [this](QState *from, QState *to, double lowerBound, double upperBound) {
        auto *transition = new TemperatureRangeTransition(this, lowerBound, upperBound);
        transition->setTargetState(to);
        from->addTransition(transition);
    };

    addTransition(m_normalState, m_warningState, kWarningThreshold, kCriticalThreshold);
    addTransition(m_normalState, m_criticalState, kCriticalThreshold, kUnbounded);

    addTransition(m_warningState, m_normalState, -kUnbounded, kWarningThreshold);
    addTransition(m_warningState, m_criticalState, kCriticalThreshold, kUnbounded);

    addTransition(m_criticalState, m_warningState, kWarningThreshold, kCriticalThreshold);
    addTransition(m_criticalState, m_normalState, -kUnbounded, kWarningThreshold);

    m_machine->setInitialState(m_normalState);
    m_machine->start();
}

QString Device::id() const
{
    return m_id;
}

QString Device::name() const
{
    return m_name;
}

double Device::temperature() const
{
    return m_temperature;
}

void Device::setTemperature(double value)
{
    if (m_temperature == value)
        return;
    m_temperature = value;
    emit temperatureChanged();
}

int Device::signalStrength() const
{
    return m_signalStrength;
}

void Device::setSignalStrength(int value)
{
    if (m_signalStrength == value)
        return;
    m_signalStrength = value;
    emit signalStrengthChanged();
}

Device::Status Device::status() const
{
    return m_status;
}

void Device::setStatus(Status value)
{
    if (m_status == value)
        return;
    m_status = value;
    emit statusChanged();
}

QString Device::statusText() const
{
    switch (m_status) {
    case Warning:
        return QStringLiteral("warning");
    case Critical:
        return QStringLiteral("critical");
    case Normal:
    default:
        return QStringLiteral("normal");
    }
}

QList<double> Device::pressureWaveform() const
{
    return m_pressureWaveform;
}

QList<double> Device::flowWaveform() const
{
    return m_flowWaveform;
}

QList<double> Device::volumeWaveform() const
{
    return m_volumeWaveform;
}

int Device::waveformCursor() const
{
    return m_waveformCursor;
}

void Device::pushWaveformSample(double pressure, double flow, double volume)
{
    m_pressureWaveform[m_waveformCursor] = pressure;
    m_flowWaveform[m_waveformCursor] = flow;
    m_volumeWaveform[m_waveformCursor] = volume;
    m_waveformCursor = (m_waveformCursor + 1) % kWaveformCapacity;
    emit waveformChanged();
}
