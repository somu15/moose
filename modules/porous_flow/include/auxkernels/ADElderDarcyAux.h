#pragma once

#include "AuxKernel.h"

/**
 * Outputs one component of the density-dependent Darcy velocity used by the
 * Elder benchmark kernels.
 */
class ADElderDarcyAux : public AuxKernel
{
public:
  static InputParameters validParams();

  ADElderDarcyAux(const InputParameters & parameters);

protected:
  virtual Real computeValue() override;

  Real rho() const;

  const VariableValue & _c;
  const VariableGradient & _grad_p;

  const unsigned int _component;

  const Real _rho0;
  const Real _beta;
  const Real _permeability;
  const Real _mu;
  const Real _gravity;
};
