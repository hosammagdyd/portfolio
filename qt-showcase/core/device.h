#pragma once

#include <QList>
#include <QObject>
#include <QString>
#include <qqmlintegration.h>

QT_BEGIN_NAMESPACE
class QStateMachine;
class QState;
QT_END_NAMESPACE

class Device : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_UNCREATABLE("Devices are created by DeviceListModel")

    Q_PROPERTY(QString id READ id CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(double temperature READ temperature WRITE setTemperature NOTIFY temperatureChanged)
    Q_PROPERTY(int signalStrength READ signalStrength WRITE setSignalStrength NOTIFY signalStrengthChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)

    Q_PROPERTY(QList<double> pressureWaveform READ pressureWaveform NOTIFY waveformChanged)
    Q_PROPERTY(QList<double> flowWaveform READ flowWaveform NOTIFY waveformChanged)
    Q_PROPERTY(QList<double> volumeWaveform READ volumeWaveform NOTIFY waveformChanged)
    Q_PROPERTY(int waveformCursor READ waveformCursor NOTIFY waveformChanged)

public:
    enum Status { Normal, Warning, Critical };
    Q_ENUM(Status)

    // Fixed-length ring buffer representing one full oscilloscope sweep.
    static constexpr int kWaveformCapacity = 200;

    explicit Device(QString id, QString name, QObject *parent = nullptr);

    QString id() const;
    QString name() const;

    double temperature() const;
    void setTemperature(double value);

    int signalStrength() const;
    void setSignalStrength(int value);

    Status status() const;
    QString statusText() const;

    QList<double> pressureWaveform() const;
    QList<double> flowWaveform() const;
    QList<double> volumeWaveform() const;
    int waveformCursor() const;

    // Writes one sample into each channel at the current sweep position,
    // then advances (and wraps) the cursor — oscilloscope-style redraw.
    void pushWaveformSample(double pressure, double flow, double volume);

signals:
    void temperatureChanged();
    void signalStrengthChanged();
    void statusChanged();
    void waveformChanged();

private:
    void setStatus(Status value);
    void setupStateMachine();

    QString m_id;
    QString m_name;
    double m_temperature = 0.0;
    int m_signalStrength = 100;
    Status m_status = Normal;

    QList<double> m_pressureWaveform;
    QList<double> m_flowWaveform;
    QList<double> m_volumeWaveform;
    int m_waveformCursor = 0;

    QStateMachine *m_machine = nullptr;
    QState *m_normalState = nullptr;
    QState *m_warningState = nullptr;
    QState *m_criticalState = nullptr;
};
