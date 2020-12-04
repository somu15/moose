//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MooseApp.h"

<<<<<<< HEAD:tutorials/tutorial01_app_development/step09_mat_props/test/include/base/BabblerTestApp.h
class BabblerTestApp : public MooseApp
=======
class FsiTestApp : public MooseApp
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/test/include/base/FsiTestApp.h
{
public:
  static InputParameters validParams();

<<<<<<< HEAD:tutorials/tutorial01_app_development/step09_mat_props/test/include/base/BabblerTestApp.h
  BabblerTestApp(InputParameters parameters);
  virtual ~BabblerTestApp();
=======
  FsiTestApp(InputParameters parameters);
  virtual ~FsiTestApp();
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/test/include/base/FsiTestApp.h

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s, bool use_test_objs = false);
};
