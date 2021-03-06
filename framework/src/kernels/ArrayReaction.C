//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayReaction.h"

registerMooseObject("MooseApp", ArrayReaction);

template <>
InputParameters
validParams<ArrayReaction>()
{
  InputParameters params = validParams<ArrayKernel>();
  params.addParam<MaterialPropertyName>("reaction_coefficient", "The name of the reactivity");
  MooseEnum opt("scalar=0 array=1 full=2", "array");
  params.addParam<MooseEnum>("reaction_coefficient_type", opt, "Reaction coefficient type");
  params.addClassDescription("The array reaction operator with the weak "
                             "form of $(\\psi_i, u_h)$.");
  return params;
}

ArrayReaction::ArrayReaction(const InputParameters & parameters)
  : ArrayKernel(parameters), _r_type(getParam<MooseEnum>("reaction_coefficient_type"))
{
  if (_r_type == 0)
    _r = &getMaterialProperty<Real>("reaction_coefficient");
  else if (_r_type == 1)
    _r_array = &getMaterialProperty<RealEigenVector>("reaction_coefficient");
  else if (_r_type == 2)
    _r_2d_array = &getMaterialProperty<RealEigenMatrix>("reaction_coefficient");
}

RealEigenVector
ArrayReaction::computeQpResidual()
{
  if (_r_type == 0)
    return (*_r)[_qp] * _u[_qp] * _test[_i][_qp];

  else if (_r_type == 1)
  {
    mooseAssert((*_r_array)[_qp].size() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    return ((*_r_array)[_qp].array() * _u[_qp].array()) * _test[_i][_qp];
  }

  else
  {
    mooseAssert((*_r_2d_array)[_qp].cols() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    mooseAssert((*_r_2d_array)[_qp].rows() == _var.count(),
                "reaction_coefficient size is inconsistent with the number of components of array "
                "variable");
    return (*_r_2d_array)[_qp] * _u[_qp] * _test[_i][_qp];
  }
}

RealEigenVector
ArrayReaction::computeQpJacobian()
{
  if (_r_type == 0)
    return RealEigenVector::Constant(_var.count(), _phi[_j][_qp] * _test[_i][_qp] * (*_r)[_qp]);
  else if (_r_type == 1)
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_array)[_qp];

  else
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_2d_array)[_qp].diagonal();
}

RealEigenMatrix
ArrayReaction::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number() && _r_type == 2)
    return _phi[_j][_qp] * _test[_i][_qp] * (*_r_2d_array)[_qp];
  else
    return ArrayKernel::computeQpOffDiagJacobian(jvar);
}
