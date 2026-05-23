#pragma once

#include "ADKernel.h"

/**
 * Pressure equation for the conditional Elder q | c solve:
 *
 *   beta * epsilon_p * c_dot_cond + div(rho(c_cond) u) = 0
 *
 * with prescribed concentration fields c_cond and c_dot_cond, and Darcy velocity
 *
 *   u = -(k / mu) * (grad(p) - rho(c_cond) * g_vec).
 */
class ADElderContinuityPrescribedC : public ADKernel
{
public:
  static InputParameters validParams();

  ADElderContinuityPrescribedC(const InputParameters & parameters);

protected:
  virtual ADReal computeQpResidual() override;

  Real rho() const;
  ADRealVectorValue darcyVelocity() const;

  const VariableValue & _c;
  const VariableValue & _c_dot;
  const ADVariableGradient & _grad_p;

  const Real _rho0;
  const Real _beta;
  const Real _epsilon_p;
  const Real _permeability;
  const Real _mu;
  const Real _gravity;
};
