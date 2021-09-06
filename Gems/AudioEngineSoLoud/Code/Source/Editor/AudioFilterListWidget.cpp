/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioFilterListItemWidget.h>
#include <AudioFilterListWidget.h>
#include <AudioFilterParamsWidget.h>

#include <QBoxLayout>
#include <QCursor>
#include <QListWidget>
#include <QMenu>
#include <QPushButton>

namespace AudioControls
{
    AudioFilterListWidget::AudioFilterListWidget(QWidget* parent, bool isBusMode, Qt::WindowFlags f)
        : QFrame(parent, f)
        , m_isBusMode(isBusMode)
    {
        if (m_isBusMode)
        {
            setFrameStyle(QFrame::Panel);
            setLineWidth(2);
        }

        QHBoxLayout* layout = new QHBoxLayout(this);
        setLayout(layout);

        m_listWidget = new QListWidget(this);
        m_listWidget->setSelectionRectVisible(false);
        m_listWidget->setSelectionMode(QAbstractItemView::NoSelection);
        m_listWidget->setAlternatingRowColors(true);
        layout->addWidget(m_listWidget);

        for (int32_t i = 0; i < Audio::NumberOfFiltersPerBus; ++i)
        {
            AudioFilterListItemWidget* widget = new AudioFilterListItemWidget(i, this);
            connect(widget, SIGNAL(FilterEnabledStateChanged(int32_t, bool)), this, SLOT(OnFilterEnabledStateChanged(int32_t, bool)));
            connect(widget, SIGNAL(FilterParamsButtonClicked(int32_t)), this, SLOT(OnFilterParamsButtonClicked(int32_t)));
            connect(widget, SIGNAL(SetFilterButtonClicked(int32_t)), this, SLOT(OnSetFilterButtonClicked(int32_t)));
            m_listWidget->addItem("");
            m_listWidget->setItemWidget(m_listWidget->item(m_listWidget->count() - 1), widget);
        }

        m_filterTypesMenu = new QMenu(this);

        Audio::AudioBusManagerNotificationBus::Handler::BusConnect();
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::RequestAudioFilters);
    }

    AudioFilterListWidget::~AudioFilterListWidget()
    {
        Audio::AudioBusManagerNotificationBus::Handler::BusDisconnect();
    }

    void AudioFilterListWidget::SetData(AZ::Name busName, const Audio::FilterBlockData& filterBlock)
    {
        m_busName = busName;
        m_filterBlock = filterBlock;

        for (int32_t i = 0; i < Audio::NumberOfFiltersPerBus; ++i)
        {
            auto widget = qobject_cast<AudioFilterListItemWidget*>(m_listWidget->itemWidget(m_listWidget->item(i)));

            if (i < filterBlock.m_filters.size())
            {
                widget->SetFilterName(filterBlock.m_filters[i].m_name.GetCStr());
                widget->SetFilterEnabled(filterBlock.m_filters[i].m_isEnabled);
            }
            else
            {
                widget->SetFilterName("");
                widget->SetFilterEnabled(false);
            }
        }
    }

    const Audio::FilterBlockData& AudioFilterListWidget::GetData() const
    {
        return m_filterBlock;
    }

    void AudioFilterListWidget::OnFilterEnabledStateChanged(int32_t filterIndex, bool isEnabled)
    {
        if (m_isBusMode)
        {
            Audio::AudioBusManagerRequestBus::QueueBroadcast(
                &Audio::AudioBusManagerRequestBus::Events::SetAudioBusFilterEnabled, m_busName, filterIndex, isEnabled);
        }
        else
        {
            m_filterBlock.m_filters[filterIndex].m_isEnabled = isEnabled;
            emit DataChanged();
        }
    }

    void AudioFilterListWidget::OnFilterParamsButtonClicked(int32_t filterIndex)
    {
        if (m_filterBlock.m_filters[filterIndex].m_name.IsEmpty())
        {
            return;
        }

        AudioFilterParamsWidget paramsWidget(this);
        paramsWidget.setWindowTitle("Filter params");
        paramsWidget.SetData(filterIndex, m_filterBlock.m_filters[filterIndex]);
        connect(
            &paramsWidget, SIGNAL(ParametersChanged(int32_t, const AZStd::vector<float>&)), this,
            SLOT(OnFilterParametersChanged(int32_t, const AZStd::vector<float>&)));
        paramsWidget.exec();
    }

    void AudioFilterListWidget::OnSetFilterButtonClicked(int32_t filterIndex)
    {
        QAction* selectedAction = m_filterTypesMenu->exec(QCursor::pos());
        if (!selectedAction)
        {
            return;
        }

        AZ::Name filterName((selectedAction->data().toInt() == 0) ? "" : selectedAction->text().toUtf8().data());

        if (m_isBusMode)
        {
            Audio::AudioBusManagerRequestBus::QueueBroadcast(
                &Audio::AudioBusManagerRequestBus::Events::SetAudioBusFilter, m_busName, filterIndex, filterName);
        }
        else
        {
            auto widget = qobject_cast<AudioFilterListItemWidget*>(m_listWidget->itemWidget(m_listWidget->item(filterIndex)));
            widget->SetFilterName(filterName.GetCStr());
            widget->SetFilterEnabled(m_filterBlock.m_filters[filterIndex].m_isEnabled);

            if (filterName.IsEmpty())
            {
                m_filterBlock.m_filters[filterIndex] = Audio::FilterData();
            }
            else
            {
                auto pred = [&filterName](const Audio::FilterData& fdata)
                {
                    return fdata.m_name == filterName;
                };
                auto it = AZStd::find_if(m_allFiltersData.begin(), m_allFiltersData.end(), pred);
                if (it != m_allFiltersData.end())
                {
                    m_filterBlock.m_filters[filterIndex] = *it;
                }
            }

            emit DataChanged();
        }
    }

    void AudioFilterListWidget::OnFilterParametersChanged(int32_t filterIndex, const AZStd::vector<float>& filterParamValues)
    {
        if (m_isBusMode)
        {
            Audio::AudioBusManagerRequestBus::QueueBroadcast(
                &Audio::AudioBusManagerRequestBus::Events::SetAudioBusFilterParams, m_busName, filterIndex, filterParamValues);
        }
        else
        {
            for (size_t i = 0; i < filterParamValues.size(); ++i)
            {
                m_filterBlock.m_filters[filterIndex].m_params[i].m_value = filterParamValues[i];
            }

            emit DataChanged();
        }
    }

    void AudioFilterListWidget::OnRequestCompleted_ChangeAudioBusName(bool success, AZ::Name oldBusName, AZ::Name newBusName)
    {
        if (m_isBusMode && oldBusName == m_busName && success)
        {
            m_busName = newBusName;
        }
    }

    void AudioFilterListWidget::OnRequestCompleted_SetAudioBusFilter(
        bool success, AZ::Name busName, int32_t filterIndex, Audio::FilterData filterData)
    {
        if (m_isBusMode && busName == m_busName && success)
        {
            m_filterBlock.m_filters[filterIndex] = filterData;

            auto widget = qobject_cast<AudioFilterListItemWidget*>(m_listWidget->itemWidget(m_listWidget->item(filterIndex)));
            widget->SetFilterName(filterData.m_name.GetCStr());
            widget->SetFilterEnabled(m_filterBlock.m_filters[filterIndex].m_isEnabled);
        }
    }

    void AudioFilterListWidget::OnRequestCompleted_SetAudioBusFilterEnabled(
        bool success, AZ::Name busName, int32_t filterIndex, bool isEnabled)
    {
        if (m_isBusMode && busName == m_busName && success)
        {
            m_filterBlock.m_filters[filterIndex].m_isEnabled = isEnabled;
            auto widget = qobject_cast<AudioFilterListItemWidget*>(m_listWidget->itemWidget(m_listWidget->item(filterIndex)));
            widget->SetFilterEnabled(isEnabled);
        }
    }

    void AudioFilterListWidget::OnRequestCompleted_SetAudioBusFilterParams(
        bool success, AZ::Name busName, int32_t filterIndex, AZStd::vector<float> params)
    {
        if (m_isBusMode && busName == m_busName && success)
        {
            for (size_t i = 0; i < params.size(); ++i)
            {
                m_filterBlock.m_filters[filterIndex].m_params[i].m_value = params[i];
            }
        }
    }

    void AudioFilterListWidget::OnRequestCompleted_SetAudioBusFilterParam(
        bool success, AZ::Name busName, int32_t filterIndex, AZ::Name paramName, int32_t paramIndex, float paramValue)
    {
        if (m_isBusMode && busName == m_busName && success)
        {
            m_filterBlock.m_filters[filterIndex].m_params[paramIndex].m_value = paramValue;
        }
    }

    void AudioFilterListWidget::OnUpdateAudioFilters(AZStd::vector<Audio::FilterData> filterData)
    {
        m_filterTypesMenu->clear();
        m_filterTypesMenu->addAction("[Remove filter]");

        for (size_t i = 0; i < filterData.size(); ++i)
        {
            QAction* action = m_filterTypesMenu->addAction(filterData[i].m_name.GetCStr());
            action->setData(i + 1);
        }

        if (!m_isBusMode)
        {
            m_allFiltersData = filterData;
        }
    }
} // namespace AudioControls
