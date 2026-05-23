#include "ADElderContinuityPrescribedC.h"

registerMooseObject("PorousFlowApp", ADElderContinuityPrescribedC);

InputParameters
ADElderContinuityPrescribedC::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Density-dependent Darcy continuity equation for the conditional "
                             "Elder q|c solve with prescribed c and c_dot fields.");
  params.addRequiredCoupledVar("concentration", "Prescribed concentration field c_cond.");
  params.addRequiredCoupledVar("concentration_dot", "Prescribed time derivative c_dot_cond.");
  params.addParam<Real>("rho0", 1000.0, "Reference density.");
  params.addParam<Real>("beta", 200.0, "Density-concentration coefficient.");
  params.addParam<Real>("epsilon_p", 0.1, "Porosity entering the density continuity storage.");
  params.addParam<Real>("permeability", 4.845e-13, "Intrinsic permeability.");
  params.addParam<Real>("mu", 1.0e-3, "Dynamic viscosity.");
  params.addParam<Real>("gravity", 9.81, "Magnitude of gravity, acting in the negative y direction.");
  return params;
}

ADElderContinuityPrescribedC::ADElderContinuityPrescribedC(const InputParameters & parameters)
  : ADKernel(parameters),
    _c(coupledValue("concentration")),
    _c_dot(coupledValue("concentration_dot")),
    _grad_p(_grad_u),
    _rho0(getParam<Real>("rho0")),
    _beta(getParam<Real>("beta")),
    _epsilon_p(getParam<Real>("epsilon_p")),
    _permeability(getParam<Real>("permeability")),
    _mu(getParam<Real>("mu")),
    _gravity(getParam<Real>("gravity"))
{
}

Real
ADElderContinuityPrescribedC::rho() const
{
  return _rho0 + _beta * _c[_qp];
}

ADRealVectorValue
ADElderContinuityPrescribedC::darcyVelocity() const
{
  const auto mobility = _permeability / _mu;
  const ADRealVectorValue gravity_vector(0.0, -_gravity, 0.0);
  return -mobility * (_grad_p[_qp] - rho() * gravity_vector);
}

ADReal
ADElderContinuityPrescribedC::computeQpResidual()
{
  const auto storage = _beta * _epsilon_p * _c_dot[_qp] * _test[_i][_qp];
  const auto mass_flux = rho() * darcyVelocity();
  return storage - mass_flux * _grad_test[_i][_qp];
}
