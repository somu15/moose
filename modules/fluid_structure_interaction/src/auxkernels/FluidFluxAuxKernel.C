//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidFluxAuxKernel.h"

registerMooseObject("MooseApp", FluidFluxAuxKernel);

InputParameters
FluidFluxAuxKernel::validParams()
{
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Calculates the flux on the fluid side.");
  params.addRequiredCoupledVar("pressure", "pressure variable");
  params.addRequiredParam<Real>("fluiddens", "fluid density");
  return params;
}

FluidFluxAuxKernel::FluidFluxAuxKernel(const InputParameters & parameters)
  : AuxKernel(parameters),
    _grad_press(coupledGradient("pressure")),
    _fluiddens(getParam<Real>("fluiddens"))
{
}

Real
FluidFluxAuxKernel::computeValue()
{
  return _fluiddens * _grad_press[_qp](0) + _fluiddens * _grad_press[_qp](1) + _fluiddens * _grad_press[_qp](2);
}
