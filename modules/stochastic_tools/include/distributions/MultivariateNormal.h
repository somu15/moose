//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "MultivariateDistribution.h"

/**
 * A class used to generate a MultivariateNormal distribution
 */
class MultivariateNormal : public MultivariateDistribution
{
public:
  static InputParameters validParams();

  MultivariateNormal(const InputParameters & parameters);

  virtual Real pdf(const DenseVector<Real> & x) const override;
  // virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const DenseVector<Real> & p) const override;

  static Real pdf(const DenseVector<Real> & x, const DenseVector<Real> & mean, const DenseMatrix<Real> & std_dev);
  // static Real cdf(const Real & x, const Real & mean, const Real & std_dev);
  static Real quantile(const DenseVector<Real> & p, const DenseVector<Real> & mean, const DenseMatrix<Real> & std_dev);

protected:

  /// The mean (or expectation) of the distribution (mu)
  const DenseVector<Real> & _mean;

  /// The standard deviation of the distribution (sigma)
  const DenseMatrix<Real> & _covariance;
};
