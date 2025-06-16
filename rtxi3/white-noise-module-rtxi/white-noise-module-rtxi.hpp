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
#include <random>
#include <rtxi/event.hpp>
#include <rtxi/io.hpp>
#include <rtxi/widgets.hpp>

namespace RTHybridElectricalSynapse {

constexpr std::string_view MODULE_NAME = "White noise module generator";

enum PARAMETER : Widgets::Variable::Id {
  // set parameter ids here
  WHITE_NOISE_MIN = 0,
  WHITE_NOISE_MAX,
  WHITE_NOISE_TEST,
};

inline std::vector<Widgets::Variable::Info> get_default_vars() {
  return {

      {WHITE_NOISE_MIN, "Min value", "The min output value",
       Widgets::Variable::DOUBLE_PARAMETER, -0.5},
      {WHITE_NOISE_MAX, "Max value", "The max output value",
       Widgets::Variable::DOUBLE_PARAMETER, 0.5},
      {WHITE_NOISE_TEST, "Test value", "A test value for test",
       Widgets::Variable::DOUBLE_PARAMETER, 0.0},
  };
}

inline std::vector<IO::channel_t> get_default_channels() {
  return {
      {"Out value", "Generated white noise values", IO::OUTPUT},
  };
}
struct synapse_state_t {
  double test = 0.0;
};

class Panel : public Widgets::Panel {
  Q_OBJECT
public:
  Panel(QMainWindow *main_window, Event::Manager *ev_manager);
  // Any functions and data related to the GUI are to be placed
  // here
  void refresh() override;

private:
  QLineEdit *test_edit = nullptr;
};

class Plugin : public Widgets::Plugin {

public:
  void receiveEvent(Event::Object *event) override;
  explicit Plugin(Event::Manager *ev_manager);
  synapse_state_t get_synapse_state();

private:
  RT::OS::Fifo *component_fifo;
};

class Component : public Widgets::Component {
public:
  explicit Component(Widgets::Plugin *hplugin);

  void execute() override;
  // Additional functionality needed for RealTime computation is to be placed
  // here

  RT::OS::Fifo *get_fifo_ptr() { return this->fifo.get(); }

private:
  double period;
  double out_test;

  double min_val;
  double max_val;

  std::random_device rd_wn;
  std::mt19937 gen_wn;
  std::uniform_real_distribution<> dis_wn;

  double result;

  synapse_state_t synapse_state;
  std::unique_ptr<RT::OS::Fifo> fifo;

  void init_parameters();
  ;
  void sm_electrical(double v_post, double v_pre, double *ret);
};

} // namespace RTHybridElectricalSynapse
