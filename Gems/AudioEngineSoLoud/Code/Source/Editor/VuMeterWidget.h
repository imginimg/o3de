/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <QPen>
#include <QWidget>

namespace AudioControls
{
    class VuMeterWidget : public QWidget
    {
        Q_OBJECT

    public:
        struct ValueZone
        {
            qreal m_startValue = 0;
            QColor m_color;
            QColor m_peakColor;
        };

        explicit VuMeterWidget(QWidget* parent = nullptr);
        ~VuMeterWidget() = default;

        void SetChannelValue(int channelIndex, qreal value);

        void SetChannels(const QVector<QString>& channels);
        void SetValueZones(const QVector<ValueZone>& zones);
        void SetMinMaxValues(qreal minValue, qreal maxValue);

        void SetPeakLevelPenWidth(qreal width);
        void SetPeakDropTimeoutMs(int timeout);

        void SetScaleSpacing_LabelsAndLines(int spacing);
        void SetScaleMajorMaxStep(int step);
        void SetScaleMajorMinInterval(int interval);
        void SetScaleMinorMinInterval(int interval);
        void SetValueBarSpacing(int spacing);

        void SetScaleMajorLinesPen(const QPen& pen);
        void SetScaleMinorLinesPen(const QPen& pen);

        void SetScaleLabelFont(const QFont& font);
        void SetScaleLabelColor(const QColor& color);
        void SetChannelLabelFont(const QFont& font);
        void SetChannelLabelColor(const QColor& color);
        void SetChannelValueFont(const QFont& font);
        void SetChannelValueColor(const QColor& color);

        QSize minimumSizeHint() const override;
        QSize sizeHint() const override;

    protected:
        void timerEvent(QTimerEvent* event) override;
        void paintEvent(QPaintEvent* event) override;

    private:
        void RecalcChannelLabelBoundRect();
        void RecalcChannelValueBoundRect();
        void RecalcScaleLabelBoundRect();

        qreal GetScaleTopMargin() const;
        qreal GetScaleBottomMargin() const;
        qreal GetScaleLinesLeftMargin() const;
        qreal GetScaleLinesTopMargin() const;
        qreal GetScaleLinesBottomMargin() const;
        qreal GetValueBarsLeftMargin() const;
        qreal GetValueBarsRightMargin() const;

        void DrawChannelValues();
        void DrawScale();
        void DrawChannelValueBars();
        void DrawChannelLabels();

        QVector<QString> m_channels{ "L", "R" };
        QVector<qreal> m_values;
        QVector<qreal> m_peakValues;
        QVector<int> m_peakDropTimers;
        qreal m_peakLevelPenWidth = 2.0;
        int m_peakDropTimeoutMs = 3000;

        QVector<ValueZone> m_valueZones{ { -72.0, Qt::darkGreen, Qt::green },
                                         { -12.0, Qt::darkYellow, Qt::yellow },
                                         { 0.0, Qt::darkRed, Qt::red } };

        qreal m_maxValue = 12.0;
        qreal m_minValue = -72.0;

        int m_scaleSpacing_LabelsAndLines = 4;
        int m_scaleMajorMaxStep = 12;
        int m_scaleMajorMinInterval = 18;
        int m_scaleMinorMinInterval = 6;
        int m_valueBarSpacing = 8;

        QPen m_scaleMajorLinesPen = QPen(Qt::white, 1.0);
        QPen m_scaleMinorLinesPen = QPen(Qt::gray, 1.0);
        QFont m_scaleLabelFont = QFont("", 8);
        QFont m_channelLabelFont = QFont("", 8);
        QFont m_channelValueFont = QFont("", 9);
        QColor m_scaleLabelColor = Qt::white;
        QColor m_channelLabelColor = Qt::white;
        QColor m_channelValueColor = Qt::white;

        QRectF m_scaleLabelBoundRect;
        QRectF m_channelLabelBoundRect;
        QRectF m_channelValueBoundRect;
    };

} // namespace AudioControls
