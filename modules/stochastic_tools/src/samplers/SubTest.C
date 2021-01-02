//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubTest.h"
#include "Distribution.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"
#include "KernelDensity1D.h"

registerMooseObjectAliased("StochasticToolsApp", SubTest, "SubTest");
registerMooseObjectReplaced("StochasticToolsApp",
                            SubTest,
                            "07/01/2020 00:00",
                            SubTest);

InputParameters
SubTest::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addRequiredParam<ReporterName>(
      "results_reporter", "Reporter with results of samples created by trainer.");
  // params.addRequiredParam<ReporterName>(
  //     "inputs_reporter", "Reporter with inputs of samples created by trainer.");
  // params.addRequiredParam<VectorPostprocessorName>(
  //     "inputs_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  // params.addRequiredParam<std::vector<std::string>>(
  //     "inputs_names",
  //     "Name of vector from vectorpostprocessor with inputs of samples created by trainer");
  params.addRequiredParam<Real>(
      "subset_probability",
      "Conditional probability of each subset");
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distribution");
  params.addParam<int>(
      "num_samplessub",
      "Number of samples per subset");
  return params;
}

SubTest::SubTest(const InputParameters & parameters)
  : Sampler(parameters), VectorPostprocessorInterface(this), ReporterInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    // _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
    _num_samplessub(getParam<int>("num_samplessub")),
    _subset_probability(getParam<Real>("subset_probability")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 0))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  _new_sample_sca = 0.0;
  _acceptance_ratio = 0.0;
  _consecutive_indicator = 0;
  _inputs_sto.resize(_distributions.size());
  _markov_seed.resize(_distributions.size());
  _inputs_sorted.resize(_distributions.size());
  _index1 = 0;
  _rnd1 = 0.0;
  _subset = 0;
  _check_even = 0;
  setNumberOfRandomSeeds(100000);
  _prev_val.resize(_distributions.size());
  for (unsigned i = 0; i < _distributions.size(); ++i)
    _prev_val[i].resize(getParam<dof_id_type>("num_rows"));
}

// void
// SubTest::sampleSetUp()
// {
//   _values_distributed1 = isVectorPostprocessorDistributed("inputs_vpp");
//   _inputs_ptr.clear();
//   _inputs_ptr.reserve(_inputs_names.size());
//   for (const auto & name : _inputs_names)
//     _inputs_ptr.push_back(&getVectorPostprocessorValue("inputs_vpp", name, !_values_distributed1));
// }

std::vector<Real>
SubTest::sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  tmp.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = outputs[i];
  }
  std::sort(tmp.begin(), tmp.end());
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < (out.size()); ++i)
  {
    out[i] = tmp[i + std::round(samplessub * (1-subset_prob))];
  }
  return out;
}

std::vector<Real>
SubTest::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
{
  std::vector<Real> tmp;
  std::vector<Real> tmp1;
  tmp.resize(samplessub);
  tmp1.resize(samplessub);
  for (unsigned int i = ((subset-1) * samplessub); i < (subset * samplessub); ++i)
  {
    tmp[i - ((subset-1) * samplessub)] = outputs[i];
    tmp1[i - ((subset-1) * samplessub)] = inputs[i];
  }
  std::vector<int> tmp2(tmp.size());
  std::iota(tmp2.begin(), tmp2.end(), 0);
  auto comparator = [&tmp](int a, int b){ return tmp[a] < tmp[b]; };
  std::sort(tmp2.begin(), tmp2.end(), comparator);
  std::vector<Real> out;
  out.resize(std::floor(samplessub * subset_prob));
  for (unsigned int i = 0; i < out.size(); ++i)
  {
    out[i] = tmp1[tmp2[i + std::ceil(samplessub * (1-subset_prob))]];
  }
  return out;
}

Real
SubTest::computeMIN(const std::vector<Real> & data)
{
  Real tmp = data[0];
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    if(tmp>data[i])
    {
       tmp=data[i];
    }
  }
  return tmp;
}

// Real
// SubTest::computeSample(dof_id_type row_index, dof_id_type col_index)
// {
//   TIME_SECTION(_perf_compute_sample);
//   dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
//   if (_step <= _num_samplessub)
//   {
//     _subset = std::floor(_step / _num_samplessub);
//     if (_step > 1 && col_index == 0 && _check_even != _step)
//     {
//       for (dof_id_type j = 0; j < _distributions.size(); ++j)
//         _inputs_sto[j].push_back((*_inputs_ptr[j])[row_index-offset]);
//         // _inputs_sto[j].push_back(getReporterValue<std::vector<Real>>("inputs_reporter")[j]);
//       _outputs_sto.push_back(std::abs(getReporterValue<Real>("results_reporter")));
//     }
//     _check_even = _step;
//     return _distributions[col_index]->quantile(getRand(_step));
//   } else
//   {
//     _subset = (std::floor((_step-1) / _num_samplessub));
//     if (col_index == 0 && _check_even != _step)
//     {
//       _count_max = std::floor(1 / _subset_probability);
//       if (_subset > (std::floor((_step-2) / _num_samplessub)))
//       {
//         _ind_sto = -1;
//         _count = 1000000;
//         _output_sorted = SubTest::sortOUTPUT(_outputs_sto, _num_samplessub, _subset, _subset_probability);
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//         {
//           _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
//           _inputs_sorted[j] = SubTest::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
//         }
//         _output_limits.push_back(SubTest::computeMIN(_output_sorted));
//       }
//       if ( std::abs(getReporterValue<Real>("results_reporter")) >= _output_limits[_subset-1]) //
//       {
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//           _inputs_sto[j].push_back((*_inputs_ptr[j])[row_index-offset]);
//           // _inputs_sto[j].push_back(getReporterValue<std::vector<Real>>("inputs_reporter")[j]);
//         _outputs_sto.push_back(std::abs(getReporterValue<Real>("results_reporter")));
//       } else
//       {
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//           _inputs_sto[j].push_back(_inputs_sto[j][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]);
//         _outputs_sto.push_back(_outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]);
//       }
//       if (_count > _count_max)
//       {
//         ++_ind_sto;
//         for (dof_id_type k = 0; k < _distributions.size(); ++k)
//           _markov_seed[k] = _inputs_sorted[k][_ind_sto];
//         _count = 0;
//       } else
//       {
//         for (dof_id_type k = 0; k < _distributions.size(); ++k)
//           _markov_seed[k] = _inputs_sto[k][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)];
//       }
//       ++_count;
//       Real rv;
//       for (dof_id_type i = 0; i < _distributions.size(); ++i)
//       {
//         rv = Normal::quantile(getRand(_step), _markov_seed[i], _proposal_std[i]);
//         _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));
//         if (_acceptance_ratio > std::log(getRand(_step)))
//           _new_sample_vec[i] = rv;
//         else
//           _new_sample_vec[i] = _markov_seed[i];
//       }
//     }
//     _check_even = _step;
//     return _new_sample_vec[col_index];
//   }
// }

Real
SubTest::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step <= _num_samplessub)
  {
    _subset = std::floor(_step / _num_samplessub);
    if (_step > 1 && col_index == 0 && _check_even != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j].push_back(_prev_val[j][row_index]);
        // _inputs_sto[j].push_back(getReporterValue<std::vector<Real>>("inputs_reporter")[j]);
      _outputs_sto.push_back(std::abs(getReporterValue<Real>("results_reporter")));
    }
    _check_even = _step;
    _prev_val[col_index][row_index] = _distributions[col_index]->quantile(getRand(_step));
    return _prev_val[col_index][row_index];
  } else
  {
    _subset = (std::floor((_step-1) / _num_samplessub));
    if (col_index == 0 && _check_even != _step)
    {
      _count_max = std::floor(1 / _subset_probability);
      if (_subset > (std::floor((_step-2) / _num_samplessub)))
      {
        _ind_sto = -1;
        _count = 1000000;
        _output_sorted = SubTest::sortOUTPUT(_outputs_sto, _num_samplessub, _subset, _subset_probability);
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
          _inputs_sorted[j] = SubTest::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
        }
        _output_limits.push_back(SubTest::computeMIN(_output_sorted));
      }
      if ( std::abs(getReporterValue<Real>("results_reporter")) >= _output_limits[_subset-1]) //
      {
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
          _inputs_sto[j].push_back(_prev_val[j][row_index]);
          // _inputs_sto[j].push_back(getReporterValue<std::vector<Real>>("inputs_reporter")[j]);
        _outputs_sto.push_back(std::abs(getReporterValue<Real>("results_reporter")));
      } else
      {
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
          _inputs_sto[j].push_back(_inputs_sto[j][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]);
        _outputs_sto.push_back(_outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]);
      }
      if (_count > _count_max)
      {
        ++_ind_sto;
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sorted[k][_ind_sto];
        _count = 0;
      } else
      {
        for (dof_id_type k = 0; k < _distributions.size(); ++k)
          _markov_seed[k] = _inputs_sto[k][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)];
      }
      ++_count;
      Real rv;
      for (dof_id_type i = 0; i < _distributions.size(); ++i)
      {
        rv = Normal::quantile(getRand(_step), _markov_seed[i], _proposal_std[i]);
        _acceptance_ratio = std::log(_distributions[i]->pdf(rv)) - std::log(_distributions[i]->pdf(_markov_seed[i]));
        if (_acceptance_ratio > std::log(getRand(_step)))
          _new_sample_vec[i] = rv;
        else
          _new_sample_vec[i] = _markov_seed[i];
      }
    }
    _check_even = _step;
    _prev_val[col_index][row_index] = _new_sample_vec[col_index];
    return _new_sample_vec[col_index];
  }
}
