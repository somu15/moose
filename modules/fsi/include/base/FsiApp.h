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

<<<<<<< HEAD:tutorials/tutorial01_app_development/step08_test_harness/include/base/BabblerApp.h
class BabblerApp : public MooseApp
=======
class FsiApp : public MooseApp
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/include/base/FsiApp.h
{
public:
  static InputParameters validParams();

<<<<<<< HEAD:tutorials/tutorial01_app_development/step08_test_harness/include/base/BabblerApp.h
  BabblerApp(InputParameters parameters);
  virtual ~BabblerApp();
=======
  FsiApp(InputParameters parameters);
  virtual ~FsiApp();
>>>>>>> 6e7c9cf753... Rename fluid_structure_interaction to fsi:modules/fsi/include/base/FsiApp.h

  static void registerApps();
  static void registerAll(Factory & f, ActionFactory & af, Syntax & s);
};
