//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "InterfaceKernel.h"
#include "RankTwoTensor.h"

class FluidStructureInterface_Damping : public InterfaceKernel
{
public:
  static InputParameters validParams();

  FluidStructureInterface_Damping(const InputParameters & parameters);

protected:
  virtual Real computeQpResidual(Moose::DGResidualType type) override;
  virtual Real computeQpJacobian(Moose::DGJacobianType type) override;

  const MooseVariableFE<Real>::FieldVariableSecond * _second_sln;
  const MooseVariableFE<Real>::FieldVariableSecond * _second_sln_old;

  const MaterialProperty<Real> & _D;
  Real _viscosity;
  const VariableValue & _neighbor_dotdot;
  const VariableValue & _neighbor_dotdot_du;
  unsigned int _component;
};
