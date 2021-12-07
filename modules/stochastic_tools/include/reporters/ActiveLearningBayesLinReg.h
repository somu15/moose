//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningReporterBase.h"
// #include "GeneralReporter.h"
#include <Eigen/Dense>

// Forward declaration
template <typename T>
class ActiveLearningBayesLinRegTempl;

// Typedef for object registration
typedef ActiveLearningBayesLinRegTempl<Real> ActiveLearningBayesLinReg;

/**
 * This object is mainly meant for demonstration for eventual active learning
 * algrorithms, but could prove useful. Basically, it enables a inputted function
 * to determine if a multiapp solve is "possible". For instance, maybe a certain
 * sampled value needs to be possitive, this class can filter those samples out
 * and replace quantities of interest with a default value.
 */
template <typename T>
class ActiveLearningBayesLinRegTempl : public ActiveLearningReporterTempl<T>
{
public:
  static InputParameters validParams();
  ActiveLearningBayesLinRegTempl(const InputParameters & parameters);

protected:
  /// Model output value from SubApp
  const std::vector<Real> & _output_value;

  /**
   * This evaluates the inputted function to determine whether a multiapp solve is
   * necessary/allowed, otherwise it replaces the "transferred" quantity with a
   * default value.
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          T & val) override;

private:
  /// store the default value
  T _default_value;

  RealEigenMatrix _test;
  RealEigenMatrix _test_inv;
};
