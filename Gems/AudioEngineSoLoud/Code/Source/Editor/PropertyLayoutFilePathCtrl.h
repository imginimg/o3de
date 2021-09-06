/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#pragma once

#if !defined(Q_MOC_RUN)
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/std/string/string.h>
#include <AzToolsFramework/UI/PropertyEditor/PropertyEditorAPI.h>

#include <QWidget>
#endif

class QLineEdit;
class QToolButton;

namespace AzQtComponents
{
    class BrowseEdit;
}

namespace AudioControls
{
    class PropertyLayoutFilePathCtrl : public QWidget
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(PropertyLayoutFilePathCtrl, AZ::SystemAllocator, 0);

        explicit PropertyLayoutFilePathCtrl(QWidget* parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
        ~PropertyLayoutFilePathCtrl() = default;

        QWidget* GetFirstInTabOrder();
        QWidget* GetLastInTabOrder();

        void SetValue(AZStd::string& value);
        AZStd::string GetValue() const;

    signals:
        void ValueChanged(AZStd::string& newValue);

    private:
        AzQtComponents::BrowseEdit* m_browseEdit = nullptr;
    };

    class PropertyLayoutFilePathHandler
        : QObject
        , public AzToolsFramework::PropertyHandler<AZStd::string, PropertyLayoutFilePathCtrl>
    {
        Q_OBJECT

    public:
        AZ_CLASS_ALLOCATOR(PropertyLayoutFilePathHandler, AZ::SystemAllocator, 0);

        AZ::u32 GetHandlerName() const override;
        QWidget* GetFirstInTabOrder(PropertyLayoutFilePathCtrl* widget) override;
        QWidget* GetLastInTabOrder(PropertyLayoutFilePathCtrl* widget) override;

        QWidget* CreateGUI(QWidget* parent) override;
        void WriteGUIValuesIntoProperty(
            size_t index, PropertyLayoutFilePathCtrl* GUI, property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
        bool ReadValuesIntoGUI(
            size_t index, PropertyLayoutFilePathCtrl* GUI, const property_t& instance, AzToolsFramework::InstanceDataNode* node) override;
    };

    void RegisterLayoutFilePathHandler();
} // namespace AudioControls
