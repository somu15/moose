//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"

/**
 * A class used to generate a Gamma distribution
 */
class Gamma : public Distribution
{
public:
  static InputParameters validParams();

  Gamma(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const Real & alpha, const Real & beta);
  static Real cdf(const Real & x, const Real & alpha, const Real & beta);
  static Real quantile(const Real & p, const Real & alpha, const Real & beta);

protected:
  ///@{
  /// Coefficients for the rational function used to approximate the quantile
  // static const std::array<Real, 6> _a;
  // static const std::array<Real, 6> _b;
  ///@}

  /// The mean (or expectation) of the distribution (mu)
  const Real & _alpha;

  /// The standard deviation of the distribution (sigma)
  const Real & _beta;
};
