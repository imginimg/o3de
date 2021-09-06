/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <IEditor.h>

#include <QMainWindow>

#include <EditorEngineInterop.h>
#include <ui_AudioBusManagerWindow.h>

class QHBoxLayout;
class QMenu;

namespace AudioControls
{
    class AudioBusWidget;

    class AudioBusManagerWindow
        : public QMainWindow
        , private Ui::AudioBusManagerWindow
        , private Audio::AudioBusManagerNotificationBus::Handler
    {
        Q_OBJECT

    public:
        explicit AudioBusManagerWindow(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioBusManagerWindow();

        static const GUID& GetClassID();

    protected:
        constexpr static const int RequestBusPeakVolumesIntervalMs = 5;
        void timerEvent(QTimerEvent* event) override;

        void mousePressEvent(QMouseEvent* event) override;

    public slots:
        void OnAddBusClicked();
        void OnRemoveBusClicked();
        void OnResetClicked();
        void OnSaveClicked();
        void OnSaveAsClicked();
        void OnLoadClicked();
        void OnCustomContextMenuRequested(const QPoint& pos);

    private:
        // AudioBusManagerNotifications
        void OnRequestCompleted_AddAudioBus(bool success, Audio::BusData busData) override;
        void OnRequestCompleted_RemoveAudioBus(bool success, AZ::Name busName) override;
        void OnRequestCompleted_ChangeAudioBusIndex(bool success, AZ::Name busName, int32_t oldBusIndex, int32_t newBusIndex) override;
        void OnUpdateAudioBusLayout(Audio::BusLayoutData busLayout) override;
        void OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames) override;
        // ~AudioBusManagerNotifications

        void SetCurrentSavePath(const QString& path);
        void SaveLayoutFile(bool saveAs = true);
        void DestroyAudioBusWidgets();

        AZStd::vector<AZ::Name> m_busNames;
        class AudioBusWidget* m_selectedWidget = nullptr;
        QHBoxLayout* m_busWidgetsLayout = nullptr;
        QMenu* m_contextMenu = nullptr;
        QString m_currentSavePath;
        bool m_loadLayoutWasRequested = false;
    };
} // namespace AudioControls
