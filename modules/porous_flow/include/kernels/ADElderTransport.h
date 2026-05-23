#pragma once

#include "ADKernel.h"

class Function;

/**
 * Concentration equation for the MultiphysicsBench Elder mass-transport case:
 *
 *   d(theta_s c) / dt + u . grad(c) - div(theta_s tau_D_L grad(c)) = S_c.
 */
class ADElderTransport : public ADKernel
{
public:
  static InputParameters validParams();

  ADElderTransport(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  ADReal rho() const;
  ADRealVectorValue darcyVelocity() const;

  const ADVariableGradient & _grad_c;
  const ADVariableValue & _c_dot;
  const ADVariableValue & _c;
  const ADVariableGradient & _grad_p;

  const Real _rho0;
  const Real _beta;
  const Real _theta_s;
  const Real _tau_D_L;
  const Real _permeability;
  const Real _mu;
  const Real _gravity;

  const Function & _source;
};
