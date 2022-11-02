/*
 Copyright (C) 2022 GNB-UAM

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.

 */


#include <aec-module-rtxi.h>
#include <iostream>
#include <main_window.h>
#include <math.h>
#include <deque>

#include <fstream>
#include <sstream>

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new AecModuleRtxi();
}

static DefaultGUIModel::variable_t vars[] = {
  {"Voltage", "Readed voltage from the cell", DefaultGUIModel::INPUT |  DefaultGUIModel::DOUBLE,},
  {"Current", "Injected current in the cell", DefaultGUIModel::INPUT |  DefaultGUIModel::DOUBLE,},
  {"Clean Voltage", "Voltage compensated",    DefaultGUIModel::OUTPUT | DefaultGUIModel::DOUBLE,},
  {"Aux", "Aux", DefaultGUIModel::STATE,},
  {"Current Scale", "Scale for the Current (multiplying)", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,},
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

AecModuleRtxi::AecModuleRtxi(void)
  : DefaultGUIModel("AecModuleRtxi with Custom GUI", ::vars, ::num_vars)
{
  setWhatsThis("<p><b>AecModuleRtxi:</b><br>QWhatsThis description.</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  customizeGUI();
  initParameters();
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));
}

AecModuleRtxi::~AecModuleRtxi(void)
{
}

void
AecModuleRtxi::execute(void)
{
  currents.push_back(input(1)*current_scale);

  if(currents.size()>kernel.size()+1){
    currents.pop_front();

    conv_result_full  = conv(kernel, currents);
    conv_result_clean = std::vector<double>(conv_result_full.begin(), conv_result_full.end()-(kernel.size()-1));

    aux = conv_result_clean[conv_result_clean.size()-1];
    output(0) = input(0) - aux;
  }

  return;
}

void
AecModuleRtxi::initParameters(void)
{
	current_scale=1;
}

void
AecModuleRtxi::update(DefaultGUIModel::update_flags_t flag)
{
  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      setState("Aux", aux);
      setParameter("Current Scale",current_scale);
      kernel = read_kernel();
      break;

    case MODIFY:
      kernel = read_kernel();
      current_scale = getParameter("Current Scale").toDouble();
      break;

    case UNPAUSE:
      kernel = read_kernel();
      currents.clear();
      break;

    case PAUSE:
      output(0) = 0;
      break;

    case PERIOD:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      break;

    default:
      break;
  }
}

void
AecModuleRtxi::customizeGUI(void)
{

}

template<typename T> std::vector<T>
AecModuleRtxi::conv(std::vector<T> const &f, std::deque<T> const &g) {
  int const nf = f.size();
  int const ng = g.size();
  int const n  = nf + ng - 1;
  std::vector<T> out(n, T());
  for(auto i(0); i < n; ++i) {
    int const jmn = (i >= ng - 1)? i - (ng - 1) : 0;
    int const jmx = (i <  nf - 1)? i            : nf - 1;
    for(auto j(jmn); j <= jmx; ++j) {
      out[i] += (f[j] * g[i - j]);
    }
  }
  return out; 
}

std::vector<double>
AecModuleRtxi::read_kernel(){
  std::vector<double> new_kernel{};

  /***************/
  /* READ KERNEL */
  /***************/
  std::ifstream kernel_file;
  kernel_file.open("/home/gnb/RTXI_pluggins/aec-for-rtxi/aec_kernel.txt");

  std::string str;
  while (std::getline(kernel_file, str)){
        std::stringstream read_val(str);
        double read_val_2;
        read_val >> read_val_2;
        new_kernel.push_back(read_val_2);
  }

  kernel_file.close();
  return new_kernel;
}

