#pragma once

#include <QColor>
#include <QList>
#include <QQuickItem>
#include <qqmlintegration.h>

// Renders an oscilloscope-style sweeping waveform directly on the Qt Quick
// scene graph (QSGGeometryNode / RHI, GPU-accelerated) rather than through
// QML Canvas, which rasterizes on the CPU every repaint.
class WaveformChartItem : public QQuickItem
{
    Q_OBJECT
    QML_ELEMENT

    Q_PROPERTY(QList<double> samples READ samples WRITE setSamples NOTIFY samplesChanged)
    Q_PROPERTY(int cursor READ cursor WRITE setCursor NOTIFY cursorChanged)
    Q_PROPERTY(double minValue READ minValue WRITE setMinValue NOTIFY rangeChanged)
    Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue NOTIFY rangeChanged)
    Q_PROPERTY(QColor lineColor READ lineColor WRITE setLineColor NOTIFY lineColorChanged)
    Q_PROPERTY(QColor gridColor READ gridColor WRITE setGridColor NOTIFY gridColorChanged)
    Q_PROPERTY(int gridRows READ gridRows WRITE setGridRows NOTIFY gridRowsChanged)

public:
    explicit WaveformChartItem(QQuickItem *parent = nullptr);

    QList<double> samples() const;
    void setSamples(const QList<double> &samples);

    int cursor() const;
    void setCursor(int cursor);

    double minValue() const;
    void setMinValue(double value);

    double maxValue() const;
    void setMaxValue(double value);

    QColor lineColor() const;
    void setLineColor(const QColor &color);

    QColor gridColor() const;
    void setGridColor(const QColor &color);

    int gridRows() const;
    void setGridRows(int rows);

signals:
    void samplesChanged();
    void cursorChanged();
    void rangeChanged();
    void lineColorChanged();
    void gridColorChanged();
    void gridRowsChanged();

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *) override;

private:
    QList<double> m_samples;
    int m_cursor = 0;
    double m_minValue = 0.0;
    double m_maxValue = 100.0;
    QColor m_lineColor{"#3b82f6"};
    QColor m_gridColor{"#2d333b"};
    int m_gridRows = 4;
};
