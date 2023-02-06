//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningGPDecision.h"

class BifidelityActiveLearningGPDecision : public ActiveLearningGPDecision
{
public:
  static InputParameters validParams();
  BifidelityActiveLearningGPDecision(const InputParameters & parameters);

protected:
  /**
   * This is where most of the computations happen:
   *   - Data is accumulated for training
   *   - GP models are trained
   *   - Decision is made whether more data is needed for GP training
   */
  virtual void preNeedSample() override;

  /**
   * Based on the computations in preNeedSample, the decision to get more data is passed and results
   * from the GP fills @param val
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;
};