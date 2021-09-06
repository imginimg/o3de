/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <EditorEngineInterop.h>

#include <QFrame>

class QListWidget;
class QPushButton;
class QMenu;

namespace AudioControls
{
    class AudioFilterListWidget
        : public QFrame
        , private Audio::AudioBusManagerNotificationBus::Handler
    {
        Q_OBJECT

    public:
        explicit AudioFilterListWidget(QWidget* parent = nullptr, bool isBusMode = true, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioFilterListWidget();

        void SetData(AZ::Name busName, const Audio::FilterBlockData& filterBlock);
        const Audio::FilterBlockData& GetData() const;

    signals:
        void DataChanged();

    private slots:
        void OnFilterEnabledStateChanged(int32_t filterIndex, bool isEnabled);
        void OnFilterParamsButtonClicked(int32_t filterIndex);
        void OnSetFilterButtonClicked(int32_t filterIndex);
        void OnFilterParametersChanged(int32_t filterIndex, const AZStd::vector<float>& filterParamValues);

    private:
        // AudioBusManagerNotifications
        void OnRequestCompleted_ChangeAudioBusName(bool success, AZ::Name oldBusName, AZ::Name newBusName) override;
        void OnRequestCompleted_SetAudioBusFilter(
            bool success, AZ::Name busName, int32_t filterIndex, Audio::FilterData filterData) override;
        void OnRequestCompleted_SetAudioBusFilterEnabled(bool success, AZ::Name busName, int32_t filterIndex, bool isEnabled) override;
        void OnRequestCompleted_SetAudioBusFilterParams(
            bool success, AZ::Name busName, int32_t filterIndex, AZStd::vector<float> params) override;
        void OnRequestCompleted_SetAudioBusFilterParam(
            bool success, AZ::Name busName, int32_t filterIndex, AZ::Name paramName, int32_t paramIndex, float paramValue)
            override;
        void OnUpdateAudioFilters(AZStd::vector<Audio::FilterData> filterData) override;
        // ~AudioBusManagerNotifications

        QListWidget* m_listWidget = nullptr;
        QMenu* m_filterTypesMenu = nullptr;
        AZ::Name m_busName;
        Audio::FilterBlockData m_filterBlock;
        AZStd::vector<Audio::FilterData> m_allFiltersData;
        bool m_isBusMode = false;
    };
} // namespace AudioControls
