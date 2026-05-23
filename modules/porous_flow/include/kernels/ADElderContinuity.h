#pragma once

#include "ADKernel.h"

/**
 * Pressure equation for the MultiphysicsBench Elder mass-transport case:
 *
 *   beta * d(epsilon_p c) / dt + div(rho(c) u) = 0
 *
 * with Darcy velocity
 *
 *   u = -(k / mu) * (grad(p) - rho(c) * g_vec).
 */
class ADElderContinuity : public ADKernel
{
public:
  static InputParameters validParams();

  ADElderContinuity(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  ADReal rho() const;
  ADRealVectorValue darcyVelocity() const;

  const ADVariableValue & _c;
  const ADVariableGradient & _grad_p;
  const ADVariableValue & _c_dot;

  const Real _rho0;
  const Real _beta;
  const Real _epsilon_p;
  const Real _permeability;
  const Real _mu;
  const Real _gravity;
};
