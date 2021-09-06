/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <QDialog>

#include <EditorEngineInterop.h>

class QGridLayout;

namespace AudioControls
{
    class AudioFilterParamsWidget : public QDialog
    {
        Q_OBJECT

    public:
        explicit AudioFilterParamsWidget(QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioFilterParamsWidget() = default;

        void SetData(int32_t filterIndex, const Audio::FilterData& filterData);

    signals:
        void ParametersChanged(int32_t filterIndex, const AZStd::vector<float>& filterParamValues);

    private slots:
        void OnWidgetValueChanged();

    private:
        QGridLayout* m_mainLayout = nullptr;
        Audio::FilterData m_filterData;
        int32_t m_filterIndex = 0;
        bool isInUpdateWidgets = false;
    };
} // namespace AudioControls
