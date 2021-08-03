/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <ACETypes.h>
#include <QWidget>
#include <ui_AudioFileToTriggerParamsWidget.h>

namespace AudioControls
{
    class CAudioFileToTriggerParamsWidget
        : public QWidget
        , private Ui::AudioFileToTriggerParamsWidget
    {
        Q_OBJECT

    public:
        explicit CAudioFileToTriggerParamsWidget(TConnectionPtr connection, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~CAudioFileToTriggerParamsWidget() = default;

    signals:
        void ParametersChanged();

    private slots:
        void OnActionChanged(int index);
        void UpdateConnectionFromWidgets();

    private:
        void UpdateWidgetsFromConnection();

        TConnectionPtr m_connection;
        bool m_inUpdateWidgetsFromConnection = false;
    };
} // namespace AudioControls
