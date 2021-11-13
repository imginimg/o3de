/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioBusManagerWindow.h>
#include <AudioBusWidget.h>

#include <AzCore/IO/FileIO.h>
#include <AzCore/std/string/conversions.h>

#include <QFileDialog>
#include <QHBoxLayout>
#include <QMenu>
#include <QMouseEvent>

namespace AudioControls
{
    AudioBusManagerWindow::AudioBusManagerWindow(QWidget* parent, Qt::WindowFlags f)
        : QMainWindow(parent, f)
    {
        setupUi(this);

        connect(m_resetAction, SIGNAL(triggered()), this, SLOT(OnResetClicked()));
        connect(m_loadAction, SIGNAL(triggered()), this, SLOT(OnLoadClicked()));
        connect(m_saveAction, SIGNAL(triggered()), this, SLOT(OnSaveClicked()));
        connect(m_saveAsAction, SIGNAL(triggered()), this, SLOT(OnSaveAsClicked()));
        connect(m_addBusAction, SIGNAL(triggered()), this, SLOT(OnAddBusClicked()));
        connect(m_removeBusAction, SIGNAL(triggered()), this, SLOT(OnRemoveBusClicked()));

        connect(m_addBusPB, SIGNAL(clicked(bool)), this, SLOT(OnAddBusClicked()));
        connect(m_removeBusPB, SIGNAL(clicked(bool)), this, SLOT(OnRemoveBusClicked()));

        connect(this, &AudioBusManagerWindow::customContextMenuRequested, this, &AudioBusManagerWindow::OnCustomContextMenuRequested);
        m_contextMenu = new QMenu(this);
        m_contextMenu->addAction(m_removeBusAction);

        m_busWidgetsLayout = new QHBoxLayout(m_busParentWidget);
        m_busParentWidget->setLayout(m_busWidgetsLayout);

        m_fileNameLabel->setVisible(false);

        Audio::AudioBusManagerNotificationBus::Handler::BusConnect();
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::RequestAudioBusLayout);
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetVisualizationEnabled, true);
    }

    AudioBusManagerWindow::~AudioBusManagerWindow()
    {
        Audio::AudioBusManagerNotificationBus::Handler::BusDisconnect();
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetVisualizationEnabled, false);
    }

    const GUID& AudioBusManagerWindow::GetClassID()
    {
        static const GUID guid = { 0xb7f5541e, 0x55eb, 0x417a, { 0xaa, 0x6b, 0x4, 0xdc, 0xef, 0x5, 0x2d, 0x59 } };
        return guid;
    }

    void AudioBusManagerWindow::timerEvent(QTimerEvent*)
    {
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::RequestAudioBusPeakVolumes);
    }

    void AudioBusManagerWindow::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton)
        {
            m_removeBusPB->setDisabled(true);
            m_selectedWidget = nullptr;

            int widgetIndex = 0;
            for (; widgetIndex < m_busWidgetsLayout->count(); ++widgetIndex)
            {
                auto widget = qobject_cast<AudioBusWidget*>(m_busWidgetsLayout->itemAt(widgetIndex)->widget());
                if (!widget)
                {
                    continue;
                }

                QPoint cursorPos = m_busWidgetsScrollArea->widget()->mapFromGlobal(event->globalPos());
                bool widgetSelected = widget->GetBusName() != AZ::Name(Audio::MasterBusName) && widget->geometry().contains(cursorPos);
                widget->SetSelected(widgetSelected);

                if (widgetSelected)
                {
                    m_removeBusPB->setEnabled(true);
                    m_selectedWidget = widget;
                }
            }

            if (widgetIndex < m_busWidgetsLayout->count())
            {
                event->accept();
            }
        }

        QWidget::mousePressEvent(event);
    }

    void AudioBusManagerWindow::OnRemoveBusClicked()
    {
        if (!m_selectedWidget)
        {
            return;
        }

        Audio::AudioBusManagerRequestBus::QueueBroadcast(
            &Audio::AudioBusManagerRequestBus::Events::RemoveAudioBus, m_selectedWidget->GetBusName());

        m_removeBusPB->setDisabled(true);
        m_selectedWidget = nullptr;
    }

    void AudioBusManagerWindow::OnResetClicked()
    {
        SetCurrentSavePath(QString());
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::ResetAudioBusLayout);
    }

    void AudioBusManagerWindow::OnSaveClicked()
    {
        SaveLayoutFile(false);
    }

    void AudioBusManagerWindow::OnSaveAsClicked()
    {
        SaveLayoutFile();
    }

    void AudioBusManagerWindow::OnLoadClicked()
    {
        QString path = QFileDialog::getOpenFileName(
            this, "Open an audio bus layout", AZ::IO::FileIOBase::GetInstance()->GetAlias("@devassets@"),
            QString("Audio bus layout (*.%1)").arg(Audio::BusLayoutFileExt));

        if (path.isEmpty())
        {
            return;
        }

        SetCurrentSavePath(path);
        m_loadLayoutWasRequested = true;

        Audio::AudioBusManagerRequestBus::QueueBroadcast(
            &Audio::AudioBusManagerRequestBus::Events::LoadAudioBusLayout, path.toUtf8().data());
    }

    void AudioBusManagerWindow::OnCustomContextMenuRequested(const QPoint& pos)
    {
        if (m_selectedWidget)
        {
            m_contextMenu->exec(mapToGlobal(pos));
        }
    }

    void AudioBusManagerWindow::OnAddBusClicked()
    {
        const AZStd::string namePrefix = "Bus";
        AZStd::string name;

        const int maxTryCount = 500;
        int i = 0;
        for (; i < maxTryCount; ++i)
        {
            name = (i == 0) ? namePrefix : namePrefix + AZStd::to_string(i);
            if (AZStd::find(m_busNames.begin(), m_busNames.end(), AZ::Name(name)) == m_busNames.end())
            {
                break;
            }
        }
        AZ_Assert(i != maxTryCount, "");

        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::AddAudioBus, AZ::Name(name));
    }

    void AudioBusManagerWindow::OnRequestCompleted_AddAudioBus(bool success, Audio::BusData busData)
    {
        if (!success)
        {
            return;
        }

        auto widget = new AudioBusWidget(m_busParentWidget);
        m_busWidgetsLayout->insertWidget(m_busWidgetsLayout->count() - 1, widget);

        widget->SetData(busData, aznumeric_cast<int32_t>(m_busNames.size()));
    }

    void AudioBusManagerWindow::OnRequestCompleted_RemoveAudioBus(bool success, AZ::Name busName)
    {
        if (!success)
        {
            return;
        }

        for (int i = 0; i < m_busWidgetsLayout->count() - 1; ++i)
        {
            auto widget = qobject_cast<AudioBusWidget*>(m_busWidgetsLayout->itemAt(i)->widget());
            AZ_Assert(widget, "");

            if (widget->GetBusName() == busName)
            {
                m_busWidgetsLayout->removeWidget(widget);
                widget->deleteLater();
            }
        }
    }

    void AudioBusManagerWindow::OnRequestCompleted_ChangeAudioBusIndex(
        bool success, AZ::Name busName, int32_t oldBusIndex, int32_t newBusIndex)
    {
        if (!success)
        {
            return;
        }

        QLayoutItem* item = m_busWidgetsLayout->itemAt(oldBusIndex);
        m_busWidgetsLayout->removeItem(item);
        m_busWidgetsLayout->insertItem(newBusIndex, item);
    }

    void AudioBusManagerWindow::OnUpdateAudioBusLayout(Audio::BusLayoutData busLayout)
    {
        if (m_loadLayoutWasRequested)
        {
            m_loadLayoutWasRequested = false;
        }
        else
        {
            SetCurrentSavePath(QString());
        }

        DestroyAudioBusWidgets();

        for (size_t i = 0; i < busLayout.m_buses.size(); ++i)
        {
            auto widget = new AudioBusWidget(m_busParentWidget);
            m_busWidgetsLayout->addWidget(widget);
            widget->SetData(busLayout.m_buses[i], aznumeric_cast<int32_t>(i));
        }

        m_busWidgetsLayout->addItem(new QSpacerItem(0, 0, QSizePolicy::Expanding));

        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::RequestAudioBusNames);
        startTimer(RequestBusPeakVolumesIntervalMs);
    }

    void AudioBusManagerWindow::OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames)
    {
        m_busNames = busNames;
    }

    void AudioBusManagerWindow::SetCurrentSavePath(const QString& path)
    {
        m_currentSavePath = path;

        if (path.isEmpty())
        {
            m_fileNameLabel->setVisible(false);
        }
        else
        {
            m_fileNameLabel->setText(QString("Layout: %1").arg(QFileInfo(m_currentSavePath).baseName()));
            m_fileNameLabel->setVisible(true);
        }
    }

    void AudioBusManagerWindow::SaveLayoutFile(bool saveAs)
    {
        if (saveAs || m_currentSavePath.isEmpty())
        {
            QString path = QFileDialog::getSaveFileName(
                this, "Save the audio bus layout", AZ::IO::FileIOBase::GetInstance()->GetAlias("@devassets@"),
                QString("Audio bus layout (*.%1)").arg(Audio::BusLayoutFileExt));

            if (!path.isEmpty())
            {
                SetCurrentSavePath(path);
            }
        }

        if (!m_currentSavePath.isEmpty())
        {
            Audio::AudioBusManagerRequestBus::QueueBroadcast(
                &Audio::AudioBusManagerRequestBus::Events::SaveAudioBusLayout, m_currentSavePath.toUtf8().data());
        }
    }

    void AudioBusManagerWindow::DestroyAudioBusWidgets()
    {
        while (auto item = m_busWidgetsLayout->takeAt(0))
        {
            item->widget()->deleteLater();
            delete item;
        }
    }
} // namespace AudioControls
