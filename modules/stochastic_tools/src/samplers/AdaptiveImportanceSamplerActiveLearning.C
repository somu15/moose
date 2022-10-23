//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AdaptiveImportanceSamplerActiveLearning.h"
#include "AdaptiveMonteCarloUtils.h"
#include "Distribution.h"
#include "Normal.h"
#include "Uniform.h"

registerMooseObjectAliased("StochasticToolsApp", AdaptiveImportanceSamplerActiveLearning, "AISActiveLearning");

InputParameters
AdaptiveImportanceSamplerActiveLearning::validParams()
{
  InputParameters params = AdaptiveImportanceSampler::validParams();
  params.addClassDescription("Adaptive Importance Sampler with Gaussian Process Active Learning.");
  params.addRequiredParam<ReporterName>("inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<ReporterName>("flag_sample",
                                        "Flag samples if the surrogate prediction was inadequate.");
  return params;
}

AdaptiveImportanceSamplerActiveLearning::AdaptiveImportanceSamplerActiveLearning(const InputParameters & parameters)
  : AdaptiveImportanceSampler(parameters),
    _flag_sample(getReporterValue<std::vector<bool>>("flag_sample")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _inputs(getReporterValue<std::vector<std::vector<Real>>>("inputs_reporter"))
{

  /* `inputs_sto` is a member variable that aids in forming the importance distribution.
     One dimension of this variable is equal to the number of distributions. The other dimension
     of the variable, at the last step, is equal to the number of samples the user desires.*/
  _inputs_sto.resize(_distributions.size());

  // Mapping all the input distributions to a standard normal space
  for (unsigned int i = 0; i < _distributions.size(); ++i)
    _inputs_sto[i].push_back(Normal::quantile(_distributions[i]->cdf(_initial_values[i]), 0, 1));

  /* `prev_value` is a member variable for tracking the previously accepted samples in the
     MCMC algorithm and proposing the next sample.*/
  _prev_value.resize(_distributions.size());

  // `check_step` is a member variable for ensuring that the MCMC algorithm proceeds in a sequential
  // fashion.
  _check_step = 0;

  // Storage for means of input values for proposing the next sample
  _mean_sto.resize(_distributions.size());

  // Storage for standard deviations of input values for proposing the next sample
  _std_sto.resize(_distributions.size());

}

Real
AdaptiveImportanceSamplerActiveLearning::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  const bool sample = _step > 1 && col_index == 0 && _check_step != _step;
  if (!_flag_sample[0])
  {
    if (_step <= _num_samples_train)
    {
        /* This is the importance distribution training step. Markov Chains are set up
        to sample from the importance region or the failure region using the Metropolis
        algorithm. Given that the previous sample resulted in a model failure, the next
        sample is proposed such that it is very likely to result in a model failure as well.
        The `initial_values` and `proposal_std` parameters provided by the user affects the
        formation of the importance distribution. */
        if (sample)
        {
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
            _prev_value[j] = Normal::quantile(_distributions[j]->cdf(_inputs[j][0]), 0.0, 1.0);
        Real acceptance_ratio = 0.0;
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
            acceptance_ratio += std::log(Normal::pdf(_prev_value[i], 0.0, 1.0)) -
                                std::log(Normal::pdf(_inputs_sto[i].back(), 0.0, 1.0));
        if (acceptance_ratio > std::log(getRand(_step)))
        {
            for (dof_id_type i = 0; i < _distributions.size(); ++i)
            _inputs_sto[i].push_back(_prev_value[i]);
        }
        else
        {
            for (dof_id_type i = 0; i < _distributions.size(); ++i)
            _inputs_sto[i].push_back(_inputs_sto[i].back());
        }
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
            _prev_value[i] = Normal::quantile(getRand(_step), _inputs_sto[i].back(), _proposal_std[i]);
        }
    }
    else if (sample)
    {
        /* This is the importance sampling step using the importance distribution created
        in the previous step. Once the importance distribution is known, sampling from
        it is similar to a regular Monte Carlo sampling. */
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
        {
        if (_step == _num_samples_train + 1)
        {
            _mean_sto[i] = AdaptiveMonteCarloUtils::computeMean(_inputs_sto[i], 1);
            _std_sto[i] = AdaptiveMonteCarloUtils::computeSTD(_inputs_sto[i], 1);
        }
        _prev_value[i] = (Normal::quantile(getRand(_step), _mean_sto[i], _std_factor * _std_sto[i]));
        }
    }
  }
  _check_step = _step;
  return _distributions[col_index]->quantile(Normal::cdf(_prev_value[col_index], 0.0, 1.0));
}
