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
  // for (const DistributionName & name : _distribution_names)
  //   _distributions.push_back(&getDistributionByName(name));
  //
  // setNumberOfRows(getParam<dof_id_type>("num_rows"));
  // setNumberOfCols(_distributions.size());
  // _new_sample_vec.resize(_distributions.size());
  // _acceptance_ratio = 0.0;
  // _inputs_sto.resize(_distributions.size());
  // _inputs_sorted.resize(_distributions.size());
  // for (unsigned int i = 0; i < _distributions.size(); ++i)
  // {
  //   _inputs_sto[i].resize(getParam<dof_id_type>("num_rows"));
  //   _inputs_sorted[i].resize(getParam<dof_id_type>("num_rows"));
  // }
  // _outputs_sto.resize(getParam<dof_id_type>("num_rows"));
  // _output_sorted.resize(getParam<dof_id_type>("num_rows"));
  // _markov_seed.resize(_distributions.size());
  // _subset = 0;
  // // _check_even = 0;
  // _check_even.resize(getParam<dof_id_type>("num_rows"));
  // setNumberOfRandomSeeds(100000);

  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(1);
  setNumberOfCols(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  _acceptance_ratio = 0.0;
  _inputs_sto.resize(_distributions.size());
  _inputs_sorted.resize(_distributions.size());
  for (unsigned int i = 0; i < _distributions.size(); ++i)
  {
    _inputs_sto[i].resize(1);
    _inputs_sorted[i].resize(1);
  }
  _outputs_sto.resize(1);
  _output_sorted.resize(1);
  _markov_seed.resize(_distributions.size());
  _subset = 0;
  // _check_even = 0;
  _check_even.resize(1);
}

// std::vector<Real>
// SubsetSimulation::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
// {
//   std::vector<Real> tmp;
//   std::vector<Real> tmp1;
//   tmp.resize(samplessub);
//   tmp1.resize(samplessub);
//   for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
//   {
//     tmp[i - ((subset-1) * samplessub)] = outputs[i];
//     tmp1[i - ((subset-1) * samplessub)] = inputs[i];
//   }
//   std::vector<int> tmp2(tmp.size());
//   std::iota(tmp2.begin(), tmp2.end(), 0);
//   auto comparator = [&tmp](int a, int b){ return tmp[a] < tmp[b]; };
//   std::sort(tmp2.begin(), tmp2.end(), comparator);
//   std::vector<Real> out;
//   out.resize(std::floor(samplessub * subset_prob));
//   for (unsigned int i = 0; i < out.size(); ++i)
//   {
//     out[i] = tmp1[tmp2[i + std::ceil(samplessub * (1-subset_prob))]];
//   }
//   return out;
// }

Real
SubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  if (_step <= (_num_samplessub))
  {
    _subset = std::floor(_step / _num_samplessub);
    if (_step > 1 && col_index == 0 && _check_even[row_index] != _step)
    {
      // std::cout << getReporterValue<Real>("output_reporter") << std::endl;
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j][row_index].push_back(getReporterValueByName<Real>(_inputs_names[j]));
      _outputs_sto[row_index].push_back((_use_absolute_value) ? std::abs(getReporterValue<Real>("output_reporter")) : getReporterValue<Real>("output_reporter"));
    }
    _check_even[row_index] = _step;
    // std::cout << getReporterValue<Real>("output_reporter") << std::endl;
    return _distributions[col_index]->quantile(getRand());
  } else
  {
    _subset = (std::floor((_step-1) / _num_samplessub));
    if (col_index == 0 && _check_even[row_index] != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j][row_index].push_back(getReporterValueByName<Real>(_inputs_names[j]));
      _outputs_sto[row_index].push_back((_use_absolute_value) ? std::abs(getReporterValue<Real>("output_reporter")) : getReporterValue<Real>("output_reporter"));
      _count_max = std::floor(1 / _subset_probability);
      if (_subset > (std::floor((_step-2) / _num_samplessub)))
      {
        _ind_sto = -1;
        _count = INT_MAX;
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
        {
          _inputs_sorted[j][row_index].resize(std::floor(_num_samplessub * _subset_probability));
          _inputs_sorted[j][row_index] = AdaptiveMonteCarloUtils::sortINPUT(_inputs_sto[j][row_index], _outputs_sto[row_index], _num_samplessub, _subset, _subset_probability);
        }
      }
      if (_count >= _count_max)
      {
        ++_ind_sto;
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sorted[k][row_index][_ind_sto];
        _count = 0;
      } else
      {
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sto[k][row_index][_inputs_sto[k].size()-1];
      }
      ++_count;
      Real rv;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
      {
        rv = Uniform::quantile(getRand(), (_markov_seed[i] - _proposal_std[i]), (_markov_seed[i] + _proposal_std[i]));

        _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));
        if (_acceptance_ratio > std::log(getRand()))
          _new_sample_vec[i] = rv;
        else
          _new_sample_vec[i] = _markov_seed[i];

        // rv = std::exp(Normal::quantile(getRand(), std::log(_markov_seed[i]), _proposal_std[i]));
        // _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));
        // if (_acceptance_ratio > std::log(getRand()))
        //   _new_sample_vec[i] = rv;
        // else
        //   _new_sample_vec[i] = _markov_seed[i];

      }
    }
    _check_even[row_index] = _step;
    return _new_sample_vec[col_index];
  }
}
