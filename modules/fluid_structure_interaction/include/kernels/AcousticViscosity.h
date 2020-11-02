//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Kernel.h"
#include "Material.h"
// #include "MooseVariableFE.h"
#include "MooseVariableData.h"

class AcousticViscosity;

template <>
InputParameters validParams<AcousticViscosity>();

/**
 * This kernel implements the Laplacian operator:
 * $\nabla u \cdot \nabla \phi_i$
 */
class AcousticViscosity : public Kernel
{
public:
  static InputParameters validParams();

  AcousticViscosity(const InputParameters & parameters);

protected:
  // const VariableValue * _grad_dot_req; MooseVariableFE<double>::
  // const MooseVariableFE<double>::FieldVariableGradient * _grad_dot_req;
  const MooseVariableFE<double>::FieldVariableGradient * _grad_dot_req;
  // const VariableValue * _phi_dot_req;

  const MooseVariableFE<Real>::FieldVariableSecond * _second_sln;
  const MooseVariableFE<Real>::FieldVariableSecond * _second_sln_old;

  const MaterialProperty<Real> & _inv_co_sq;

  Real _viscosity;

  Real _densityF;

  virtual Real computeQpResidual() override;

  virtual Real computeQpJacobian() override;
};
