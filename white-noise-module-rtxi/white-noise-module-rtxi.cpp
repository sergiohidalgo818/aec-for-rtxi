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
 * This is a template implementation file for a user module derived from
 * DefaultGUIModel with a custom GUI.
 */

#include "white-noise-module-rtxi.h"
#include <iostream>
#include <main_window.h>
#include <math.h>
#include <random>

extern "C" Plugin::Object*
createRTXIPlugin(void)
{
  return new WhiteNoiseModuleRtxi();
}

static DefaultGUIModel::variable_t vars[] = {
  /*Module variables*/
  {"Min value", "The min output value", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,},
  {"Max value", "The max output value", DefaultGUIModel::PARAMETER | DefaultGUIModel::DOUBLE,},
  {"Test value","A test value for test", DefaultGUIModel::STATE | DefaultGUIModel::DOUBLE,},
  {"Out value", "Generated white noise values", DefaultGUIModel::OUTPUT | DefaultGUIModel::DOUBLE,},
};

static size_t num_vars = sizeof(vars) / sizeof(DefaultGUIModel::variable_t);

WhiteNoiseModuleRtxi::WhiteNoiseModuleRtxi(void)
  : DefaultGUIModel("White Noise Generator", ::vars, ::num_vars),
    gen_wn(rd_wn()),
    dis_wn(min_val, max_val)
{
  setWhatsThis("<p><b>WhiteNoise:</b><br>QWhatsThis description.</p>");
  DefaultGUIModel::createGUI(vars,
                             num_vars); // this is required to create the GUI
  customizeGUI();
  initParameters();
  update(INIT); // this is optional, you may place initialization code directly
                // into the constructor
  refresh();    // this is required to update the GUI with parameter and state
                // values
  QTimer::singleShot(0, this, SLOT(resizeMe()));

  //std::random_device rd_wn; // obtain a random number from hardware
  //std::mt19937 gen_wn(rd_wn()); // seed the generator
  //std::uniform_real_distribution<double> dis_wn(min_val, max_val); // define the range
}

WhiteNoiseModuleRtxi::~WhiteNoiseModuleRtxi(void)
{
}

void
WhiteNoiseModuleRtxi::execute(void)
{
  decltype(dis_wn.param()) new_range (min_val, max_val);
  dis_wn.param(new_range);
  output(0) = dis_wn(gen_wn);
  //output(0) = 13.5;
  return;
}

void
WhiteNoiseModuleRtxi::initParameters(void)
{
  min_val = -0.5;
  max_val = 0.5;
  // double result ;

  setParameter("Min value", min_val);
  setParameter("Max value", max_val);
}

void
WhiteNoiseModuleRtxi::update(DefaultGUIModel::update_flags_t flag)
{


  switch (flag) {
    case INIT:
      period = RT::System::getInstance()->getPeriod() * 1e-6; // ms
      setParameter("Min value", min_val);
      setParameter("Max value", max_val);
      setState("Test value", output(0));
      break;

    case MODIFY:
      min_val = getParameter("Min value").toDouble();
      max_val = getParameter("Max value").toDouble();
      break;

    case UNPAUSE:
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
WhiteNoiseModuleRtxi::customizeGUI(void)
{
}
