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
    class AudioFileToRtpcParamsWidget
        : public QWidget
        , private Ui::AudioFileToRtpcParamsWidget
    {
        Q_OBJECT

    public:
        explicit AudioFileToRtpcParamsWidget(TConnectionPtr connection, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioFileToRtpcParamsWidget() = default;

    signals:
        void PropertiesChanged();

    private slots:
        void OnRtpcTypeChanged(int index);
        void UpdateConnectionFromWidgets();

    private:
        void UpdateWidgetsFromConnection();

        TConnectionPtr m_connection;
        bool m_inUpdateWidgetsFromConnection = false;
    };
} // namespace AudioControls
