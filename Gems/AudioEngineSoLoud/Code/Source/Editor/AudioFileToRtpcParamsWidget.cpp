/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioConnections.h>
#include <AudioFileToRtpcParamsWidget.h>

namespace AudioControls
{
    CAudioFileToRtpcParamsWidget::CAudioFileToRtpcParamsWidget(TConnectionPtr connection, QWidget* parent, Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_connection(connection)
    {
        AZ_Assert(connection, "Connection must be a non-null pointer.");

        setupUi(this);

        AZ_Assert(m_rtpcTypeCB->count() == Audio::EAudioFileRtpc::Count, "m_rtpcTypeCB has wrong number of items.");
        AZ_Assert(m_rtpcScopeCB->count() == 2, "m_rtpcScopeCB has wrong number of items.");

        connect(m_rtpcTypeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(OnRtpcTypeChanged(int)));
        connect(m_rtpcTypeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));
        connect(m_rtpcScopeCB, SIGNAL(currentIndexChanged(int)), this, SLOT(UpdateConnectionFromWidgets()));

        UpdateWidgetsFromConnection();
    }

    void CAudioFileToRtpcParamsWidget::OnRtpcTypeChanged(int index)
    {
        m_rtpcScopeCB->setEnabled(index == 0);
    }

    void CAudioFileToRtpcParamsWidget::UpdateConnectionFromWidgets()
    {
        if (m_inUpdateWidgetsFromConnection)
            return;

        CAudioFileToRtpcConnection* con = static_cast<CAudioFileToRtpcConnection*>(m_connection.get());
        con->m_params.m_type = static_cast<Audio::EAudioFileRtpc::Type>(m_rtpcTypeCB->currentIndex());
        con->m_params.m_perObject = m_rtpcScopeCB->currentIndex() == 1;

        emit ParametersChanged();
    }

    void CAudioFileToRtpcParamsWidget::UpdateWidgetsFromConnection()
    {
        m_inUpdateWidgetsFromConnection = true;

        CAudioFileToRtpcConnection* con = static_cast<CAudioFileToRtpcConnection*>(m_connection.get());
        m_rtpcTypeCB->setCurrentIndex(con->m_params.m_type);
        con->m_params.m_perObject ? m_rtpcScopeCB->setCurrentIndex(1) : m_rtpcScopeCB->setCurrentIndex(0);

        m_inUpdateWidgetsFromConnection = false;
    }
} // namespace AudioControls
