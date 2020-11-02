//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "FluidStructureInterface_Damping.h"
#include "MooseVariableFE.h"

registerMooseObject("FluidStructureInteractionApp", FluidStructureInterface_Damping);

InputParameters
FluidStructureInterface_Damping::validParams()
{
  InputParameters params = InterfaceKernel::validParams();
  params.addClassDescription("Enforces displacement and stress/pressure continuity "
                             "between the fluid and structural domains. Element "
                             "is always the structure and neighbor is always the"
                             " fluid.");
  params.addParam<MaterialPropertyName>("D", "D", "Fluid density.");
  params.addRequiredParam<Real>("viscosity", "Fluid viscosity.");
  params.addRequiredRangeCheckedParam<unsigned int>(
      "component", "component < 3", "The desired component of displacement.");
  return params;
}

FluidStructureInterface_Damping::FluidStructureInterface_Damping(const InputParameters & parameters)
  : InterfaceKernel(parameters),
    _D(getMaterialProperty<Real>("D")),
    _viscosity(getParam<Real>("viscosity")),
    _neighbor_dotdot(_neighbor_var.uDotDotNeighbor()),
    _neighbor_dotdot_du(_neighbor_var.duDotDotDuNeighbor()),
    _component(getParam<unsigned int>("component"))
{
  _second_sln = &(_var.secondSln());
  _second_sln_old = &(_var.secondSlnOld());
}

Real
FluidStructureInterface_Damping::computeQpResidual(Moose::DGResidualType type)
{
  Real r = 0;
  switch (_component)
  {
    case 0:
    {
      TensorValue<Real> siz = (*_second_sln)[_qp];
      TensorValue<Real> siz1 = (*_second_sln_old)[_qp];
      std::cout << siz << std::endl;
      std::cout << siz1 << std::endl;
      switch (type)
      {
        case Moose::Element: // Element is fluid
          r = _test[_i][_qp] * _D[_qp] * _neighbor_dotdot[_qp] * _normals[_qp](0) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_neighbor_value[_qp] * _normals[_qp](0);
          break;

        case Moose::Neighbor: // Neighbor is structure
          r = _test_neighbor[_i][_qp] * -_u[_qp] * _normals[_qp](0);
          break;
      }
      break;
    }
    case 1:
    {
      switch (type)
      {
        case Moose::Element: // Element is fluid
          r = _test[_i][_qp] * _D[_qp] * _neighbor_dotdot[_qp] * _normals[_qp](1) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_neighbor_value[_qp] * _normals[_qp](1);
          break;

        case Moose::Neighbor: // Neighbor is structure
          r = _test_neighbor[_i][_qp] * -_u[_qp] * _normals[_qp](1);
          break;
      }
      break;
    }
    case 2:
    {
      switch (type)
      {
        case Moose::Element: // Element is fluid
          r = _test[_i][_qp] * _D[_qp] * _neighbor_dotdot[_qp] * _normals[_qp](2) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_neighbor_value[_qp] * _normals[_qp](2);
          break;

        case Moose::Neighbor: // Neighbor is structure
          r = _test_neighbor[_i][_qp] * -_u[_qp] * _normals[_qp](2);
          break;
      }
      break;
    }
  }
  return r;
}

Real
FluidStructureInterface_Damping::computeQpJacobian(Moose::DGJacobianType type)
{
  Real jac = 0;
  switch (_component)
  {
    case 0:
    {
      switch (type)
      {
        case Moose::ElementElement:
          break;
        case Moose::NeighborNeighbor:
          break;
        case Moose::ElementNeighbor:
          jac = _test[_i][_qp] * _D[_qp] * _phi_neighbor[_j][_qp] * _neighbor_dotdot_du[_qp] * _normals[_qp](0) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_phi_neighbor[_j][_qp] * _normals[_qp](0);
          break;
        case Moose::NeighborElement:
          jac = _test_neighbor[_i][_qp] * -_phi[_j][_qp] * _normals[_qp](0);
          break;
      }
      break;
    }
    case 1:
    {
      switch (type)
      {
        case Moose::ElementElement:
          break;
        case Moose::NeighborNeighbor:
          break;
        case Moose::ElementNeighbor:
          jac = _test[_i][_qp] * _D[_qp] * _phi_neighbor[_j][_qp] * _neighbor_dotdot_du[_qp] * _normals[_qp](1) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_phi_neighbor[_j][_qp] * _normals[_qp](1);
          break;
        case Moose::NeighborElement:
          jac = _test_neighbor[_i][_qp] * -_phi[_j][_qp] * _normals[_qp](1);
          break;
      }
      break;
    }
    case 2:
    {
      switch (type)
      {
        case Moose::ElementElement:
          break;
        case Moose::NeighborNeighbor:
          break;
        case Moose::ElementNeighbor:
          jac = _test[_i][_qp] * _D[_qp] * _phi_neighbor[_j][_qp] * _neighbor_dotdot_du[_qp] * _normals[_qp](2) - _grad_test[_i][_qp] * 4 / 3 * _viscosity * _grad_phi_neighbor[_j][_qp] * _normals[_qp](2);
          break;
        case Moose::NeighborElement:
          jac = _test_neighbor[_i][_qp] * -_phi[_j][_qp] * _normals[_qp](2);
          break;
      }
      break;
    }
  }
  return jac;
}
