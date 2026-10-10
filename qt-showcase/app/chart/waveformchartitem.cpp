#include "waveformchartitem.h"

#include <QPointF>
#include <QSGFlatColorMaterial>
#include <QSGGeometry>
#include <QSGGeometryNode>
#include <QVector>
#include <algorithm>
#include <cmath>

namespace {

QSGGeometryNode *makeLineNode(const QColor &color, int vertexCount,
                               QSGGeometry::DrawingMode mode, float lineWidth = 1.0f)
{
    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), vertexCount);
    geometry->setDrawingMode(mode);
    geometry->setLineWidth(lineWidth);

    auto *material = new QSGFlatColorMaterial;
    material->setColor(color);

    auto *node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);
    return node;
}

// GPU line-width hints (QSGGeometry::setLineWidth) are frequently ignored by
// GL drivers and WebGL (both commonly clamp to 1px regardless of the
// request), so a visibly thick trace has to be real triangulated geometry —
// a quad per segment, offset from the centerline by half the thickness along
// each segment's normal — rather than a wide GL_LINE_STRIP.
QSGGeometryNode *makeThickLineNode(const QColor &color, const QVector<QPointF> &points,
                                    float thickness)
{
    const int segmentCount = points.size() - 1;
    if (segmentCount <= 0)
        return nullptr;

    auto *geometry = new QSGGeometry(QSGGeometry::defaultAttributes_Point2D(), segmentCount * 6);
    geometry->setDrawingMode(QSGGeometry::DrawTriangles);

    auto *material = new QSGFlatColorMaterial;
    material->setColor(color);

    auto *node = new QSGGeometryNode;
    node->setGeometry(geometry);
    node->setFlag(QSGNode::OwnsGeometry);
    node->setMaterial(material);
    node->setFlag(QSGNode::OwnsMaterial);

    QSGGeometry::Point2D *vertices = geometry->vertexDataAsPoint2D();
    const float halfThickness = thickness / 2.0f;

    for (int i = 0; i < segmentCount; ++i) {
        const QPointF &p0 = points.at(i);
        const QPointF &p1 = points.at(i + 1);

        QPointF direction = p1 - p0;
        const qreal length = std::hypot(direction.x(), direction.y());
        if (length > 0.0001)
            direction /= length;
        else
            direction = QPointF(1, 0);

        const QPointF normal(-direction.y() * halfThickness, direction.x() * halfThickness);

        const QPointF a = p0 + normal;
        const QPointF b = p0 - normal;
        const QPointF c = p1 + normal;
        const QPointF d = p1 - normal;

        const int base = i * 6;
        vertices[base + 0].set(a.x(), a.y());
        vertices[base + 1].set(b.x(), b.y());
        vertices[base + 2].set(c.x(), c.y());

        vertices[base + 3].set(b.x(), b.y());
        vertices[base + 4].set(d.x(), d.y());
        vertices[base + 5].set(c.x(), c.y());
    }

    return node;
}

} // namespace

WaveformChartItem::WaveformChartItem(QQuickItem *parent)
    : QQuickItem(parent)
{
    setFlag(ItemHasContents, true);
}

QList<double> WaveformChartItem::samples() const
{
    return m_samples;
}

void WaveformChartItem::setSamples(const QList<double> &samples)
{
    m_samples = samples;
    emit samplesChanged();
    update();
}

int WaveformChartItem::cursor() const
{
    return m_cursor;
}

void WaveformChartItem::setCursor(int cursor)
{
    if (m_cursor == cursor)
        return;
    m_cursor = cursor;
    emit cursorChanged();
    update();
}

double WaveformChartItem::minValue() const
{
    return m_minValue;
}

void WaveformChartItem::setMinValue(double value)
{
    if (m_minValue == value)
        return;
    m_minValue = value;
    emit rangeChanged();
    update();
}

double WaveformChartItem::maxValue() const
{
    return m_maxValue;
}

void WaveformChartItem::setMaxValue(double value)
{
    if (m_maxValue == value)
        return;
    m_maxValue = value;
    emit rangeChanged();
    update();
}

QColor WaveformChartItem::lineColor() const
{
    return m_lineColor;
}

void WaveformChartItem::setLineColor(const QColor &color)
{
    if (m_lineColor == color)
        return;
    m_lineColor = color;
    emit lineColorChanged();
    update();
}

QColor WaveformChartItem::gridColor() const
{
    return m_gridColor;
}

void WaveformChartItem::setGridColor(const QColor &color)
{
    if (m_gridColor == color)
        return;
    m_gridColor = color;
    emit gridColorChanged();
    update();
}

int WaveformChartItem::gridRows() const
{
    return m_gridRows;
}

void WaveformChartItem::setGridRows(int rows)
{
    if (m_gridRows == rows)
        return;
    m_gridRows = rows;
    emit gridRowsChanged();
    update();
}

QSGNode *WaveformChartItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    delete oldNode;
    auto *root = new QSGNode;

    const qreal w = width();
    const qreal h = height();
    if (w <= 0 || h <= 0)
        return root;

    if (m_gridRows > 0) {
        auto *gridNode = makeLineNode(m_gridColor, (m_gridRows + 1) * 2, QSGGeometry::DrawLines);
        QSGGeometry::Point2D *vertices = gridNode->geometry()->vertexDataAsPoint2D();
        for (int row = 0; row <= m_gridRows; ++row) {
            const qreal y = (h / m_gridRows) * row;
            vertices[row * 2].set(0, y);
            vertices[row * 2 + 1].set(w, y);
        }
        root->appendChildNode(gridNode);
    }

    const int sampleCount = m_samples.size();
    const double range = std::max(0.0001, m_maxValue - m_minValue);
    constexpr float kTraceThickness = 3.0f;
    constexpr float kCursorThickness = 2.5f;

    if (sampleCount >= 2) {
        const qreal stepX = w / (sampleCount - 1);
        auto yFor = [&](double value) {
            const double normalized = (value - m_minValue) / range;
            return h - normalized * h;
        };

        const int cursor = std::clamp(m_cursor, 0, sampleCount);

        // Trailing arc ahead of the cursor: data left over from the previous
        // sweep, not yet overwritten. Drawn dim.
        const int trailingCount = sampleCount - cursor;
        if (trailingCount >= 2) {
            QColor dim = m_lineColor;
            dim.setAlphaF(0.3f);
            QVector<QPointF> points;
            points.reserve(trailingCount);
            for (int i = 0; i < trailingCount; ++i) {
                const int sampleIndex = cursor + i;
                points.append(QPointF(sampleIndex * stepX, yFor(m_samples.at(sampleIndex))));
            }
            if (auto *node = makeThickLineNode(dim, points, kTraceThickness))
                root->appendChildNode(node);
        }

        // Freshly written arc behind the cursor, full brightness.
        if (cursor >= 2) {
            QVector<QPointF> points;
            points.reserve(cursor);
            for (int i = 0; i < cursor; ++i)
                points.append(QPointF(i * stepX, yFor(m_samples.at(i))));
            if (auto *node = makeThickLineNode(m_lineColor, points, kTraceThickness))
                root->appendChildNode(node);
        }

        // Sweep cursor line.
        {
            const QColor cursorColor = m_lineColor.lighter(140);
            const qreal cursorX = cursor * stepX;
            const QVector<QPointF> points{QPointF(cursorX, 0), QPointF(cursorX, h)};
            if (auto *node = makeThickLineNode(cursorColor, points, kCursorThickness))
                root->appendChildNode(node);
        }
    }

    return root;
}
