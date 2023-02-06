//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BifidelityActiveLearningGPDecision.h"

registerMooseObjectAliased("StochasticToolsApp",
                           BifidelityActiveLearningGPDecision,
                           "BFALGPDecision");

InputParameters
BifidelityActiveLearningGPDecision::validParams()
{
  InputParameters params = ActiveLearningGPDecision::validParams();
  params.addClassDescription(
      "Evaluates a low-fidelity model, adds a GP correction, determines the "
      "prediction quality, launches full model if LF + GP prediction is inadequate, and retrains GP.");
  return params;
}

BifidelityActiveLearningGPDecision::BifidelityActiveLearningGPDecision(
    const InputParameters & parameters)
  : ActiveLearningGPDecision(parameters)
{
}

void
BifidelityActiveLearningGPDecision::preNeedSample()
{
  // Accumulate inputs and outputs if we previously decided we needed a sample
  if (_step > 1 && _decision)
  {
    // Accumulate data into _batch members
    setupData(_inputs, _outputs_global);

    // Retrain if we are outside the training phase
    if (_step >= _n_train)
      _al_gp.reTrain(_inputs_batch, _outputs_batch);
  }

  // Gather inputs for the current step
  _inputs = _inputs_global;

  // Evaluate GP and decide if we need more data if outside training phase
  if (_step >= _n_train)
    _decision = facilitateDecision();
}

bool
BifidelityActiveLearningGPDecision::needSample(const std::vector<Real> &,
                                     dof_id_type,
                                     dof_id_type global_ind,
                                     Real & val)
{
  if (!_decision)
    val = _gp_mean[global_ind];
  return _decision;
}