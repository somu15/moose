//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "CriticalTimeStep.h"

registerMooseObject("TensorMechanicsApp", CriticalTimeStep);

template <>
InputParameters
validParams<CriticalTimeStep>()
{
  InputParameters params = validParams<ElementPostprocessor>();
  params.addClassDescription(
      "Computes and reports the critical time step for the explicit solver.");
  params.addParam<MaterialPropertyName>(
      "density",
      "Name of Material Property  or a constant real number defining the density of the material.");
      params.addParam<Real>(
        "factor", 1.0,
      "Factor to mulitply to the critical time step.");
  return params;
}

CriticalTimeStep::CriticalTimeStep(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    GuaranteeConsumer(this),
    _mat_dens(getMaterialPropertyByName<Real>("density")),
    _effective_stiffness(getMaterialPropertyByName<Real>("effective_stiffness")),
    _factor(getParam<Real>("factor"))
{
}

void
CriticalTimeStep::initialSetup()
{
  if (!hasGuaranteedMaterialProperty("effective_stiffness", Guarantee::ISOTROPIC))
    mooseError("CriticalTimeStep can only be used with elasticity tensor materials "
               "that guarantee isotropic tensors.");
}

void
CriticalTimeStep::initialize()
{
  _total_size = std::numeric_limits<double>::max();
  _elems = 0;
}

void
CriticalTimeStep::execute()
{
  Real dens = _mat_dens[0];
  _total_size =
      std::min(_factor * _current_elem->hmin() * std::sqrt(dens) / (_effective_stiffness[0]), _total_size);
  _elems++;
}

Real
CriticalTimeStep::getValue()
{
  gatherSum(_total_size);
  gatherSum(_elems);

  return _total_size;
}

void
CriticalTimeStep::threadJoin(const UserObject & y)
{
  const CriticalTimeStep & pps = static_cast<const CriticalTimeStep &>(y);
  _total_size += pps._total_size;
  _elems += pps._elems;
}
