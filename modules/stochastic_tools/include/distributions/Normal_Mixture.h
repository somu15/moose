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
 * A class used to generate a Normal_Mixture distribution
 */
class Normal_Mixture : public Distribution
{
public:
  static InputParameters validParams();

  Normal_Mixture(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight);
  static Real cdf(const Real & x, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight);
  static Real quantile(const Real & p, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight);

protected:

  /// The mean (or expectation) vector of the mixture distribution (mu)
  const std::vector<Real> & _mean;

  /// The standard deviation vector of the mixture distribution (sigma)
  const std::vector<Real> & _standard_deviation;

  /// The weight vector of the mixture distribution (sigma)
  const std::vector<Real> & _weight;
};
