//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BifidelityActiveLearningDecision.h"
#include "Sampler.h"
#include "AdaptiveMonteCarloUtils.h"

#include <math.h>

registerMooseObjectAliased("StochasticToolsApp", BifidelityActiveLearningDecision, "BifidelityALDecision");

InputParameters
BifidelityActiveLearningDecision::validParams()
{
  InputParameters params = ActiveLearningGPDecision::validParams();
  params.addClassDescription(
      "Evaluates a low-fidelity model, correct it with a GP error term, "
      "determines quality of the corrected low-fidelity prediction, "
      "launches full model if corrected low-fidelity prediction is inadequate, and retrains GP.");
  params.addRequiredParam<ReporterName>("lf_value",
                                        "Value of the model output from the low-fidelity SubApp.");
  params.addParam<ReporterValueName>("lf_outputs", "lf_outputs", "Low-fidelity model outputs to be transmitted to the output file.");
  return params;
}

BifidelityActiveLearningDecision::BifidelityActiveLearningDecision(const InputParameters & parameters)
  : ActiveLearningGPDecision(parameters),
  _lf_value(getReporterValue<std::vector<Real>>("lf_value")),
  _lf_outputs(declareValue<std::vector<Real>>("lf_outputs"))
{
  _lf_outputs.resize(_sampler.getNumberOfRows());
}

bool
BifidelityActiveLearningDecision::needSample(const std::vector<Real> & row,
                                     dof_id_type local_ind,
                                     dof_id_type,
                                     Real & val)
{
  _gp_sto.resize(2);
  _output_parallel.resize(1);
  _gp_mean_parallel.resize(1);
  _gp_std_parallel.resize(1);
  DenseMatrix<Real> inputs_parallel(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  // Storing all the inputs and outputs (required for each time step of the mainApp)
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      inputs_parallel(ss, j) = data[j];
  }
  _output_parallel[local_ind] = val;
  _local_comm.sum(inputs_parallel.get_values());
  _local_comm.allgather(_output_parallel);
  _lf_outputs[local_ind] = _lf_value[0];
  _local_comm.allgather(_lf_outputs);
  std::vector<Real> hf_lf_diff = AdaptiveMonteCarloUtils::vectorDifference(_output_parallel, _lf_outputs);
  if (_step <= _n_train) // Wait until all the training data is generated
  {
    if (_step > 2)
      ActiveLearningGPDecision::setupData(hf_lf_diff, _inputs_batch_prev);
    if (_step == _n_train) // Once training data is generated, train the GP
    {
      if (local_ind == 0)
      {
        // std::cout << Moose::stringify(_outputs_batch) << std::endl;
        // for (dof_id_type ss = 0; ss < _sampler.getNumberOfCols(); ++ss)
        //   std::cout << Moose::stringify(_inputs_batch[ss]) << std::endl;
        _al_gp.reTrain(_inputs_batch, _outputs_batch);
      }
      // Setting up variables and making decisions
      ActiveLearningGPDecision::facilitateDecision(row, local_ind, val, true, &_lf_value[0], _outputs_batch);
      _local_comm.allgather(_gp_mean_parallel);
      _local_comm.allgather(_gp_std_parallel);
      ActiveLearningGPDecision::transferOutput(inputs_parallel, _gp_mean_parallel, _gp_std_parallel);
    }
    // Start tracking the GP failures until a user-specified batch size is met
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  }
  else // Training data generation and GP training completed. Active learning starts.
  {
    bool retrain = _track_gp_fails >= _allowed_gp_fails;
    // If the number of GP fails is greater than the user-specified batch size, retrain GP
    if (retrain)
    {
      ActiveLearningGPDecision::setupData(hf_lf_diff, _inputs_batch_prev);
      _al_gp.reTrain(_inputs_batch, _outputs_batch);
      _track_gp_fails = 0;
    }
    // Setting up variables and making decisions
    ActiveLearningGPDecision::facilitateDecision(
        row, local_ind, val, retrain, &_lf_value[0], _outputs_batch);
    _local_comm.allgather(_gp_mean_parallel);
    _local_comm.allgather(_gp_std_parallel);
    ActiveLearningGPDecision::transferOutput(inputs_parallel, _gp_mean_parallel, _gp_std_parallel);
    // Start re-tracking the GP failures until a user-specified batch size is met
    if (_track_gp_fails >= _allowed_gp_fails)
    {
      for (unsigned int i = 0; i < _flag_sample.size(); ++i)
        _decision[i] = true;
    }
  }
  // Storage of previous step inputs for usage in the next step
  for (dof_id_type ss = 0; ss < _sampler.getNumberOfRows(); ++ss)
  {
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      _inputs_batch_prev[ss][j] = inputs_parallel(ss, j);
  }
  return _decision[local_ind];
}
