/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioConnections.h>
#include <AudioFileToTriggerParamsWidget.h>
#include <AudioFilterListWidget.h>

#include <QFormLayout>

namespace AudioControls
{
    AudioFileToTriggerParamsWidget::AudioFileToTriggerParamsWidget(TConnectionPtr connection, QWidget* parent, Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_connection(connection)
    {
        AZ_Assert(connection, "Connection must be a non-null pointer.");

        setupUi(this);
        AZ_Assert(m_actionCB->count() == Audio::AudioAction::Count, "m_actionCB has wrong number of items.");
        AZ_Assert(m_attenuationModeCB->count() == Audio::AttenuationMode::Count, "m_attenuationModeCB has wrong number of items.");
        AZ_Assert(m_inaudibleBehaviorCB->count() == Audio::InaudibleBehavior::Count, "m_inaudibleBehaviorCB has wrong number of items.");

        {
            auto font = QApplication::font();
            font.setPointSize(font.pointSize() + 1);
            m_filtersGB->setFont(font);
        }

        {
            m_filterListWidget = new AudioFilterListWidget(m_filtersGB, false);
            connect(m_filterListWidget, SIGNAL(DataChanged()), this, SLOT(UpdateConnectionFromWidgets()));
            auto layout = new QVBoxLayout(m_filtersGB);
            layout->addWidget(m_filterListWidget);
        }

        connect(m_actionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(OnActionChanged(int)));
        connect(m_actionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_audioBusCB, SIGNAL(currentTextChanged(const QString&)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_volumeDSB, SIGNAL(valueChanged(double)), this, SLOT(OnVolumeChanged()));
        connect(m_volumeSlider, &QSlider::valueChanged, m_volumeDSB, &QDoubleSpinBox::setValue);
        connect(m_loopingChB, SIGNAL(stateChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_positionalChB, SIGNAL(stateChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_attenuationModeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_attenuationRolloffFactorDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_minDistanceDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_maxDistanceDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_playSpeedDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_inaudibleBehaviorCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_protectedChB, SIGNAL(stateChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));

        UpdateWidgetsFromConnection();

        Audio::AudioBusManagerNotificationBus::Handler::BusConnect();
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::RequestAudioBusNames);
    }

    AudioFileToTriggerParamsWidget::~AudioFileToTriggerParamsWidget()
    {
        Audio::AudioBusManagerNotificationBus::Handler::BusDisconnect();
    }

    void AudioFileToTriggerParamsWidget::OnActionChanged(int index)
    {
        m_dependentWidgetsParent->setEnabled(index == Audio::AudioAction::Start);
    }

    void AudioFileToTriggerParamsWidget::OnVolumeChanged()
    {
        m_volumeSlider->blockSignals(true);
        m_volumeSlider->setValue(m_volumeDSB->value());
        m_volumeSlider->blockSignals(false);

        UpdateConnectionFromWidgets();
    }

    void AudioFileToTriggerParamsWidget::UpdateConnectionFromWidgets()
    {
        if (m_inUpdateWidgetsFromConnection)
        {
            return;
        }

        AudioFileToTriggerConnection* con = static_cast<AudioFileToTriggerConnection*>(m_connection.get());
        con->m_params.m_action = static_cast<Audio::AudioAction::Type>(m_actionCB->currentIndex());
        con->m_params.m_audioBusName = m_audioBusCB->currentText().toUtf8().data();
        con->m_params.m_volume = m_volumeDSB->value();
        con->m_params.m_looping = m_loopingChB->isChecked();
        con->m_params.m_positional = m_positionalChB->isChecked();
        con->m_params.m_attenuationMode = static_cast<Audio::AttenuationMode::Type>(m_attenuationModeCB->currentIndex());
        con->m_params.m_attenuationRolloffFactor = m_attenuationRolloffFactorDSB->value();
        con->m_params.m_minDistance = m_minDistanceDSB->value();
        con->m_params.m_maxDistance = m_maxDistanceDSB->value();
        con->m_params.m_filterBlock = m_filterListWidget->GetData();
        con->m_params.m_inaudibleBehavior = static_cast<Audio::InaudibleBehavior::Type>(m_inaudibleBehaviorCB->currentIndex());
        con->m_params.m_protected = m_protectedChB->isChecked();
        con->m_params.m_playSpeed = m_playSpeedDSB->value();

        emit PropertiesChanged();
    }

    void AudioFileToTriggerParamsWidget::OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames)
    {
        m_audioBusCB->blockSignals(true);
        m_audioBusCB->clear();

        for (const auto& name : busNames)
        {
            m_audioBusCB->addItem(name.GetCStr());
        }

        auto con = static_cast<AudioFileToTriggerConnection*>(m_connection.get());
        const AZ::Name& currentBusName = con->m_params.m_audioBusName;
        if (AZStd::find(busNames.begin(), busNames.end(), currentBusName) == busNames.end())
        {
            m_audioBusCB->addItem(currentBusName.GetCStr());
        }

        m_audioBusCB->setCurrentText(currentBusName.GetCStr());
        m_audioBusCB->blockSignals(false);
    }

    void AudioFileToTriggerParamsWidget::UpdateWidgetsFromConnection()
    {
        m_inUpdateWidgetsFromConnection = true;

        AudioFileToTriggerConnection* con = static_cast<AudioFileToTriggerConnection*>(m_connection.get());
        m_actionCB->setCurrentIndex(con->m_params.m_action);
        m_audioBusCB->setCurrentText(con->m_params.m_audioBusName.GetCStr());
        m_volumeDSB->setValue(con->m_params.m_volume);
        m_loopingChB->setChecked(con->m_params.m_looping);
        m_positionalChB->setChecked(con->m_params.m_positional);
        m_attenuationModeCB->setCurrentIndex(con->m_params.m_attenuationMode);
        m_attenuationRolloffFactorDSB->setValue(con->m_params.m_attenuationRolloffFactor);
        m_minDistanceDSB->setValue(con->m_params.m_minDistance);
        m_maxDistanceDSB->setValue(con->m_params.m_maxDistance);
        m_filterListWidget->SetData(AZ::Name(), con->m_params.m_filterBlock);
        m_inaudibleBehaviorCB->setCurrentIndex(con->m_params.m_inaudibleBehavior);
        m_protectedChB->setChecked(con->m_params.m_protected);
        m_playSpeedDSB->setValue(con->m_params.m_playSpeed);

        m_inUpdateWidgetsFromConnection = false;
    }
} // namespace AudioControls
