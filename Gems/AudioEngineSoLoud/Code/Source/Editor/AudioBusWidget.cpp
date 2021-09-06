/*
 * Copyright (c) Contributors to the Open 3D Engine Project.
 * For complete copyright and license terms please see the LICENSE at the root of this distribution.
 *
 * SPDX-License-Identifier: Apache-2.0 OR MIT
 *
 */

#include <AudioBusWidget.h>
#include <AudioFilterListWidget.h>
#include <Util.h>
#include <VuMeterWidget.h>

#include <AzQtComponents/Components/Widgets/CheckBox.h>

#include <QDrag>
#include <QMessageBox>
#include <QMimeData>
#include <QMouseEvent>

namespace AudioControls
{
    AudioBusWidget::AudioBusWidget(QWidget* parent, Qt::WindowFlags f)
        : QFrame(parent, f)
    {
        setupUi(this);
        setLineWidth(2);
        AzQtComponents::CheckBox::applyToggleSwitchStyle(m_monoChB);
        AzQtComponents::CheckBox::applyToggleSwitchStyle(m_muteChB);

        {
            auto font = QApplication::font();
            font.setPointSize(font.pointSize() + 1);
            m_volumeGB->setFont(font);
        }

        {
            auto vuMeterWidget = new VuMeterWidget(this);
            vuMeterWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

            QLayoutItem* item = m_volumeWidgetsLayout->replaceWidget(m_vuMeterWidget, vuMeterWidget);
            delete item;
            delete m_vuMeterWidget;
            m_vuMeterWidget = vuMeterWidget;
        }

        m_filterListWidget = new AudioFilterListWidget(this);
        layout()->addWidget(m_filterListWidget);

        connect(m_busNameLineEdit, SIGNAL(editingFinished()), this, SLOT(OnBusNameChanged()));
        connect(m_volumeDsb, SIGNAL(valueChanged(double)), this, SLOT(OnVolumeChanged(double)));
        connect(m_volumeSlider, SIGNAL(valueChanged(int)), this, SLOT(OnVolumeChanged(int)));
        connect(m_muteChB, SIGNAL(toggled(bool)), this, SLOT(OnMuteStateChanged(bool)));
        connect(m_monoChB, SIGNAL(toggled(bool)), this, SLOT(OnMonoStateChanged(bool)));
        connect(m_outputBusCB, SIGNAL(currentTextChanged(const QString&)), this, SLOT(OnOutputBusNameChanged(const QString&)));

        Audio::AudioBusManagerNotificationBus::Handler::BusConnect();
    }

    AudioBusWidget::~AudioBusWidget()
    {
        Audio::AudioBusManagerNotificationBus::Handler::BusDisconnect();
    }

    void AudioBusWidget::SetData(const Audio::BusData& busData, int32_t busIndex)
    {
        AZ_Assert(!busData.m_name.IsEmpty(), "");
        AZ_Assert(m_busIndex >= 0, "");

        m_busName = busData.m_name;
        m_outputBusName = busData.m_outputBusName;
        m_busIndex = busIndex;

        BlockSignals(true);

        m_busNameLineEdit->setText(m_busName.GetCStr());
        m_busNameLineEdit->setReadOnly(busData.m_name == AZ::Name(Audio::MasterBusName));
        m_volumeDsb->setValue(busData.m_volume);
        m_volumeSlider->setValue(busData.m_volume);
        m_muteChB->setChecked(busData.m_isMuted);
        m_monoChB->setChecked(busData.m_isMono);
        m_outputBusCB->setDisabled(busData.m_name == AZ::Name(Audio::MasterBusName));
        m_outputBusCB->setCurrentText(busData.m_outputBusName.GetCStr());

        BlockSignals(false);

        m_filterListWidget->SetData(m_busName, busData.m_filterBlock);
    }

    void AudioBusWidget::SetSelected(bool isSelected)
    {
        setFrameStyle(isSelected ? QFrame::Panel : QFrame::NoFrame);
    }

    const AZ::Name& AudioBusWidget::GetBusName() const
    {
        return m_busName;
    }

    void AudioBusWidget::mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton && m_busName != AZ::Name(Audio::MasterBusName))
        {
            m_dragStartPosition = event->pos();
        }

        QFrame::mousePressEvent(event);
    }

    void AudioBusWidget::mouseMoveEvent(QMouseEvent* event)
    {
        if ((event->buttons() & Qt::LeftButton) && m_busName != AZ::Name(Audio::MasterBusName) &&
            (event->pos() - m_dragStartPosition).manhattanLength() >= QApplication::startDragDistance())
        {
            QDrag* drag = new QDrag(this);

            QMimeData* mimeData = new QMimeData;
            mimeData->setText(m_busName.GetCStr());
            drag->setMimeData(mimeData);

            QPixmap pixmap = grab();
            drag->setPixmap(pixmap);
            drag->exec();

            event->accept();
        }

        QFrame::mouseMoveEvent(event);
    }

    void AudioBusWidget::dragEnterEvent(QDragEnterEvent* event)
    {
        if (event->source() != this && m_busName != AZ::Name(Audio::MasterBusName))
        {
            event->acceptProposedAction();
        }

        QFrame::dragEnterEvent(event);
    }

    void AudioBusWidget::dropEvent(QDropEvent* event)
    {
        if (event->source() != this)
        {
            AZ::Name busNameToReposition(event->mimeData()->text().toUtf8().data());
            Audio::AudioBusManagerRequestBus::QueueBroadcast(
                &Audio::AudioBusManagerRequestBus::Events::ChangeAudioBusIndex, busNameToReposition, m_busIndex);

            event->acceptProposedAction();
        }

        QFrame::dropEvent(event);
    }

    void AudioBusWidget::OnBusNameChanged()
    {
        AZ::Name newName(m_busNameLineEdit->text().toUtf8().data());

        if (newName == m_busName)
        {
            return;
        }

        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::ChangeAudioBusName, m_busName, newName);
    }

    void AudioBusWidget::OnMuteStateChanged(bool value)
    {
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetAudioBusMuted, m_busName, value);
    }

    void AudioBusWidget::OnMonoStateChanged(bool value)
    {
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetAudioBusMono, m_busName, value);
    }

    void AudioBusWidget::OnOutputBusNameChanged(const QString& newBusName)
    {
        AZ::Name newOutputName(newBusName.toUtf8().data());
        Audio::AudioBusManagerRequestBus::QueueBroadcast(
            &Audio::AudioBusManagerRequestBus::Events::SetAudioBusOutput, m_busName, newOutputName);
    }

    void AudioBusWidget::OnVolumeChanged(int value)
    {
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetAudioBusVolumeDb, m_busName, value);
    }

    void AudioBusWidget::OnVolumeChanged(double value)
    {
        Audio::AudioBusManagerRequestBus::QueueBroadcast(&Audio::AudioBusManagerRequestBus::Events::SetAudioBusVolumeDb, m_busName, value);
    }

    void AudioBusWidget::OnRequestCompleted_ChangeAudioBusName(bool success, AZ::Name oldBusName, AZ::Name newBusName)
    {
        if (m_busName == oldBusName)
        {
            m_busNameLineEdit->blockSignals(true);

            if (success)
            {
                m_busName = newBusName;
                m_busNameLineEdit->setText(newBusName.GetCStr());
            }
            else
            {
                m_busNameLineEdit->setText(oldBusName.GetCStr());
            }

            m_busNameLineEdit->blockSignals(false);
        }
        else if (m_outputBusName == oldBusName && success)
        {
            m_outputBusName = newBusName;

            m_outputBusCB->blockSignals(true);
            m_outputBusCB->setCurrentText(newBusName.GetCStr());
            m_outputBusCB->blockSignals(false);
        }
    }

    void AudioBusWidget::OnRequestCompleted_ChangeAudioBusIndex(bool success, AZ::Name busName, int32_t, int32_t newBusIndex)
    {
        if (m_busName == busName && success)
        {
            m_busIndex = newBusIndex;
        }
    }

    void AudioBusWidget::OnRequestCompleted_SetAudioBusVolumeDb(bool success, AZ::Name busName, float volume)
    {
        if (busName == m_busName && success)
        {
            m_volumeSlider->blockSignals(true);
            m_volumeDsb->blockSignals(true);
            m_volumeSlider->setValue(volume);
            m_volumeDsb->setValue(volume);
            m_volumeSlider->blockSignals(false);
            m_volumeDsb->blockSignals(false);
        }
    }

    void AudioBusWidget::OnRequestCompleted_SetAudioBusVolumeLinear(bool success, AZ::Name busName, float volume)
    {
        if (busName == m_busName && success)
        {
            m_volumeSlider->blockSignals(true);
            m_volumeDsb->blockSignals(true);
            m_volumeSlider->setValue(Audio::LinearToDb(volume));
            m_volumeDsb->setValue(Audio::LinearToDb(volume));
            m_volumeSlider->blockSignals(false);
            m_volumeDsb->blockSignals(false);
        }
    }

    void AudioBusWidget::OnRequestCompleted_SetAudioBusMuted(bool success, AZ::Name busName, bool isMuted)
    {
        if (busName == m_busName && success)
        {
            m_muteChB->blockSignals(true);
            m_muteChB->setChecked(isMuted);
            m_muteChB->blockSignals(false);
        }
    }

    void AudioBusWidget::OnRequestCompleted_SetAudioBusMono(bool success, AZ::Name busName, bool isMono)
    {
        if (busName == m_busName && success)
        {
            m_monoChB->blockSignals(true);
            m_monoChB->setChecked(isMono);
            m_monoChB->blockSignals(false);
        }
    }

    void AudioBusWidget::OnRequestCompleted_SetAudioBusOutput(bool success, AZ::Name busName, AZ::Name outputBusName)
    {
        if (m_busName != busName)
        {
            return;
        }

        m_outputBusCB->blockSignals(true);

        if (success)
        {
            m_outputBusName = outputBusName;
            m_outputBusCB->setCurrentText(outputBusName.GetCStr());
        }
        else
        {
            m_outputBusCB->setCurrentText(m_outputBusName.GetCStr());
        }

        m_outputBusCB->blockSignals(false);
    }

    void AudioBusWidget::OnUpdateAudioBusNames(AZStd::vector<AZ::Name> busNames)
    {
        if (m_busName == AZ::Name(Audio::MasterBusName))
        {
            return;
        }

        m_outputBusCB->blockSignals(true);
        m_outputBusCB->clear();

        for (size_t i = 0; i < busNames.size(); ++i)
        {
            const AZ::Name& name = busNames[i];

            if (name == m_busName)
            {
                m_busIndex = aznumeric_cast<int32_t>(i);
                break;
            }

            m_outputBusCB->addItem(name.GetCStr());
        }

        m_outputBusCB->setCurrentText(m_outputBusName.GetCStr());
        m_outputBusCB->blockSignals(false);
    }

    void AudioBusWidget::OnUpdateAudioBusPeakVolumes(AZ::Name busName, AZStd::vector<float> channelVolumes)
    {
        if (m_busName != busName)
        {
            return;
        }

        auto vuMeterWidget = qobject_cast<VuMeterWidget*>(m_vuMeterWidget);

        float channel1Value = Audio::LinearToDb(channelVolumes[0]);
        float channel2Value = (channelVolumes.size() < 2) ? channelVolumes[0] : channelVolumes[1];
        channel2Value = Audio::LinearToDb(channel2Value);

        vuMeterWidget->SetChannelValue(0, channel1Value);
        vuMeterWidget->SetChannelValue(1, channel2Value);
    }

    void AudioBusWidget::BlockSignals(bool isBlock)
    {
        m_busNameLineEdit->blockSignals(isBlock);
        m_volumeDsb->blockSignals(isBlock);
        m_volumeSlider->blockSignals(isBlock);
        m_muteChB->blockSignals(isBlock);
        m_monoChB->blockSignals(isBlock);
        m_outputBusCB->blockSignals(isBlock);
    }
} // namespace AudioControls
