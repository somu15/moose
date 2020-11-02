//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AcousticViscosity.h"
#include "MooseVariableData.h"

registerMooseObject("MooseApp", AcousticViscosity);

defineLegacyParams(AcousticViscosity);

InputParameters
AcousticViscosity::validParams()
{
  InputParameters params = Kernel::validParams();
  params.addClassDescription("The Laplacian operator ($-\\nabla \\cdot \\nabla u$), with the weak form of $(\\nabla \\phi_i, \\nabla u_h)$.");
  params.addParam<MaterialPropertyName>(
      "inv_co_sq", "inv_co_sq", "Inverse of sqaure of the fluid's speed of sound.");
  params.addRequiredParam<Real>("viscosity", "fluid viscosity");
  params.addRequiredParam<Real>("densityF", "fluid density");
  return params;
}

AcousticViscosity::AcousticViscosity(const InputParameters & parameters) : Kernel(parameters),
_inv_co_sq(getMaterialProperty<Real>("inv_co_sq")),
_viscosity(getParam<Real>("viscosity")),
_densityF(getParam<Real>("densityF"))
{
  // _grad_dot_req = &(_var.gradSlnDot());
  // _grad_dot_req = (_sys.solutionUDot());
  // auto & u_dot = *_sys.solutionUDot();
  // NumericVector<Number> & u_dot = *_sys.solutionUDot();
  _grad_dot_req = &(_var.gradSlnOld());
  _second_sln = &(_var.secondSln());
  _second_sln_old = &(_var.secondSlnOld());
}

Real
AcousticViscosity::computeQpResidual()
{
  // MooseArray<VectorValue<Real>> grad_dot(_qrule->n_points(), 0);
  // for (const auto qp_index : index_range(_qrule->n_points()))
  //   for (const auto i : index_range(_dof_indices.size()))
  //     grad_dot[qp] += _dof_values_dot[i] * _grad_phi[i][qp];
  // auto * grad_dot = &_var.gradSlnDot();
  // auto & grad_dot = *_sys.solutionUDot();
  // NumericVector<Number> & u_dot = *_sys.solutionUDot();
  // const MooseVariableFE<double>::FieldVariableGradient * u_dot = &_var.gradSlnDot();
  // const VariableValue * u_dot = &_sys.gradDot();
  // return 4/3 * _viscosity * _inv_co_sq[_qp] / _densityF * _grad_dot_req[_qp] * _grad_test[_i][_qp];
  // std::cout << 4/3 * _viscosity * _inv_co_sq[_qp] / _densityF * (_grad_u[_qp] - (*_grad_dot_req)[_qp]) / _dt * _grad_test[_i][_qp] << std::endl;
  TensorValue<Real> siz = (*_second_sln)[_qp];
  TensorValue<Real> siz1 = (*_second_sln_old)[_qp];
  std::cout << siz << std::endl;
  std::cout << siz1 << std::endl;
  return 4/3 * _viscosity * _inv_co_sq[_qp] / _densityF * (_grad_u[_qp] - (*_grad_dot_req)[_qp]) / _dt * _grad_test[_i][_qp];
  // return 0.0;
}

Real
AcousticViscosity::computeQpJacobian()
{
  // return 4/3 * _viscosity * _inv_co_sq[_qp] / _densityF * _grad_phi_dot[_j][_qp] * _grad_test[_i][_qp];
  // std::cout << _grad_phi_old[_j][_qp] << std::endl;
  return 4/3 * _viscosity * _inv_co_sq[_qp] / _densityF * _grad_phi[_j][_qp] * _grad_test[_i][_qp];
}
