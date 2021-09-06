/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#include <QWidget>

class QLabel;
class QToolButton;
class QCheckBox;

namespace AudioControls
{
    class AudioFilterListItemWidget : public QWidget
    {
        Q_OBJECT

    public:
        AudioFilterListItemWidget(int32_t index, QWidget* parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());
        ~AudioFilterListItemWidget() = default;

        void SetFilterName(const QString& name);
        void SetFilterEnabled(bool isEnabled);

    signals:
        void FilterEnabledStateChanged(int32_t index, bool isEnabled);
        void FilterParamsButtonClicked(int32_t index);
        void SetFilterButtonClicked(int32_t index);

    private slots:
        void OnFilterEnabledCbToggled(bool isEnabled);
        void OnFilterParamsButtonClicked();
        void OnSetFilterButtonClicked();

    private:
        QLabel* m_filterNameLabel = nullptr;
        QCheckBox* m_filterEnabledCB = nullptr;
        QToolButton* m_filterParamsButton = nullptr;
        QToolButton* m_setFilterButton = nullptr;
        int32_t m_index = 0;
    };
} // namespace AudioControls
