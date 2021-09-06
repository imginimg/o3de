/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <PropertyLayoutFilePathCtrl.h>
#include <Config.h>

#include <QFileDialog>
#include <QHBoxLayout>

#include <AzCore/IO/FileIO.h>
#include <AzQtComponents/Components/Widgets/BrowseEdit.h>

namespace AudioControls
{
    PropertyLayoutFilePathCtrl::PropertyLayoutFilePathCtrl(QWidget* parent, Qt::WindowFlags flags)
        : QWidget(parent, flags)
    {
        QHBoxLayout* layout = new QHBoxLayout(this);
        layout->setContentsMargins(0, 0, 0, 0);

        m_browseEdit = new AzQtComponents::BrowseEdit(this);
        m_browseEdit->setClearButtonEnabled(true);
        layout->addWidget(m_browseEdit);

        connect(
            m_browseEdit, &AzQtComponents::BrowseEdit::attachedButtonTriggered, this,
            [this]()
            {
                QString filePath = QFileDialog::getOpenFileName(
                    this, "Open an audio bus layout", AZ::IO::FileIOBase::GetInstance()->GetAlias("@devassets@"),
                    QString("Audio bus layout (*.%1)").arg(Audio::BusLayoutFileExt));

                QDir dir(AZ::IO::FileIOBase::GetInstance()->GetAlias("@devassets@"));
                filePath = dir.relativeFilePath(filePath);

                if (!filePath.isEmpty())
                {
                    m_browseEdit->setText(filePath);
                    AZStd::string path = filePath.toUtf8().data();
                    emit ValueChanged(path);
                }
            });

        connect(
            m_browseEdit, &AzQtComponents::BrowseEdit::editingFinished, this,
            [this]()
            {
                AZStd::string path = m_browseEdit->text().toUtf8().data();
                emit ValueChanged(path);
            });
    }

    QWidget* PropertyLayoutFilePathCtrl::GetFirstInTabOrder()
    {
        return m_browseEdit;
    }

    QWidget* PropertyLayoutFilePathCtrl::GetLastInTabOrder()
    {
        return m_browseEdit;
    }

    void PropertyLayoutFilePathCtrl::SetValue(AZStd::string& value)
    {
        QString text = m_browseEdit->text();
        if (text.compare(value.data()) != 0)
        {
            m_browseEdit->blockSignals(true);
            m_browseEdit->setText(value.c_str());
            m_browseEdit->blockSignals(false);
        }
    }

    AZStd::string PropertyLayoutFilePathCtrl::GetValue() const
    {
        return m_browseEdit->text().toUtf8().data();
    }

    AZ::u32 PropertyLayoutFilePathHandler::GetHandlerName(void) const
    {
        return AZ_CRC("LayoutFilePath");
    }

    QWidget* PropertyLayoutFilePathHandler::GetFirstInTabOrder(PropertyLayoutFilePathCtrl* widget)
    {
        return widget->GetFirstInTabOrder();
    }

    QWidget* PropertyLayoutFilePathHandler::GetLastInTabOrder(PropertyLayoutFilePathCtrl* widget)
    {
        return widget->GetLastInTabOrder();
    }

    QWidget* PropertyLayoutFilePathHandler::CreateGUI(QWidget* parent)
    {
        PropertyLayoutFilePathCtrl* widget = aznew PropertyLayoutFilePathCtrl(parent);
        connect(
            widget, &PropertyLayoutFilePathCtrl::ValueChanged, this,
            [widget]()
            {
                using namespace AzToolsFramework;
                PropertyEditorGUIMessages::Bus::Broadcast(&PropertyEditorGUIMessages::Bus::Events::RequestWrite, widget);
            });
        return widget;
    }

    void PropertyLayoutFilePathHandler::WriteGUIValuesIntoProperty(
        size_t, PropertyLayoutFilePathCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode*)
    {
        AZStd::string val = GUI->GetValue();
        instance = static_cast<property_t>(val);
    }

    bool PropertyLayoutFilePathHandler::ReadValuesIntoGUI(
        size_t, PropertyLayoutFilePathCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode*)
    {
        AZStd::string val = instance;
        GUI->SetValue(val);
        return false;
    }

    void RegisterLayoutFilePathHandler()
    {
        using namespace AzToolsFramework;
        PropertyTypeRegistrationMessages::Bus::Broadcast(
            &PropertyTypeRegistrationMessages::Bus::Events::RegisterPropertyType, aznew PropertyLayoutFilePathHandler);
    }
} // namespace AudioControls
