#include "ADElderTransportPrescribedVelocity.h"

#include "Function.h"

registerMooseObject("PorousFlowApp", ADElderTransportPrescribedVelocity);

InputParameters
ADElderTransportPrescribedVelocity::validParams()
{
  InputParameters params = ADKernel::validParams();
  params.addClassDescription("Concentration transport equation with prescribed Darcy velocity "
                             "for the Elder mass-transport benchmark.");
  params.addRequiredCoupledVar("ux", "Prescribed x-component of Darcy velocity.");
  params.addRequiredCoupledVar("uy", "Prescribed y-component of Darcy velocity.");
  params.addParam<Real>("theta_s", 0.1, "Saturated water content coefficient.");
  params.addParam<Real>("tau_D_L", 3.56e-6, "Effective diffusion coefficient tau * D_L.");
  params.addParam<FunctionName>("source", "0", "Concentration source S_c.");
  return params;
}

ADElderTransportPrescribedVelocity::ADElderTransportPrescribedVelocity(
    const InputParameters & parameters)
  : ADKernel(parameters),
    _grad_c(_grad_u),
    _c_dot(_var.adUDot()),
    _ux(adCoupledValue("ux")),
    _uy(adCoupledValue("uy")),
    _theta_s(getParam<Real>("theta_s")),
    _tau_D_L(getParam<Real>("tau_D_L")),
    _source(getFunction("source"))
{
}

ADReal
ADElderTransportPrescribedVelocity::computeQpResidual()
{
  const auto source = _source.value(_t, _q_point[_qp]);
  const ADRealVectorValue velocity(_ux[_qp], _uy[_qp], 0.0);
  return (_theta_s * _c_dot[_qp] + velocity * _grad_c[_qp] - source) *
             _test[_i][_qp] +
         _theta_s * _tau_D_L * _grad_c[_qp] * _grad_test[_i][_qp];
}
