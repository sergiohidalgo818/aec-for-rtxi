/*
 * Copyright (C) 2011 Georgia Institute of Technology, University of Utah,
 * Weill Cornell Medical College
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * This is a template implementation file for a user module,
 */
#include "white-noise-module-rtxi.hpp"

#include <QTimer>
#include <cmath>
#include <qlineedit.h>
#include <rtxi/fifo.hpp>
#include <rtxi/rt.hpp>
#include <rtxi/rtos.hpp>
void RTHybridElectricalSynapse::Plugin::receiveEvent(Event::Object *event) {
  auto *module_panel =
      dynamic_cast<RTHybridElectricalSynapse::Panel *>(this->getPanel());
  switch (event->getType()) {
  case Event::Type::RT_THREAD_INSERT_EVENT:
  case Event::Type::RT_DEVICE_INSERT_EVENT:
    // module_panel->updateBlockInfo();
    break;
  case Event::Type::RT_THREAD_REMOVE_EVENT:
    // module_panel->updateBlockChannels(
    //     std::any_cast<RT::Thread *>(event->getParam("thread")));
    // module_panel->updateBlockInfo();
    break;
  case Event::Type::RT_DEVICE_REMOVE_EVENT:
    // module_panel->updateBlockChannels(
    //     std::any_cast<RT::Device *>(event->getParam("device")));
    // module_panel->updateBlockInfo();
    break;
  default:
    break;
  }
}
RTHybridElectricalSynapse::Component::Component(Widgets::Plugin *hplugin)
    : Widgets::Component(hplugin,
                         std::string(RTHybridElectricalSynapse::MODULE_NAME),
                         RTHybridElectricalSynapse::get_default_channels(),
                         RTHybridElectricalSynapse::get_default_vars()) {
  if (RT::OS::getFifo(this->fifo,
                      10 * sizeof(RTHybridElectricalSynapse::synapse_state_t)) <
      0) {
    ERROR_MSG("PerformanceMeasurement::Component::Component : Unable to craate "
              "component fifo");
    this->setState(RT::State::PAUSE);
  }
}

RTHybridElectricalSynapse::Plugin::Plugin(Event::Manager *ev_manager)
    : Widgets::Plugin(ev_manager,
                      std::string(RTHybridElectricalSynapse::MODULE_NAME)) {
  auto component = std::make_unique<RTHybridElectricalSynapse::Component>(this);
  this->component_fifo = component->get_fifo_ptr();
  this->attachComponent(std::move(component));
}

RTHybridElectricalSynapse::Panel::Panel(QMainWindow *main_window,
                                        Event::Manager *ev_manager)
    : Widgets::Panel(std::string(RTHybridElectricalSynapse::MODULE_NAME),
                     main_window, ev_manager)
// ,
// s12_edit(new QLineEdit(this)), o12_edit(new QLineEdit(this)),
// s21_edit(new QLineEdit(this)), o21_edit(new QLineEdit(this))
{
  setWhatsThis("<p><b>White noise module for RTXI</b><p>");
  createGUI(RTHybridElectricalSynapse::get_default_vars(),
            {}); // this is required to create the GUI
  auto edits = findChildren<QLineEdit *>();
  QString readonlyStyle = R"(
    QLineEdit {
    background-color: #e0e0e0;
    color: #666666;
    border: 1px solid #cccccc;
    }
    QLineEdit:focus {
    border: 1px solid #cccccc;
    outline: none;
    }
  )";

  test_edit = edits[WHITE_NOISE_TEST];

  if (test_edit) {
    test_edit->setReadOnly(true);
    test_edit->setStyleSheet(readonlyStyle);
  }

  this->parentWidget()->adjustSize();
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this,
          &RTHybridElectricalSynapse::Panel::refresh);
  timer->start(1000); // refresh every 500 ms
}
RTHybridElectricalSynapse::synapse_state_t
RTHybridElectricalSynapse::Plugin::get_synapse_state() {

  RTHybridElectricalSynapse::synapse_state_t stat;
  while (this->component_fifo->read(
             &stat, sizeof(RTHybridElectricalSynapse::synapse_state_t)) > 0) {
  };
  return stat;
}

void RTHybridElectricalSynapse::Panel::refresh() {
  auto *hostplugin =
      dynamic_cast<RTHybridElectricalSynapse::Plugin *>(this->getHostPlugin());
  const RTHybridElectricalSynapse::synapse_state_t s_state =
      hostplugin->get_synapse_state();

  test_edit->setText(QString::number(s_state.test));
}

void RTHybridElectricalSynapse::Component::init_parameters(void) {
  min_val = getValue<double>(WHITE_NOISE_MIN);
  max_val = getValue<double>(WHITE_NOISE_MAX);
  out_test = getValue<double>(WHITE_NOISE_TEST);
}

void RTHybridElectricalSynapse::Component::execute() {
  // This is the real-time function that will be called
  switch (this->getState()) {
  case RT::State::EXEC: {
    decltype(dis_wn.param()) new_range(min_val, max_val);
    dis_wn.param(new_range);
    double result = dis_wn(gen_wn);
    writeoutput(0, result);
    synapse_state.test = result;
  } break;
  case RT::State::INIT:
    period = RT::OS::getPeriod() * 1e-6; // ms

    init_parameters();
    setState(RT::State::PAUSE);
    break;
  case RT::State::MODIFY:
    setState(RT::State::EXEC);
    break;
  case RT::State::PERIOD:
    period = RT::OS::getPeriod() * 1e-6; // ms
    setState(RT::State::EXEC);
    break;
  case RT::State::PAUSE:
    writeoutput(0, 0.0);
    break;
  case RT::State::UNPAUSE:
    setState(RT::State::EXEC);
    break;

  default:
    break;
  }
}

///////// DO NOT MODIFY BELOW //////////
// The exception is if your plugin is not going to need real-time functionality.
// For this case just replace the craeteRTXIComponent return type to nullptr.
// RTXI will automatically handle that case and won't attach a component to the
// real time thread for your plugin.

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager *ev_manager) {
  return std::make_unique<RTHybridElectricalSynapse::Plugin>(ev_manager);
}

Widgets::Panel *createRTXIPanel(QMainWindow *main_window,
                                Event::Manager *ev_manager) {
  return new RTHybridElectricalSynapse::Panel(main_window, ev_manager);
}

std::unique_ptr<Widgets::Component>
createRTXIComponent(Widgets::Plugin *host_plugin) {
  return std::make_unique<RTHybridElectricalSynapse::Component>(host_plugin);
}

Widgets::FactoryMethods fact;

extern "C" {
Widgets::FactoryMethods *getFactories() {
  fact.createPanel = &createRTXIPanel;
  fact.createComponent = &createRTXIComponent;
  fact.createPlugin = &createRTXIPlugin;
  return &fact;
}
};

//////////// END //////////////////////
