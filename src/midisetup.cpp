/*
    MIDI Virtual Piano Keyboard
    Copyright (C) 2008-2026, Pedro Lopez-Cabanillas <plcl@users.sf.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; If not, see <http://www.gnu.org/licenses/>.
*/

#include <QMessageBox>
#include <QMetaObject>
#include <QMetaProperty>

#include <drumstick/configurationdialogs.h>
#include <drumstick/settingsfactory.h>

#include "iconutils.h"
#include "midisetup.h"
#include "vpianosettings.h"

MidiSetup::MidiSetup(QWidget *parent) : QDialog(parent),
    m_settingsChanged(false),
    m_midiIn(nullptr),
    m_savedIn(nullptr),
    m_midiOut(nullptr),
    m_savedOut(nullptr),
    m_midiOut2(nullptr),
    m_savedOut2(nullptr)
{
    ui.setupUi(this);
    ui.btnConfigInput->setIcon(IconUtils::GetIcon("wrench"));
    ui.btnConfigOutput->setIcon(IconUtils::GetIcon("wrench"));
    connect(ui.chkEnableInput, &QCheckBox::toggled, this, &MidiSetup::toggledInput);
    connect(ui.chkAdvanced, &QCheckBox::clicked, this, &MidiSetup::clickedAdvanced);
    connect(ui.comboinputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MidiSetup::refreshInputs);
    connect(ui.comboOutputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &MidiSetup::refreshOutputs);
    connect(ui.comboOutputBackends2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MidiSetup::refreshOutputs2);
    connect(ui.btnConfigInput, &QToolButton::clicked, this, &MidiSetup::configureInput);
    connect(ui.btnConfigOutput, &QToolButton::clicked, this, &MidiSetup::configureOutput);
    connect(ui.btnConfigOutput2, &QToolButton::clicked, this, &MidiSetup::configureOutput2);
}

void MidiSetup::toggledInput(bool state)
{
    ui.chkOmni->setEnabled(state);
    ui.chkEnableThru->setEnabled(state);
    ui.comboinputBackends->setEnabled(state);
    ui.comboInput->setEnabled(state);
    if (state) {
        refresh();
    } else {
        ui.chkOmni->setChecked(false);
        ui.chkEnableThru->setChecked(false);
        ui.comboinputBackends->setCurrentIndex(-1);
        ui.comboInput->setCurrentIndex(-1);
    }
}

void MidiSetup::inputNotAvailable()
{
    toggledInput(false);
}

void MidiSetup::clearCombos()
{
    ui.comboInput->clear();
    ui.comboOutput->clear();
}

void MidiSetup::retranslateUi()
{
    ui.retranslateUi(this);
}

void MidiSetup::setInput(MIDIInput *in)
{
    m_savedIn = m_midiIn = in;
    m_connIn = in->currentConnection();
}

void MidiSetup::setOutput(MIDIOutput *out)
{
    m_savedOut = m_midiOut = out;
    m_connOut = out != nullptr ? out->currentConnection() : MIDIConnection();
}

void MidiSetup::setOutput2(MIDIOutput *out)
{
    m_savedOut2 = m_midiOut2 = out;
    m_connOut2 = out != nullptr ? out->currentConnection() : MIDIConnection();
}

void MidiSetup::setInputs(QList<MIDIInput *> ins)
{
    ui.comboinputBackends->disconnect();
    ui.comboinputBackends->clear();
    foreach(MIDIInput *i, ins) {
        ui.comboinputBackends->addItem(i->backendName(), QVariant::fromValue(i));
    }
    connect(ui.comboinputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MidiSetup::refreshInputs);
}

void MidiSetup::setOutputs(QList<MIDIOutput *> outs)
{
    ui.comboOutputBackends->disconnect();
    foreach(MIDIOutput *o, outs) {
        ui.comboOutputBackends->addItem(o->backendName(), QVariant::fromValue(o));
    }
    connect(ui.comboOutputBackends, QOverload<int>::of(&QComboBox::currentIndexChanged), this,  &MidiSetup::refreshOutputs);
}

void MidiSetup::setOutputs2(QList<MIDIOutput *> outs)
{
    ui.comboOutputBackends2->disconnect();
    ui.comboOutputBackends2->clear();
    ui.comboOutputBackends2->addItem(tr("None"));
    foreach(MIDIOutput *o, outs) {
        ui.comboOutputBackends2->addItem(o->backendName(), QVariant::fromValue(o));
    }
    connect(ui.comboOutputBackends2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MidiSetup::refreshOutputs2);
}

void MidiSetup::showEvent(QShowEvent *)
{
    ui.chkEnableInput->setChecked(VPianoSettings::instance()->inputEnabled());
    ui.chkEnableThru->setChecked(VPianoSettings::instance()->midiThru());
    ui.chkAdvanced->setChecked(VPianoSettings::instance()->advanced());
    ui.chkOmni->setChecked(VPianoSettings::instance()->omniMode());
    refresh();
}

void MidiSetup::accept()
{
    m_connIn = ui.comboInput->currentData().value<MIDIConnection>();
    m_connOut = ui.comboOutput->currentData().value<MIDIConnection>();
    m_connOut2 = ui.comboOutput2->currentData().value<MIDIConnection>();
    VPianoSettings::instance()->setAdvanced(ui.chkAdvanced->isChecked());
    VPianoSettings::instance()->setMidiThru(ui.chkEnableThru->isChecked());
    VPianoSettings::instance()->setOmniMode(ui.chkOmni->isChecked());
    VPianoSettings::instance()->setInputEnabled(ui.chkEnableInput->isChecked());
    reopen();
    VPianoSettings::instance()->setLastInputBackend(m_midiIn != nullptr ? m_midiIn->backendName() : QString());
    VPianoSettings::instance()->setLastOutputBackend(m_midiOut != nullptr ? m_midiOut->backendName() : QString());
    VPianoSettings::instance()->setLastOutput2Backend(m_midiOut2 != nullptr ? m_midiOut2->backendName() : QString());
    VPianoSettings::instance()->setLastInputConnection(m_connIn.first);
    VPianoSettings::instance()->setLastOutputConnection(m_connOut.first);
    VPianoSettings::instance()->setLastOutput2Connection(m_connOut2.first);
    m_settingsChanged = false;
    QDialog::accept();
}

void MidiSetup::reject()
{
    m_midiIn = m_savedIn;
    m_midiOut = m_savedOut;
    m_midiOut2 = m_savedOut2;
    reopen();
    QDialog::reject();
}

void MidiSetup::configureOutputDriverDefaults(MIDIOutput *output)
{
    const int DEF_SVOX_GAIN{99};
    const double DEF_FLUID_GAIN{0.99};
    if (output != nullptr) {
        bool ok = false;
        auto metaObj = output->metaObject();
        auto idx = metaObj->indexOfProperty("defaultGain");
        const QString cname = QString::fromLatin1(metaObj->className());
        if (idx != -1) {
            QMetaProperty prop = metaObj->property(idx);
            if (cname == "drumstick::rt::SynthController") {
                int retVal = prop.read(output).toInt();
                if (retVal != DEF_SVOX_GAIN) {
                    ok = prop.write(output, DEF_SVOX_GAIN);
                }
            } else if (cname == "drumstick::rt::FluidSynthOutput") {
                double retVal = prop.read(output).toInt();
                if (retVal != DEF_FLUID_GAIN) {
                    ok = prop.write(output, DEF_FLUID_GAIN);
                }
            }
            if (!ok) {
                qWarning() << Q_FUNC_INFO << cname << "property" << prop.name() << "write failed";
            }
        }
    }
}

void MidiSetup::refresh()
{
    bool advanced = ui.chkAdvanced->isChecked();
    if (m_midiIn != nullptr) {
        ui.comboinputBackends->setCurrentText(m_midiIn->backendName());
        refreshInputDrivers(m_midiIn->backendName(), advanced);
    }
    if (m_midiOut != nullptr) {
        ui.comboOutputBackends->setCurrentText(m_midiOut->backendName());
        refreshOutputDrivers(m_midiOut->backendName(), advanced);
    }
    if (m_midiOut2 != nullptr) {
        ui.comboOutputBackends2->setCurrentText(m_midiOut2->backendName());
        refreshOutputDrivers2(m_midiOut2->backendName(), advanced);
    } else {
        ui.comboOutputBackends2->setCurrentIndex(0);
        ui.comboOutput2->clear();
        ui.btnConfigOutput2->setEnabled(false);
    }
}

void MidiSetup::reopen()
{
    drumstick::widgets::SettingsFactory settings;
    if (m_midiOut != nullptr) {
        if (m_connOut != m_midiOut->currentConnection() || m_settingsChanged) {
            m_midiOut->close();
            if (!m_connOut.first.isEmpty()) {
                m_midiOut->initialize(settings.getQSettings());
                m_midiOut->open(m_connOut);
                auto metaObj = m_midiOut->metaObject();
                if ((metaObj->indexOfProperty("status") != -1) &&
                    (metaObj->indexOfProperty("diagnostics") != -1)) {
                    auto status = m_midiOut->property("status");
                    if (status.isValid() && !status.toBool()) {
                        auto diagnostics = m_midiOut->property("diagnostics");
                        if (diagnostics.isValid()) {
                            auto text = diagnostics.toStringList().join(QChar::LineFeed).trimmed();
                            QMessageBox::warning(this, tr("MIDI Output"), text);
                        }
                    }
                }
            }
        }
    }
    if (m_midiOut2 != nullptr) {
        if (m_connOut2 != m_midiOut2->currentConnection() || m_settingsChanged) {
            m_midiOut2->close();
            if (!m_connOut2.first.isEmpty()) {
                m_midiOut2->initialize(settings.getQSettings());
                m_midiOut2->open(m_connOut2);
                auto metaObj = m_midiOut2->metaObject();
                if ((metaObj->indexOfProperty("status") != -1) &&
                    (metaObj->indexOfProperty("diagnostics") != -1)) {
                    auto status = m_midiOut2->property("status");
                    if (status.isValid() && !status.toBool()) {
                        auto diagnostics = m_midiOut2->property("diagnostics");
                        if (diagnostics.isValid()) {
                            auto text = diagnostics.toStringList().join(QChar::LineFeed).trimmed();
                            QMessageBox::warning(this, tr("MIDI Output 2"), text);
                        }
                    }
                }
            }
        }
    }
    if (m_midiIn != nullptr) {
        if (m_connIn != m_midiIn->currentConnection() || m_settingsChanged) {
            m_midiIn->close();
            m_midiIn->initialize(settings.getQSettings());
            if (!m_connIn.first.isEmpty()) {
                m_midiIn->open(m_connIn);
                auto metaObj = m_midiIn->metaObject();
                if ((metaObj->indexOfProperty("status") != -1) &&
                    (metaObj->indexOfProperty("diagnostics") != -1)) {
                    auto status = m_midiIn->property("status");
                    if (status.isValid() && !status.toBool()) {
                        auto diagnostics = m_midiIn->property("diagnostics");
                        if (diagnostics.isValid()) {
                            auto text = diagnostics.toStringList().join(QChar::LineFeed).trimmed();
                            QMessageBox::warning(this, tr("MIDI Input"), text);
                        }
                    }
                }
            }
        }
        if (m_midiOut != nullptr) {
            m_midiIn->enableMIDIThru(ui.chkEnableThru->isChecked());
            m_midiIn->setMIDIThruDevice(m_midiOut);
        }
    }
}

void MidiSetup::refreshInputs(int idx)
{
    bool advanced = ui.chkAdvanced->isChecked();
    QString id = ui.comboinputBackends->itemText(idx);
    refreshInputDrivers(id, advanced);
}

void MidiSetup::refreshInputDrivers(QString id, bool advanced)
{
    ui.btnConfigInput->setEnabled(drumstick::widgets::inputDriverIsConfigurable(id));
    if (m_midiIn != nullptr && m_midiIn->backendName() != id) {
        m_midiIn->close();
        int idx = ui.comboinputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1)
            m_midiIn = ui.comboinputBackends->itemData(idx).value<MIDIInput*>();
        else
            m_midiIn = nullptr;
    }
    ui.comboInput->clear();
    if (m_midiIn != nullptr) {
        auto connections = m_midiIn->connections(advanced);
        foreach (const MIDIConnection& conn, connections) {
            ui.comboInput->addItem(conn.first, QVariant::fromValue(conn));
        }
        QString connIn = m_midiIn->currentConnection().first;
        if (connIn.isEmpty() && !connections.isEmpty()) {
            connIn = connections.first().first;
        }
        ui.comboInput->setCurrentText(connIn);
    }
}

void MidiSetup::refreshOutputs(int idx)
{
    bool advanced = ui.chkAdvanced->isChecked();
    QString id = ui.comboOutputBackends->itemText(idx);
    refreshOutputDrivers(id, advanced);
}

void MidiSetup::refreshOutputs2(int idx)
{
    bool advanced = ui.chkAdvanced->isChecked();
    QString id = ui.comboOutputBackends2->itemText(idx);
    refreshOutputDrivers2(id, advanced);
}

void MidiSetup::refreshOutputDrivers(QString id, bool advanced)
{
    ui.btnConfigOutput->setEnabled(drumstick::widgets::outputDriverIsConfigurable(id));
    if (m_midiOut != nullptr && m_midiOut->backendName() != id) {
        m_midiOut->close();
        int idx = ui.comboOutputBackends->findText(id, Qt::MatchStartsWith);
        if (idx > -1)
            m_midiOut = ui.comboOutputBackends->itemData(idx).value<MIDIOutput*>();
        else
            m_midiOut = nullptr;
    }
    ui.comboOutput->clear();
    if (m_midiOut != nullptr) {
        auto connections = m_midiOut->connections(advanced);
        foreach (const MIDIConnection& conn, connections) {
            ui.comboOutput->addItem(conn.first, QVariant::fromValue(conn));
        }
        QString connOut = m_midiOut->currentConnection().first;
        if (connOut.isEmpty() && !connections.isEmpty()) {
            connOut = connections.first().first;
        }
        ui.comboOutput->setCurrentText(connOut);
    }
}

void MidiSetup::refreshOutputDrivers2(QString id, bool advanced)
{
    if (id == tr("None")) {
        if (m_midiOut2 != nullptr) {
            m_midiOut2->close();
            m_midiOut2 = nullptr;
        }
        ui.btnConfigOutput2->setEnabled(false);
        ui.comboOutput2->clear();
        return;
    }
    ui.btnConfigOutput2->setEnabled(drumstick::widgets::outputDriverIsConfigurable(id));
    if (m_midiOut2 != nullptr && m_midiOut2->backendName() != id) {
        m_midiOut2->close();
        int idx = ui.comboOutputBackends2->findText(id, Qt::MatchStartsWith);
        if (idx > -1)
            m_midiOut2 = ui.comboOutputBackends2->itemData(idx).value<MIDIOutput*>();
        else
            m_midiOut2 = nullptr;
    } else if (m_midiOut2 == nullptr) {
        int idx = ui.comboOutputBackends2->findText(id, Qt::MatchStartsWith);
        if (idx > -1)
            m_midiOut2 = ui.comboOutputBackends2->itemData(idx).value<MIDIOutput*>();
    }
    ui.comboOutput2->clear();
    if (m_midiOut2 != nullptr) {
        auto connections = m_midiOut2->connections(advanced);
        foreach (const MIDIConnection& conn, connections) {
            ui.comboOutput2->addItem(conn.first, QVariant::fromValue(conn));
        }
        QString connOut = m_midiOut2->currentConnection().first;
        if (connOut.isEmpty() && !connections.isEmpty()) {
            connOut = connections.first().first;
        }
        ui.comboOutput2->setCurrentText(connOut);
    }
}

void MidiSetup::configureInput()
{
    QString driver = ui.comboinputBackends->currentText();
    if (drumstick::widgets::inputDriverIsConfigurable(driver)) {
        m_settingsChanged |= drumstick::widgets::configureInputDriver(driver, this);
    }
}

void MidiSetup::configureOutput()
{
    QString driver = ui.comboOutputBackends->currentText();
    if (drumstick::widgets::outputDriverIsConfigurable(driver)) {
        configureOutputDriverDefaults(m_midiOut);
        m_settingsChanged |= drumstick::widgets::configureOutputDriver(driver, this);
    }
}

void MidiSetup::configureOutput2()
{
    QString driver = ui.comboOutputBackends2->currentText();
    if (m_midiOut2 != nullptr && drumstick::widgets::outputDriverIsConfigurable(driver)) {
        configureOutputDriverDefaults(m_midiOut2);
        m_settingsChanged |= drumstick::widgets::configureOutputDriver(driver, this);
    }
}

void MidiSetup::clickedAdvanced(bool value)
{
    Q_UNUSED(value)
    refresh();
}

void MidiSetup::setMidiThru(bool value)
{
    ui.chkEnableThru->setChecked(value);
}
