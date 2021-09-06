/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <QFrame>

#include <EditorEngineInterop.h>
#include <ui_AudioBusWidget.h>

namespace AudioControls
{
    class AudioFilterListWidget;

    class AudioBusWidget
        : public QFrame
        , private Ui::AudioBusWidget
        , private Audio::AudioBusManagerNotificationBus::Handler
    {
        Q_OBJECT

    public:
        explicit AudioBusWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioBusWidget();

        void SetData(const Audio::BusData& busData, int32_t busIndex);
        void SetSelected(bool isSelected);
        const AZ::Name& GetBusName() const;

    protected:
        void mousePressEvent(QMouseEvent* event) override;
        void mouseMoveEvent(QMouseEvent* event) override;
        void dragEnterEvent(QDragEnterEvent* event) override;
        void dropEvent(QDropEvent* event) override;

    private slots:
        void OnBusNameChanged();
        void OnVolumeChanged(int value);
        void OnVolumeChanged(double value);
        void OnMuteStateChanged(bool value);
        void OnMonoStateChanged(bool value);
        void OnOutputBusNameChanged(const QString& newBusName);

    private:
        // AudioBusManagerNotificationBus
        void OnRequestCompleted_ChangeAudioBusName(bool success, AZ::Name oldBusName, AZ::Name newBusName) override;
        void OnRequestCompleted_ChangeAudioBusIndex(bool success, AZ::Name busName, int32_t oldBusIndex, int32_t newBusIndex) override;
        void OnRequestCompleted_SetAudioBusVolumeDb(bool success, AZ::Name busName, float volume) override;
        void OnRequestCompleted_SetAudioBusVolumeLinear(bool success, AZ::Name busName, float volume) override;
        void OnRequestCompleted_SetAudioBusMuted(bool success, AZ::Name busName, bool isMuted) override;
        void OnRequestCompleted_SetAudioBusMono(bool success, AZ::Name busName, bool isMono) override;
        void OnRequestCompleted_SetAudioBusOutput(bool success, AZ::Name busName, AZ::Name outputBusName) override;
        void OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames) override;
        void OnUpdateAudioBusPeakVolumes(AZ::Name busName, AZStd::vector<float> channelVolumes) override;
        // ~AudioBusManagerNotificationBus

        void BlockSignals(bool isBlock);

        AudioFilterListWidget* m_filterListWidget = nullptr;
        AZ::Name m_busName;
        AZ::Name m_outputBusName;
        int32_t m_busIndex = 0;
        QPoint m_dragStartPosition;
    };
} // namespace AudioControls
