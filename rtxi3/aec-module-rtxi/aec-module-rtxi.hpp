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
#include <deque>
#include <rtxi/event.hpp>
#include <rtxi/io.hpp>
#include <rtxi/widgets.hpp>
namespace AecModule {

constexpr std::string_view MODULE_NAME = "AEC Module";

enum PARAMETER : Widgets::Variable::Id {
  AEC_CURRENT_SCALE = 0,
  AEC_AUX,
};

inline std::vector<Widgets::Variable::Info> get_default_vars() {
  return {

      {AEC_CURRENT_SCALE, "Current Scale",
       "Scale for the Current (multiplying)",
       Widgets::Variable::DOUBLE_PARAMETER, 1.0},
      {AEC_AUX, "A State", "Aux", Widgets::Variable::DOUBLE_PARAMETER, 0.0},

  };
}

inline std::vector<IO::channel_t> get_default_channels() {
  return {
      {
          "Voltage",
          "Readed voltage from the cell",
          IO::INPUT,
      },
      {
          "Current",
          "Injected current in the cell",
          IO::INPUT,
      },
      {
          "Clean Voltage",
          "Voltage compensated",
          IO::OUTPUT,
      },
  };
}

struct aec_state_t {
  double aux = 0.0;
};

class Panel : public Widgets::Panel {
  Q_OBJECT
public:
  Panel(QMainWindow *main_window, Event::Manager *ev_manager);
  // Any functions and data related to the GUI are to be placed
  // here
  void refresh() override;

private:
  QLineEdit *aux_edit = nullptr;
};

class Plugin : public Widgets::Plugin {

public:
  void receiveEvent(Event::Object *event) override;
  explicit Plugin(Event::Manager *ev_manager);
  aec_state_t get_aec_state();

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
  double voltage, current, clean_voltage;
  double current_scale, aux;

  std::vector<double> kernel;
  std::vector<double> read_kernel();

  template <typename T>
  std::vector<T> conv(std::vector<T> const &f, std::deque<T> const &g);
  std::deque<double> currents;

  std::vector<double> conv_result_full;
  std::vector<double> conv_result_clean;

  aec_state_t aec_state;
  std::unique_ptr<RT::OS::Fifo> fifo;

  void init_parameters();
};

} // namespace AecModule
