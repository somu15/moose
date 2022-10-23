//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"
#include "AdaptiveImportanceSampler.h"
#include "AdaptiveImportanceSamplerActiveLearning.h"

/**
 * AdaptiveImportanceStats will help make sample accept/reject decisions in adaptive Monte Carlo
 * schemes.
 */
class AdaptiveImportanceStats : public GeneralReporter
{
public:
  static InputParameters validParams();
  AdaptiveImportanceStats(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /// Means of the importance distributions
  std::vector<Real> & _mu_imp;

  /// Standard deviations of the importance distributions
  std::vector<Real> & _std_imp;

  /// Failure probability estimate
  std::vector<Real> & _pf;

  /// Coefficient of variation of failure probability
  std::vector<Real> & _cov_pf;

  /// Flag samples if the surrogate prediction was inadequate
  const std::vector<bool> * _flag_sample;

private:
  /// Track the current step of the main App
  const int & _step;

  /// The sampler
  Sampler & _sampler;

  /// Adaptive Importance Sampler
  const AdaptiveImportanceSampler * const _ais;

  // Adaptive Importance Sampler with Active Learning
  const AdaptiveImportanceSamplerActiveLearning * const _ais_al;

  /// Ensure that the MCMC algorithm proceeds in a sequential fashion
  int _check_step;

  /// Storage for the sequential sum of pf
  Real _pf_sum;

  /// Storage for the sequential sum of variance of pf
  Real _var_sum;

  /// Storage for the distribution names
  std::vector<const Distribution *> _distributions_store;

  /// Storage for the standard deviation factor over the importance distribution
  Real _factor;
  
  /// Storage to whether or not to use an absolute value of the outputs
  bool _abs_value;

  /// Storage to for the user-specified output limit
  Real _output_lim;

  /// Storage to for the input values vector
  std::vector<Real> _input1;

  /// Storage to for the number of input parameters to the model
  unsigned int _num_cols;

  /// Storage to for the number of training samples for adaptive importance sampling
  int _train_samples;
};
