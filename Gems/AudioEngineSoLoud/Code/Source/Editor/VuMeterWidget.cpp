/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <VuMeterWidget.h>

#include <QApplication>
#include <QPainter>
#include <QTimerEvent>
#include <QtMath>

namespace AudioControls
{
    VuMeterWidget::VuMeterWidget(QWidget* parent)
        : QWidget(parent)
    {
        SetChannels(m_channels);
        SetMinMaxValues(m_minValue, m_maxValue);
        SetScaleLabelFont(QFont(QApplication::font().family(), m_scaleLabelFont.pointSize()));
        SetChannelLabelFont(QFont(QApplication::font().family(), m_channelLabelFont.pointSize()));
        SetChannelValueFont(QFont(QApplication::font().family(), m_channelValueFont.pointSize()));
    }

    void VuMeterWidget::SetChannels(const QVector<QString>& channels)
    {
        Q_ASSERT(!channels.empty());
        m_channels = channels;
        m_values.resize(channels.size());
        m_values.fill(m_minValue - 1.0);
        m_peakValues = m_values;
        m_peakDropTimers.resize(channels.size());
        RecalcChannelLabelBoundRect();
        update();
    }

    void VuMeterWidget::SetValueZones(const QVector<ValueZone>& zones)
    {
        m_valueZones = zones;
        update();
    }

    void VuMeterWidget::SetMinMaxValues(qreal minValue, qreal maxValue)
    {
        m_minValue = minValue;
        m_maxValue = maxValue;
        m_values.fill(minValue - 1.0);
        RecalcChannelValueBoundRect();
        update();
    }

    void VuMeterWidget::SetPeakLevelPenWidth(qreal width)
    {
        m_peakLevelPenWidth = width;
        update();
    }

    void VuMeterWidget::SetPeakDropTimeoutMs(int timeout)
    {
        m_peakDropTimeoutMs = timeout;
    }

    void VuMeterWidget::SetScaleSpacing_LabelsAndLines(int spacing)
    {
        m_scaleSpacing_LabelsAndLines = spacing;
        update();
    }

    void VuMeterWidget::SetScaleMajorMaxStep(int step)
    {
        m_scaleMajorMaxStep = step;
        update();
    }

    void VuMeterWidget::SetValueBarSpacing(int spacing)
    {
        m_valueBarSpacing = spacing;
        update();
    }

    void VuMeterWidget::SetScaleMajorLinesPen(const QPen& pen)
    {
        m_scaleMajorLinesPen = pen;
        update();
    }

    void VuMeterWidget::SetScaleMinorLinesPen(const QPen& pen)
    {
        m_scaleMinorLinesPen = pen;
        update();
    }

    void VuMeterWidget::SetScaleMajorMinInterval(int interval)
    {
        m_scaleMajorMinInterval = interval;
        update();
    }

    void VuMeterWidget::SetScaleMinorMinInterval(int interval)
    {
        m_scaleMinorMinInterval = interval;
        update();
    }

    void VuMeterWidget::SetScaleLabelFont(const QFont& font)
    {
        m_scaleLabelFont = font;
        RecalcScaleLabelBoundRect();
        update();
    }

    void VuMeterWidget::SetScaleLabelColor(const QColor& color)
    {
        m_scaleLabelColor = color;
        update();
    }

    void VuMeterWidget::SetChannelLabelFont(const QFont& font)
    {
        m_channelLabelFont = font;
        RecalcChannelLabelBoundRect();
        update();
    }

    void VuMeterWidget::SetChannelLabelColor(const QColor& color)
    {
        m_channelLabelColor = color;
        update();
    }

    void VuMeterWidget::SetChannelValueFont(const QFont& font)
    {
        m_channelValueFont = font;
        RecalcChannelValueBoundRect();
        update();
    }

    void VuMeterWidget::SetChannelValueColor(const QColor& color)
    {
        m_channelValueColor = color;
        update();
    }

    void VuMeterWidget::SetChannelValue(int channelIndex, qreal value)
    {
        m_values[channelIndex] = value;

        if (value > m_peakValues[channelIndex])
        {
            killTimer(m_peakDropTimers[channelIndex]);
            m_peakValues[channelIndex] = value;
            m_peakDropTimers[channelIndex] = startTimer(m_peakDropTimeoutMs);
        }

        update();
    }

    QSize VuMeterWidget::minimumSizeHint() const
    {
        int valueSpan = qCeil(m_maxValue) - qFloor(m_minValue);

        int channelLabelsWidth = m_channelLabelBoundRect.width() * m_channels.size();
        int channelValuesWidth = m_channelValueBoundRect.width() * m_channels.size();
        int biggestWidth = (channelLabelsWidth > channelValuesWidth) ? channelLabelsWidth : channelValuesWidth;

        return QSize(
            GetValueBarsLeftMargin() + GetValueBarsRightMargin() + biggestWidth,
            GetScaleLinesTopMargin() + GetScaleLinesBottomMargin() + valueSpan / m_scaleMajorMaxStep * m_scaleMajorMinInterval);
    }

    QSize VuMeterWidget::sizeHint() const
    {
        return minimumSizeHint();
    }

    void VuMeterWidget::timerEvent(QTimerEvent* event)
    {
        int timerId = event->timerId();
        killTimer(timerId);

        int channelIndex = m_peakDropTimers.indexOf(timerId);
        if (channelIndex == -1)
        {
            return;
        }

        m_peakValues[channelIndex] = m_values[channelIndex];
        m_peakDropTimers[channelIndex] = startTimer(m_peakDropTimeoutMs);
    }

    void VuMeterWidget::paintEvent(QPaintEvent*)
    {
        DrawChannelValues();
        DrawScale();
        DrawChannelValueBars();
        DrawChannelLabels();
    }

    void VuMeterWidget::RecalcChannelLabelBoundRect()
    {
        QString longestLabel;
        for (int i = 0; i < m_channels.size(); ++i)
        {
            if (m_channels[i].size() > longestLabel.size())
            {
                longestLabel = m_channels[i];
            }
        }

        QFontMetricsF fontMetrics(m_channelLabelFont, this);
        m_channelLabelBoundRect = fontMetrics.boundingRect(longestLabel);
    }

    void VuMeterWidget::RecalcChannelValueBoundRect()
    {
        QFontMetricsF fontMetrics(m_channelValueFont, this);
        QString str = QString::number(-qMax(qAbs(m_maxValue), qAbs(m_minValue)), 'g', 3);
        str.append(str);
        m_channelValueBoundRect = fontMetrics.boundingRect(str);
    }

    void VuMeterWidget::RecalcScaleLabelBoundRect()
    {
        QFontMetricsF fontMetrics(m_scaleLabelFont, this);
        m_scaleLabelBoundRect = fontMetrics.boundingRect(QString::number(-qMax(qAbs(m_maxValue), qAbs(m_minValue))));
    }

    qreal VuMeterWidget::GetScaleTopMargin() const
    {
        return m_channelValueBoundRect.height();
    }

    qreal VuMeterWidget::GetScaleBottomMargin() const
    {
        return m_channelLabelBoundRect.height();
    }

    qreal VuMeterWidget::GetScaleLinesLeftMargin() const
    {
        return m_scaleLabelBoundRect.width() + m_scaleSpacing_LabelsAndLines;
    }

    qreal VuMeterWidget::GetScaleLinesTopMargin() const
    {
        return GetScaleTopMargin() + m_scaleLabelBoundRect.height() / 2.0;
    }

    qreal VuMeterWidget::GetScaleLinesBottomMargin() const
    {
        return GetScaleBottomMargin() + m_scaleLabelBoundRect.height() / 2.0;
    }

    qreal VuMeterWidget::GetValueBarsLeftMargin() const
    {
        return GetScaleLinesLeftMargin() + m_valueBarSpacing;
    }

    qreal VuMeterWidget::GetValueBarsRightMargin() const
    {
        return m_valueBarSpacing;
    }

    void VuMeterWidget::DrawChannelValues()
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(m_channelValueFont);
        painter.setPen(m_channelValueColor);

        qreal leftMargin = GetValueBarsLeftMargin();
        qreal rightMargin = GetValueBarsRightMargin();
        qreal sectorWidth = width() - leftMargin - rightMargin;
        qreal barWidth = sectorWidth / m_channels.size();
        qreal barHalfWidth = barWidth / 2.0;
        qreal textRectHalfWidth = m_channelValueBoundRect.width() / 2.0;

        QRectF textRect = m_channelValueBoundRect;

        for (int i = 0; i < m_channels.size(); ++i)
        {
            qreal barMidpoint = leftMargin + i * barWidth + barHalfWidth;

            qreal value = m_peakValues[i];
            QString valueStr;
            if (value < m_minValue)
            {
                valueStr = "-∞";
            }
            else
            {
                valueStr = QString::number(value, 'f', 1);
            }

            textRect.moveTo(barMidpoint - textRectHalfWidth, 0.0);
            painter.drawText(textRect, Qt::AlignCenter, valueStr);
        }
    }

    void VuMeterWidget::DrawScale()
    {
        QPainter painter(this);

        qreal topLinesMargin = GetScaleLinesTopMargin();
        qreal bottomLinesMargin = GetScaleLinesBottomMargin();
        qreal leftLinesMargin = GetScaleLinesLeftMargin();
        qreal scaleSpan = height() - topLinesMargin - bottomLinesMargin;

        int maxValueInt = qCeil(m_maxValue);
        int minValueInt = qFloor(m_minValue);
        int valueSpan = qAbs(maxValueInt - minValueInt);
        int refValue = (maxValueInt == 0.0) ? minValueInt : maxValueInt;
        int majorStepCount = 0;

        int majorValueStep = 1;
        for (; majorValueStep <= m_scaleMajorMaxStep; ++majorValueStep)
        {
            if (refValue % majorValueStep)
            {
                continue;
            }

            majorStepCount = valueSpan / majorValueStep;
            int scaleSpanNeeded = m_scaleMajorMinInterval * majorStepCount;
            if (scaleSpanNeeded <= scaleSpan)
            {
                break;
            }
        }

        if (majorValueStep > m_scaleMajorMaxStep)
        {
            return;
        }

        qreal majorStepSize = scaleSpan / majorStepCount;

        // Draw major lines.
        painter.save();
        painter.setPen(m_scaleMajorLinesPen);
        for (int majorStep = 0; majorStep <= majorStepCount; ++majorStep)
        {
            qreal y = topLinesMargin + majorStep * majorStepSize;
            QLineF line(leftLinesMargin, y, width(), y);
            painter.drawLine(line);
        }
        painter.restore();

        // Draw minor lines.
        painter.save();
        painter.setPen(m_scaleMinorLinesPen);
        {
            int minorStepCount = 0;
            int minorValueStep = 1;
            for (; minorValueStep < majorValueStep; ++minorValueStep)
            {
                if (majorValueStep % minorValueStep)
                {
                    continue;
                }

                minorStepCount = majorValueStep / minorValueStep;
                int scaleSpanNeeded = m_scaleMinorMinInterval * minorStepCount;
                if (scaleSpanNeeded <= majorStepSize)
                {
                    break;
                }
            }

            if (minorValueStep < m_scaleMajorMaxStep)
            {
                qreal minorStepSize = majorStepSize / minorStepCount;

                for (int majorStep = 0; majorStep < majorStepCount; ++majorStep)
                {
                    for (int minorStep = 1; minorStep < minorStepCount; ++minorStep)
                    {
                        qreal y = topLinesMargin + majorStep * majorStepSize + minorStep * minorStepSize;
                        QLineF line(leftLinesMargin, y, width(), y);
                        painter.drawLine(line);
                    }
                }
            }
        }
        painter.restore();

        // Draw value labels.
        painter.save();
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(m_scaleLabelFont);
        painter.setPen(m_scaleLabelColor);
        for (int step = 0; step <= majorStepCount; ++step)
        {
            int value = maxValueInt - step * majorValueStep;
            QRectF rect = m_scaleLabelBoundRect;
            rect.moveTo(0, GetScaleTopMargin() + step * majorStepSize);
            painter.drawText(rect, Qt::AlignRight, QString::number(value));
        }
        painter.restore();
    }

    void VuMeterWidget::DrawChannelValueBars()
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.translate(0.0, height());
        painter.scale(1.0, -1.0);

        int maxValueInt = qCeil(m_maxValue);
        int minValueInt = qFloor(m_minValue);
        int valueSpan = qAbs(maxValueInt - minValueInt);

        qreal leftMargin = GetValueBarsLeftMargin();
        qreal topMargin = GetScaleLinesTopMargin();
        qreal bottomMargin = GetScaleLinesBottomMargin();
        qreal barSectorWidth = width() - leftMargin;
        qreal barSectorHeight = height() - topMargin - bottomMargin;
        qreal barWidthWithSpacing = barSectorWidth / m_channels.size();
        qreal barWidth = barWidthWithSpacing - m_valueBarSpacing;
        qreal barStartPointY = bottomMargin;

        for (int channelIndex = 0; channelIndex < m_channels.size(); ++channelIndex)
        {
            qreal barStartPointX = leftMargin + channelIndex * barWidthWithSpacing;
            qreal valueNorm = (m_values[channelIndex] - minValueInt) / valueSpan;

            qreal peakValueNorm = (m_peakValues[channelIndex] - minValueInt) / valueSpan;
            if (peakValueNorm > 1.0)
            {
                peakValueNorm = 1.0;
            }

            for (int zoneIndex = 0; zoneIndex < m_valueZones.size(); ++zoneIndex)
            {
                qreal zoneStartValue = m_valueZones[zoneIndex].m_startValue;
                qreal zoneEndValue = (zoneIndex == m_valueZones.size() - 1) ? m_maxValue : m_valueZones[zoneIndex + 1].m_startValue;
                qreal zoneStartNorm = (zoneStartValue - minValueInt) / valueSpan;
                qreal zoneEndNorm = (zoneEndValue - minValueInt) / valueSpan;
                qreal zoneFillNorm = (valueNorm - zoneStartNorm) / (zoneEndNorm - zoneStartNorm);
                zoneFillNorm = qBound(0.0, zoneFillNorm, 1.0);

                if (!qFuzzyIsNull(zoneFillNorm))
                {
                    QRectF barRect(
                        barStartPointX, barStartPointY + zoneStartNorm * barSectorHeight, barWidth,
                        zoneFillNorm * (zoneEndNorm - zoneStartNorm) * barSectorHeight);
                    painter.fillRect(barRect, m_valueZones[zoneIndex].m_color);
                }

                // Draw peak level.
                if (peakValueNorm >= 0.0 && peakValueNorm >= zoneStartNorm && peakValueNorm <= zoneEndNorm)
                {
                    peakValueNorm = qBound(0.0, peakValueNorm, 1.0);
                    qreal y = barStartPointY + peakValueNorm * barSectorHeight;

                    painter.save();
                    painter.setPen(QPen(m_valueZones[zoneIndex].m_peakColor, m_peakLevelPenWidth));
                    QLineF line(barStartPointX, y, barStartPointX + barWidth, y);
                    painter.drawLine(line);
                    painter.restore();
                }
            }
        }
    }

    void VuMeterWidget::DrawChannelLabels()
    {
        QPainter painter(this);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setFont(m_channelLabelFont);
        painter.setPen(m_channelLabelColor);

        qreal leftMargin = GetValueBarsLeftMargin();
        qreal rightMargin = GetValueBarsRightMargin();
        qreal sectorWidth = width() - leftMargin - rightMargin;
        qreal barWidth = sectorWidth / m_channels.size();
        qreal barHalfWidth = barWidth / 2.0;

        QRectF textRect = m_channelLabelBoundRect;
        qreal textHalfWidth = textRect.width() / 2.0;
        qreal textY = height() - textRect.height();

        for (int i = 0; i < m_channels.size(); ++i)
        {
            qreal barMidpoint = leftMargin + i * barWidth + barHalfWidth;
            textRect.moveTo(barMidpoint - textHalfWidth, textY);
            painter.drawText(textRect, Qt::AlignCenter, m_channels[i]);
        }
    }

} // namespace AudioControls
