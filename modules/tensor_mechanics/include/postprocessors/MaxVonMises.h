//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

// #include "NodalPostprocessor.h"
#include "ElementPostprocessor.h"

// Forward Declarations
class AuxiliarySystem;

class MaxVonMises : public ElementPostprocessor // public NodalPostprocessor //
{
public:
  static InputParameters validParams();

  MaxVonMises(const InputParameters & parameters);

  virtual void initialize();
  virtual void execute();
  virtual Real getValue();
  void threadJoin(const UserObject & y);

protected:
  AuxiliarySystem & _aux;

  const VariableValue * _react;

  Real _max_value;
};
