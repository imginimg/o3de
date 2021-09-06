/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioFilterParamsWidget.h>

#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QLabel>

namespace AudioControls
{
    AudioFilterParamsWidget::AudioFilterParamsWidget(QWidget* parent, Qt::WindowFlags f)
        : QDialog(parent, f)
    {
        m_mainLayout = new QGridLayout(this);
        m_mainLayout->setSizeConstraint(QLayout::SetFixedSize);
        setLayout(m_mainLayout);
    }

    void AudioFilterParamsWidget::SetData(int32_t filterIndex, const Audio::FilterData& filterData)
    {
        isInUpdateWidgets = true;

        m_filterIndex = filterIndex;
        m_filterData = filterData;

        {
            QObjectList chObjects = children();
            for (QObject* child : chObjects)
            {
                if (child != m_mainLayout)
                {
                    delete child;
                }
            }
        }

        for (size_t paramIndex = 0; paramIndex < filterData.m_params.size(); ++paramIndex)
        {
            const auto& paramData = filterData.m_params[paramIndex];

            auto paramLabel = new QLabel(paramData.m_name.GetCStr(), this);
            m_mainLayout->addWidget(paramLabel, aznumeric_cast<int>(paramIndex), 0);

            switch (paramData.m_type)
            {
                case Audio::FilterParamType::Float:
                {
                    auto editor = new QDoubleSpinBox(this);
                    editor->setRange(paramData.m_minValue, paramData.m_maxValue);
                    editor->setSingleStep((paramData.m_maxValue - paramData.m_minValue) / 20.0);
                    editor->setValue(paramData.m_value);
                    connect(editor, SIGNAL(valueChanged(double)), this, SLOT(OnWidgetValueChanged()));
                    m_mainLayout->addWidget(editor, aznumeric_cast<int>(paramIndex), 1);
                    break;
                }

                case Audio::FilterParamType::Int:
                {
                    auto editor = new QSpinBox(this);
                    editor->setRange(paramData.m_minValue, paramData.m_maxValue);
                    editor->setValue(paramData.m_value);
                    connect(editor, SIGNAL(valueChanged(int)), this, SLOT(OnWidgetValueChanged()));
                    m_mainLayout->addWidget(editor, aznumeric_cast<int>(paramIndex), 1);
                    break;
                }

                case Audio::FilterParamType::Bool:
                {
                    auto editor = new QCheckBox(this);
                    editor->setChecked(paramData.m_value);
                    connect(editor, SIGNAL(stateChanged(int)), this, SLOT(OnWidgetValueChanged()));
                    m_mainLayout->addWidget(editor, aznumeric_cast<int>(paramIndex), 1);
                    break;
                }
            }
        }

        isInUpdateWidgets = false;
    }

    void AudioFilterParamsWidget::OnWidgetValueChanged()
    {
        if (isInUpdateWidgets)
        {
            return;
        }

        for (size_t paramIndex = 0; paramIndex < m_filterData.m_params.size(); ++paramIndex)
        {
            auto& paramData = m_filterData.m_params[paramIndex];

            switch (paramData.m_type)
            {
                case Audio::FilterParamType::Float:
                {
                    auto widget = qobject_cast<QDoubleSpinBox*>(m_mainLayout->itemAtPosition(aznumeric_cast<int>(paramIndex), 1)->widget());
                    paramData.m_value = widget->value();
                    break;
                }

                case Audio::FilterParamType::Int:
                {
                    auto widget = qobject_cast<QSpinBox*>(m_mainLayout->itemAtPosition(aznumeric_cast<int>(paramIndex), 1)->widget());
                    paramData.m_value = widget->value();
                    break;
                }

                case Audio::FilterParamType::Bool:
                {
                    auto widget = qobject_cast<QCheckBox*>(m_mainLayout->itemAtPosition(aznumeric_cast<int>(paramIndex), 1)->widget());
                    paramData.m_value = widget->isChecked();
                    break;
                }
            }
        }

        AZStd::vector<float> paramValues(m_filterData.m_params.size());
        for (size_t i = 0; i < paramValues.size(); ++i)
        {
            paramValues[i] = m_filterData.m_params[i].m_value;
        }

        emit ParametersChanged(m_filterIndex, paramValues);
    }
} // namespace AudioControls
