/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <QWidget>

#include <ACETypes.h>

#include <EditorEngineInterop.h>
#include <ui_AudioFileToTriggerParamsWidget.h>

namespace AudioControls
{
    class AudioFilterListWidget;

    class AudioFileToTriggerParamsWidget
        : public QWidget
        , private Ui::AudioFileToTriggerParamsWidget
        , private Audio::AudioBusManagerNotificationBus::Handler
    {
        Q_OBJECT

    public:
        explicit AudioFileToTriggerParamsWidget(
            TConnectionPtr connection, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioFileToTriggerParamsWidget();

    signals:
        void PropertiesChanged();

    private slots:
        void OnActionChanged(int index);
        void OnVolumeChanged();
        void UpdateConnectionFromWidgets();

    private:
        // AudioBusManagerNotifications
        void OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames) override;
        // ~AudioBusManagerNotifications

        void UpdateWidgetsFromConnection();

        TConnectionPtr m_connection;
        AudioFilterListWidget* m_filterListWidget = nullptr;
        bool m_inUpdateWidgetsFromConnection = false;
    };
} // namespace AudioControls
