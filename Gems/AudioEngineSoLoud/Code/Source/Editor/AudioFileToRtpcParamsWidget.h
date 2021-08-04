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
#include <ui_AudioFileToRtpcParamsWidget.h>

namespace AudioControls
{
    class CAudioFileToRtpcParamsWidget
        : public QWidget
        , private Ui::AudioFileToRtpcParamsWidget
    {
        Q_OBJECT

    public:
        explicit CAudioFileToRtpcParamsWidget(TConnectionPtr connection, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~CAudioFileToRtpcParamsWidget() = default;

    signals:
        void ParametersChanged();

    private slots:
        void OnRtpcTypeChanged(int index);
        void UpdateConnectionFromWidgets();

    private:
        void UpdateWidgetsFromConnection();

        TConnectionPtr m_connection;
        bool m_inUpdateWidgetsFromConnection = false;
    };
} // namespace AudioControls
