/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioFilterListItemWidget.h>

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QToolButton>

namespace AudioControls
{
    AudioFilterListItemWidget::AudioFilterListItemWidget(int32_t index, QWidget* parent, Qt::WindowFlags f)
        : QWidget(parent, f)
        , m_index(index)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);
        layout->setSpacing(0);

        m_filterEnabledCB = new QCheckBox(this);
        connect(m_filterEnabledCB, SIGNAL(toggled(bool)), this, SLOT(OnFilterEnabledCbToggled(bool)));
        m_filterEnabledCB->setDisabled(true);
        layout->addWidget(m_filterEnabledCB);

        m_filterNameLabel = new QLabel(this);
        m_filterNameLabel->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
        layout->addWidget(m_filterNameLabel);

        m_filterParamsButton = new QToolButton(this);
        m_filterParamsButton->setText("...");
        connect(m_filterParamsButton, SIGNAL(clicked(bool)), this, SLOT(OnFilterParamsButtonClicked()));
        layout->addWidget(m_filterParamsButton);

        m_setFilterButton = new QToolButton(this);
        m_setFilterButton->setArrowType(Qt::RightArrow);
        connect(m_setFilterButton, SIGNAL(clicked(bool)), this, SLOT(OnSetFilterButtonClicked()));
        layout->addWidget(m_setFilterButton);
    }

    void AudioFilterListItemWidget::SetFilterName(const QString& name)
    {
        m_filterNameLabel->setText(name);

        m_filterEnabledCB->blockSignals(true);
        m_filterEnabledCB->setChecked(false);
        m_filterEnabledCB->blockSignals(false);
        m_filterEnabledCB->setDisabled(name.isEmpty());
    }

    void AudioFilterListItemWidget::SetFilterEnabled(bool isEnabled)
    {
        if (!m_filterNameLabel->text().isEmpty())
        {
            m_filterEnabledCB->blockSignals(true);
            m_filterEnabledCB->setChecked(isEnabled);
            m_filterEnabledCB->blockSignals(false);
        }
    }

    void AudioFilterListItemWidget::OnFilterEnabledCbToggled(bool isEnabled)
    {
        emit FilterEnabledStateChanged(m_index, isEnabled);
    }

    void AudioFilterListItemWidget::OnFilterParamsButtonClicked()
    {
        emit FilterParamsButtonClicked(m_index);
    }

    void AudioFilterListItemWidget::OnSetFilterButtonClicked()
    {
        emit SetFilterButtonClicked(m_index);
    }
} // namespace AudioControls
