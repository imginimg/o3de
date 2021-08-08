/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioConnections.h>
#include <AudioFileToTriggerParamsWidget.h>

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

        connect(m_actionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(OnActionChanged(int)));
        connect(m_actionCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_volumeDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_loopingChB, SIGNAL(stateChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_positionalChB, SIGNAL(stateChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_attenuationModeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_attenuationRolloffFactorDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_minDistanceDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_maxDistanceDSB, SIGNAL(editingFinished()), this, SLOT(UpdateConnectionFromWidgets()));

        UpdateWidgetsFromConnection();
    }

    void AudioFileToTriggerParamsWidget::OnActionChanged(int index)
    {
        m_dependentWidgetsParent->setEnabled(index == Audio::AudioAction::Start);
    }

    void AudioFileToTriggerParamsWidget::UpdateConnectionFromWidgets()
    {
        if (m_inUpdateWidgetsFromConnection)
        {
            return;
        }

        AudioFileToTriggerConnection* con = static_cast<AudioFileToTriggerConnection*>(m_connection.get());
        con->m_params.m_action = static_cast<Audio::AudioAction::Type>(m_actionCB->currentIndex());
        con->m_params.m_volume = m_volumeDSB->value();
        con->m_params.m_looping = m_loopingChB->isChecked();
        con->m_params.m_positional = m_positionalChB->isChecked();
        con->m_params.m_attenuationMode = static_cast<Audio::AttenuationMode::Type>(m_attenuationModeCB->currentIndex());
        con->m_params.m_attenuationRolloffFactor = m_attenuationRolloffFactorDSB->value();
        con->m_params.m_minDistance = m_minDistanceDSB->value();
        con->m_params.m_maxDistance = m_maxDistanceDSB->value();

        emit PropertiesChanged();
    }

    void AudioFileToTriggerParamsWidget::UpdateWidgetsFromConnection()
    {
        m_inUpdateWidgetsFromConnection = true;

        AudioFileToTriggerConnection* con = static_cast<AudioFileToTriggerConnection*>(m_connection.get());
        m_actionCB->setCurrentIndex(con->m_params.m_action);
        m_volumeDSB->setValue(con->m_params.m_volume);
        m_loopingChB->setChecked(con->m_params.m_looping);
        m_positionalChB->setChecked(con->m_params.m_positional);
        m_attenuationModeCB->setCurrentIndex(con->m_params.m_attenuationMode);
        m_attenuationRolloffFactorDSB->setValue(con->m_params.m_attenuationRolloffFactor);
        m_minDistanceDSB->setValue(con->m_params.m_minDistance);
        m_maxDistanceDSB->setValue(con->m_params.m_maxDistance);

        m_inUpdateWidgetsFromConnection = false;
    }
} // namespace AudioControls
