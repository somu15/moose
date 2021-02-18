//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AIS.h"
#include "Distribution.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"
#include "KernelDensity1D.h"

registerMooseObjectAliased("StochasticToolsApp", AIS, "AIS");
registerMooseObjectReplaced("StochasticToolsApp",
                            AIS,
                            "07/01/2020 00:00",
                            MonteCarlo);

InputParameters
AIS::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Adaptive Importance Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  // params.addRequiredParam<ReporterName>(
  //     "results_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<ReporterName>(
      "output_reporter", "Reporter with results of samples created by trainer.");
  params.addRequiredParam<std::vector<ReporterName>>(
      "inputs_reporter", "Reporter with input parameters.");
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distributions");
  params.addRequiredParam<Real>(
      "output_limit",
      "Limit values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "seeds",
      "Seed input values to get the MCMC sampler started");
  params.addRequiredParam<int>(
      "num_samples_train",
      "Number of samples");
  params.addRequiredParam<Real>(
      "std_factor",
      "Factor to be multiplied to the standard deviation of the importance samples");
  params.addParam<bool>(
      "use_absolute_value", false,
      "Use absolute value of the sub app output");
  return params;
}

AIS::AIS(const InputParameters & parameters)
  : Sampler(parameters), ReporterInterface(this),
    _inputs_names(getParam<std::vector<ReporterName>>("inputs_reporter")),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _seeds(getParam<std::vector<Real>>("seeds")),
    _output_limit(getParam<Real>("output_limit")),
    _num_samples_train(getParam<int>("num_samples_train")),
    _std_factor(getParam<Real>("std_factor")),
    _use_absolute_value(getParam<bool>("use_absolute_value")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _inputs_sto.resize(_distributions.size());
  // std::cout << "Here D" << std::endl;
  for (unsigned int i = 0; i < _distributions.size(); ++i)
    _inputs_sto[i].push_back(Normal::quantile(_distributions[i]->cdf(_seeds[i]),0,1));
  // setNumberOfRandomSeeds(100000);
  _check_even = 0;
  _sum_R = 0.0;
  _prev_val.resize(_distributions.size());
  for (unsigned i = 0; i < _distributions.size(); ++i)
    _prev_val[i].resize(getParam<dof_id_type>("num_rows"));
}

Real
AIS::computeSTD(const std::vector<Real> & data)
{
  Real sum1 = 0.0, sq_diff1 = 0.0;
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sum1 += (data[i]);
  }
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sq_diff1 += std::pow(((data[i])-sum1/data.size()), 2); // std::log
  }
  return std::pow(sq_diff1 / data.size(), 0.5);
}

Real
AIS::computeMEAN(const std::vector<Real> & data)
{
  Real sum1 = 0.0;
  for (unsigned int i = 2; i < data.size(); ++i)
  {
    sum1 += (data[i]);
  }
  return (sum1 / data.size());
}

Real
AIS::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  // dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  // std::cout << "Offset is" << offset << std::endl;
  if (_step <= _num_samples_train)
  {
    // std::cout << "Here" << std::endl;
    if (_step > 1 && col_index == 0 && _check_even != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _prev_val[j][row_index] = Normal::quantile(_distributions[j]->cdf(getReporterValueByName<Real>(_inputs_names[j])),0,1);
      _acceptance_ratio = 0.0;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
        _acceptance_ratio += std::log(Normal::pdf(_prev_val[i][row_index],0,1)) - std::log(Normal::pdf(_inputs_sto[i][_inputs_sto[i].size()-1],0,1));
      if (_acceptance_ratio > std::log(getRand()))
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_prev_val[i][row_index]);
      } else
      {
        for (dof_id_type i = 0; i < _distributions.size(); ++i)
          _inputs_sto[i].push_back(_inputs_sto[i][_inputs_sto[i].size()-1]);
      }
    }
    // std::cout << "Here 4" << std::endl;
    _check_even = _step;
    _prev_val[col_index][row_index] = Normal::quantile(getRand(), _inputs_sto[col_index][_inputs_sto[col_index].size()-1], _proposal_std[col_index]);
    return _distributions[col_index]->quantile(Normal::cdf(_prev_val[col_index][row_index],0,1));
  } else
  {
    if (col_index == 0 && _check_even != _step)
    {
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
      {
        _prev_val[i][row_index] = (Normal::quantile(getRand(), computeMEAN(_inputs_sto[i]), _std_factor * computeSTD(_inputs_sto[i])));
      }
    }
    _check_even = _step;
    return _distributions[col_index]->quantile(Normal::cdf(_prev_val[col_index][row_index],0,1));
  }
}
