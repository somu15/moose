#pragma once

#include "ADKernel.h"

class Function;

/**
 * Concentration equation for the Elder mass-transport case with prescribed
 * Darcy velocity:
 *
 *   d(theta_s c) / dt + u_cond . grad(c)
 *     - div(theta_s tau_D_L grad(c)) = S_c.
 */
class ADElderTransportPrescribedVelocity : public ADKernel
{
public:
  static InputParameters validParams();

  ADElderTransportPrescribedVelocity(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  const ADVariableGradient & _grad_c;
  const ADVariableValue & _c_dot;
  const ADVariableValue & _ux;
  const ADVariableValue & _uy;

  const Real _theta_s;
  const Real _tau_D_L;

  const Function & _source;
};
