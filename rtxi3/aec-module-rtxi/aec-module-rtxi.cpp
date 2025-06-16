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
#include "aec-module-rtxi.hpp"

#include <QTimer>
#include <cmath>
#include <deque>
#include <fstream>
#include <iostream>
#include <sstream>

#include <qlineedit.h>
#include <rtxi/fifo.hpp>
#include <rtxi/rt.hpp>
#include <rtxi/rtos.hpp>
void AecModule::Plugin::receiveEvent(Event::Object *event) {
  auto *module_panel = dynamic_cast<AecModule::Panel *>(this->getPanel());
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
AecModule::Component::Component(Widgets::Plugin *hplugin)
    : Widgets::Component(hplugin, std::string(AecModule::MODULE_NAME),
                         AecModule::get_default_channels(),
                         AecModule::get_default_vars()) {
  if (RT::OS::getFifo(this->fifo, 10 * sizeof(AecModule::aec_state_t)) < 0) {
    ERROR_MSG("PerformanceMeasurement::Component::Component : Unable to craate "
              "component fifo");
    this->setState(RT::State::PAUSE);
  }
}

AecModule::Plugin::Plugin(Event::Manager *ev_manager)
    : Widgets::Plugin(ev_manager, std::string(AecModule::MODULE_NAME)) {
  auto component = std::make_unique<AecModule::Component>(this);
  this->component_fifo = component->get_fifo_ptr();
  this->attachComponent(std::move(component));
}

AecModule::Panel::Panel(QMainWindow *main_window, Event::Manager *ev_manager)
    : Widgets::Panel(std::string(AecModule::MODULE_NAME), main_window,
                     ev_manager)
// ,
// s12_edit(new QLineEdit(this)), o12_edit(new QLineEdit(this)),
// s21_edit(new QLineEdit(this)), o21_edit(new QLineEdit(this))
{
  setWhatsThis(
      "<p><b>RTHybrid electrical aec model</b><br>RTHybrid module for RTXI "
      "that implements a gap junction electrical aec model.</p>");
  createGUI(AecModule::get_default_vars(),
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

  aux_edit = edits[AEC_AUX];

  if (aux_edit) {
    aux_edit->setReadOnly(true);
    aux_edit->setStyleSheet(readonlyStyle);
  }

  this->parentWidget()->adjustSize();
  QTimer *timer = new QTimer(this);
  connect(timer, &QTimer::timeout, this, &AecModule::Panel::refresh);
  timer->start(1000); // refresh every 500 ms
}
AecModule::aec_state_t AecModule::Plugin::get_aec_state() {

  AecModule::aec_state_t stat;
  while (this->component_fifo->read(&stat, sizeof(AecModule::aec_state_t)) >
         0) {
  };
  return stat;
}

void AecModule::Panel::refresh() {
  auto *hostplugin = dynamic_cast<AecModule::Plugin *>(this->getHostPlugin());
  const AecModule::aec_state_t s_state = hostplugin->get_aec_state();

  aux_edit->setText(QString::number(s_state.aux));
}

void AecModule::Component::init_parameters(void) {
  current_scale = getValue<double>(AEC_CURRENT_SCALE);
  aux = getValue<double>(AEC_AUX);
}

void AecModule::Component::execute() {
  // This is the real-time function that will be called
  switch (this->getState()) {
  case RT::State::EXEC:
    currents.push_back(readinput(1) * current_scale);

    if (currents.size() > kernel.size() + 1) {
      currents.pop_front();

      conv_result_full = conv(kernel, currents);
      conv_result_clean =
          std::vector<double>(conv_result_full.begin(),
                              conv_result_full.end() - (kernel.size() - 1));

      aux = conv_result_clean[conv_result_clean.size() - 1];
      writeoutput(0, readinput(0) - aux);
    }

    aec_state.aux = aux;

    this->fifo->writeRT(&this->aec_state, sizeof(AecModule::aec_state_t));

    break;
  case RT::State::INIT:
    period = RT::OS::getPeriod() * 1e-6; // ms
    init_parameters();
    kernel = read_kernel();

    setState(RT::State::PAUSE);
    break;
  case RT::State::MODIFY:
    kernel = read_kernel();
    setState(RT::State::EXEC);
    break;
  case RT::State::PERIOD:
    period = RT::OS::getPeriod() * 1e-6; // ms
    kernel = read_kernel();
    setState(RT::State::EXEC);
    break;
  case RT::State::PAUSE:
    writeoutput(0, 0.0);
    break;
  case RT::State::UNPAUSE:
    kernel = read_kernel();
    setState(RT::State::EXEC);
    break;

  default:
    break;
  }
}

template <typename T>
std::vector<T> AecModule::Component::conv(std::vector<T> const &f,
                                          std::deque<T> const &g) {
  int const nf = f.size();
  int const ng = g.size();
  int const n = nf + ng - 1;
  std::vector<T> out(n, T());
  for (auto i(0); i < n; ++i) {
    int const jmn = (i >= ng - 1) ? i - (ng - 1) : 0;
    int const jmx = (i < nf - 1) ? i : nf - 1;
    for (auto j(jmn); j <= jmx; ++j) {
      out[i] += (f[j] * g[i - j]);
    }
  }
  return out;
}

std::vector<double> AecModule::Component::read_kernel() {
  std::vector<double> new_kernel{};

  /***************/
  /* READ KERNEL */
  /***************/
  std::ifstream kernel_file;
  // kernel_file.open("/home/gnb/RTXI_pluggins/aec-for-rtxi/aec_kernel.txt");
  kernel_file.open(
      "/home/rtbio-preempt-rt/RTXI_pluggins/aec-for-rtxi/aec_kernel.txt");

  std::string str;
  while (std::getline(kernel_file, str)) {
    std::stringstream read_val(str);
    double read_val_2;
    read_val >> read_val_2;
    new_kernel.push_back(read_val_2);
  }

  kernel_file.close();
  return new_kernel;
}

///////// DO NOT MODIFY BELOW //////////
// The exception is if your plugin is not going to need real-time functionality.
// For this case just replace the craeteRTXIComponent return type to nullptr.
// RTXI will automatically handle that case and won't attach a component to the
// real time thread for your plugin.

std::unique_ptr<Widgets::Plugin> createRTXIPlugin(Event::Manager *ev_manager) {
  return std::make_unique<AecModule::Plugin>(ev_manager);
}

Widgets::Panel *createRTXIPanel(QMainWindow *main_window,
                                Event::Manager *ev_manager) {
  return new AecModule::Panel(main_window, ev_manager);
}

std::unique_ptr<Widgets::Component>
createRTXIComponent(Widgets::Plugin *host_plugin) {
  return std::make_unique<AecModule::Component>(host_plugin);
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
