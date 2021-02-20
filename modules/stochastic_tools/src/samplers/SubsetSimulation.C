//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubsetSimulation.h"
#include "AdaptiveMonteCarloUtils.h"

registerMooseObjectAliased("StochasticToolsApp", SubsetSimulation, "SubsetSimulation");
registerMooseObjectReplaced("StochasticToolsApp",
                            SubsetSimulation,
                            "07/01/2020 00:00",
                            SubsetSimulation);

InputParameters
SubsetSimulation::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Subset Simulation Sampler.");
  // params.addRequiredParam<dof_id_type>("num_rows", "The number of independent Subset Simulations to run in parallel.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "output_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<Real>(
      "subset_probability",
      "Conditional probability of each subset");
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distribution");
  params.addRequiredParam<int>(
      "num_samplessub",
      "Number of samples per subset");
  params.addParam<bool>(
      "use_absolute_value", false,
      "Use absolute value of the sub app output");
  return params;
}

SubsetSimulation::SubsetSimulation(const InputParameters & parameters)
  : Sampler(parameters), ReporterInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _inputs_names(getParam<std::vector<ReporterName>>("inputs_reporter")),
    _num_samplessub(getParam<int>("num_samplessub")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _subset_probability(getParam<Real>("subset_probability")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 0))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(1);
  setNumberOfCols(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  _acceptance_ratio = 0.0;
  _inputs_sto.resize(_distributions.size());
  _inputs_sorted.resize(_distributions.size());
  _markov_seed.resize(_distributions.size());
  _subset = 0;
  _check_even = 0;
  setNumberOfRandomSeeds(100000);
}

Real
SubsetSimulation::computeSample(dof_id_type /*row_index*/, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  if (_step <= (_num_samplessub))
  {
    _subset = std::floor(_step / _num_samplessub);
    if (_step > 1 && col_index == 0 && _check_even != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j].push_back(getReporterValueByName<Real>(_inputs_names[j]));
      _outputs_sto.push_back((_use_absolute_value) ? std::abs(getReporterValue<Real>("output_reporter")) : getReporterValue<Real>("output_reporter"));
    }
    _check_even = _step;
    return _distributions[col_index]->quantile(getRand(_step));
  } else
  {
    _subset = (std::floor((_step-1) / _num_samplessub));
    if (col_index == 0 && _check_even != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j].push_back(getReporterValueByName<Real>(_inputs_names[j]));
      _outputs_sto.push_back((_use_absolute_value) ? std::abs(getReporterValue<Real>("output_reporter")) : getReporterValue<Real>("output_reporter"));
      _count_max = std::floor(1 / _subset_probability);
      if (_subset > (std::floor((_step-2) / _num_samplessub)))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
          _inputs_sorted[j] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
        }
      }
      if (_count >= _count_max)
      {
        ++_ind_sto;
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sorted[k][_ind_sto];
        _count = 0;
      } else
      {
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sto[k][_inputs_sto[k].size()-1];
      }
      ++_count;
      Real rv, rv1;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
      {
        // rv = Uniform::quantile(getRand(_step), (_markov_seed[i] - 0.5*_proposal_std[i]), (_markov_seed[i] + 0.5*_proposal_std[i]));
        rv = std::exp(Normal::quantile(getRand(_step), std::log(_markov_seed[i]), _proposal_std[i]));
        _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));

        // rv1 = Normal::quantile(_distributions[i]->cdf(_markov_seed[i]),0.0,1.0);
        // rv = Normal::quantile(getRand(), rv1, _proposal_std[i]);
        // _acceptance_ratio = std::log(Normal::pdf(rv, 0.0, 1.0)) - std::log(Normal::pdf(rv1, 0.0, 1.0));
        // std::cout << "Value is " << _acceptance_ratio << std::endl;

        // rv = std::exp(Normal::quantile(getRand(), std::log(_markov_seed[i]), _proposal_std[i]));
        // _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));
        if (_acceptance_ratio > std::log(getRand(_step)))
        {
          // std::cout << "Accepted" << std::endl;
          _new_sample_vec[i] = rv; // _distributions[i]->quantile(Normal::cdf(rv,rv1,_proposal_std[i]));
        } else
            _new_sample_vec[i] = _markov_seed[i];
      }
    }
    _check_even = _step;
    return _new_sample_vec[col_index];
  }
}
