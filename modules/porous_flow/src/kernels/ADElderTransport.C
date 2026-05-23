#include "ADElderTransport.h"

#include "Function.h"

registerMooseObject("PorousFlowApp", ADElderTransport);

InputParameters
ADElderTransport::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription(
      "Concentration transport equation coupled to density-dependent Darcy flow for the Elder "
      "mass-transport benchmark.");
  params.addRequiredCoupledVar("pressure", "Pressure variable p.");
  params.addParam<Real>("rho0", 1000.0, "Reference density.");
  params.addParam<Real>("beta", 200.0, "Density-concentration coefficient.");
  params.addParam<Real>("theta_s", 0.1, "Saturated water content coefficient.");
  params.addParam<Real>("tau_D_L", 3.56e-6, "Effective diffusion coefficient tau * D_L.");
  params.addParam<Real>("permeability", 4.845e-13, "Intrinsic permeability.");
  params.addParam<Real>("mu", 1.0e-3, "Dynamic viscosity.");
  params.addParam<Real>("gravity", 9.81, "Magnitude of gravity, acting in the negative y direction.");
  params.addParam<FunctionName>("source", "0", "Concentration source S_c.");
  return params;
}

ADElderTransport::ADElderTransport(const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_c(_grad_u),
    _c_dot(_var.adUDot()),
    _c(_u),
    _grad_p(adCoupledGradient("pressure")),
    _rho0(getParam<Real>("rho0")),
    _beta(getParam<Real>("beta")),
    _theta_s(getParam<Real>("theta_s")),
    _tau_D_L(getParam<Real>("tau_D_L")),
    _permeability(getParam<Real>("permeability")),
    _mu(getParam<Real>("mu")),
    _gravity(getParam<Real>("gravity")),
    _source(getFunction("source"))
{
}

ADReal
ADElderTransport::rho() const
{
  return _rho0 + _beta * _c[_qp];
}

ADRealVectorValue
ADElderTransport::darcyVelocity() const
{
  const auto mobility = _permeability / _mu;
  const ADRealVectorValue gravity_vector(0.0, -_gravity, 0.0);
  return -mobility * (_grad_p[_qp] - rho() * gravity_vector);
}

ADReal
ADElderTransport::computeQpResidual()
{
  const auto source = _source.value(_t, _q_point[_qp]);
  return (_theta_s * _c_dot[_qp] + darcyVelocity() * _grad_c[_qp] - source) *
             _test[_i][_qp] +
         _theta_s * _tau_D_L * _grad_c[_qp] * _grad_test[_i][_qp];
}
