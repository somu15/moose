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
#include "ActiveLearningGPDecision.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"

class BifidelityActiveLearningDecision : public ActiveLearningGPDecision
{
public:
  static InputParameters validParams();
  BifidelityActiveLearningDecision(const InputParameters & parameters);

protected:
  /**
   * This evaluates the inputted function to determine whether a multiapp solve is
   * necessary/allowed, otherwise it replaces the "transferred" quantity with a
   * default value.
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;

private:

  /// Model output value from low-fidelity SubApp
  const std::vector<Real> & _lf_value;

  /// Modified value of model output by this reporter class
  std::vector<Real> & _lf_outputs;

};
