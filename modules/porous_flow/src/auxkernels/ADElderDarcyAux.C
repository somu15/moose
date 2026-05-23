#include "ADElderDarcyAux.h"

registerMooseObject("PorousFlowApp", ADElderDarcyAux);

InputParameters
ADElderDarcyAux::validParams()
{
  MooseEnum component("x=0 y=1");
  InputParameters params = AuxKernel::validParams();
  params.addClassDescription("Outputs a component of the density-dependent Darcy velocity for "
                             "the Elder mass-transport benchmark.");
  params.addRequiredCoupledVar("pressure", "Pressure variable p.");
  params.addRequiredCoupledVar("concentration", "Concentration variable c.");
  params.addParam<MooseEnum>("component", component, "Velocity component to output.");
  params.addParam<Real>("rho0", 1000.0, "Reference density.");
  params.addParam<Real>("beta", 200.0, "Density-concentration coefficient.");
  params.addParam<Real>("permeability", 4.845e-13, "Intrinsic permeability.");
  params.addParam<Real>("mu", 1.0e-3, "Dynamic viscosity.");
  params.addParam<Real>("gravity", 9.81, "Magnitude of gravity, acting in the negative y direction.");
  return params;
}

ADElderDarcyAux::ADElderDarcyAux(const InputParameters & parameters)
  : AuxKernel(parameters),
    _c(coupledValue("concentration")),
    _grad_p(coupledGradient("pressure")),
    _component(static_cast<unsigned int>(getParam<MooseEnum>("component"))),
    _rho0(getParam<Real>("rho0")),
    _beta(getParam<Real>("beta")),
    _permeability(getParam<Real>("permeability")),
    _mu(getParam<Real>("mu")),
    _gravity(getParam<Real>("gravity"))
{
}

Real
ADElderDarcyAux::rho() const
{
  return _rho0 + _beta * _c[_qp];
}

Real
ADElderDarcyAux::computeValue()
{
  const auto mobility = _permeability / _mu;
  const RealVectorValue gravity_vector(0.0, -_gravity, 0.0);
  const auto velocity = -mobility * (_grad_p[_qp] - rho() * gravity_vector);
  return velocity(_component);
}
