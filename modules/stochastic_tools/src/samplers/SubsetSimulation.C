//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubsetSimulation.h"
#include "Distribution.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"
#include "KernelDensity1D.h"

registerMooseObjectAliased("StochasticToolsApp", SubsetSimulation, "SubsetSimulation");
registerMooseObjectReplaced("StochasticToolsApp",
                            SubsetSimulation,
                            "07/01/2020 00:00",
                            SubsetSimulation);

InputParameters
SubsetSimulation::validParams()
{
  InputParameters params = Sampler::validParams();
  params.addClassDescription("Monte Carlo Sampler.");
  params.addRequiredParam<dof_id_type>("num_rows", "The number of rows per matrix to generate.");
  params.addRequiredParam<std::vector<DistributionName>>(
      "distributions",
      "The distribution names to be sampled, the number of distributions provided defines the "
      "number of columns per matrix.");
  params.addParam<VectorPostprocessorName>(
      "results_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addRequiredParam<VectorPostprocessorName>(
      "inputs_vpp", "Vectorpostprocessor with results of samples created by trainer.");
  params.addParam<std::vector<std::string>>(
      "vpp_names",
      "Name of vector from vectorpostprocessor with results of samples created by trainer");
  params.addRequiredParam<std::vector<std::string>>(
      "inputs_names",
      "Name of vector from vectorpostprocessor with inputs of samples created by trainer");
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

SubsetSimulation::SubsetSimulation(const InputParameters & parameters)
  : Sampler(parameters), VectorPostprocessorInterface(this), ReporterInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _vpp_names(getParam<std::vector<std::string>>("vpp_names")),
    _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
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
  // Include exception for more than one output
}

void
SubsetSimulation::sampleSetUp()
{
  // Change output to not be a vector
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr.clear();
  _values_ptr.reserve(_vpp_names.size());
  _values_ptr.push_back(&getVectorPostprocessorValue("results_vpp", _vpp_names[0], !_values_distributed));

  // _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  // _values_ptr.resize(_vpp_names.size() + 1);
  // for (unsigned int i = 0; i < _vpp_names.size(); ++i)
  // {
  //   _values_ptr[i] = getVectorPostprocessorValue(
  //       "results_vpp", _vpp_names[i], !_values_distributed);
  // }

  _values_distributed1 = isVectorPostprocessorDistributed("inputs_vpp");
  _inputs_ptr.clear();
  _inputs_ptr.reserve(_inputs_names.size());
  for (const auto & name : _inputs_names)
    _inputs_ptr.push_back(&getVectorPostprocessorValue("inputs_vpp", name, !_values_distributed1));
  // _inputs_ptr.resize(_inputs_names.size() + 1);
  // for (unsigned int i = 0; i < _inputs_names.size(); ++i)
  // {
  //   _inputs_ptr[i] = getVectorPostprocessorValue(
  //       "inputs_vpp", _inputs_names[i], !_values_distributed1);
  // }
}

std::vector<Real>
SubsetSimulation::sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
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

// Efficient way to do the below operations???

std::vector<Real>
SubsetSimulation::sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob)
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
SubsetSimulation::computeMIN(const std::vector<Real> & data)
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
// SubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
// {
//   TIME_SECTION(_perf_compute_sample);
//   dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
//   if (_step <= _num_samplessub)
//   {
//     _subset = std::floor(_step / _num_samplessub);
//     if (_step > 1 && col_index == 0 && _check_even != _step)
//     {
//       for (dof_id_type j = 0; j < _distributions.size(); ++j)
//         _inputs_sto[j].push_back(_inputs_ptr[j][row_index-offset]);
//       _outputs_sto.push_back(std::abs(_values_ptr[0][row_index-offset]));
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
//         _output_sorted = SubsetSimulation::sortOUTPUT(_outputs_sto, _num_samplessub, _subset, _subset_probability);
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//         {
//           _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
//           _inputs_sorted[j] = SubsetSimulation::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
//         }
//         _output_limits.push_back(SubsetSimulation::computeMIN(_output_sorted));
//       }
//       if ( std::abs(_values_ptr[0][row_index-offset]) >= _output_limits[_subset-1])
//       {
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//           _inputs_sto[j].push_back(_inputs_ptr[j][row_index-offset]);
//         _outputs_sto.push_back(std::abs(_values_ptr[0][row_index-offset]));
//       } else
//       {
//         for (dof_id_type j = 0; j < _distributions.size(); ++j)
//           _inputs_sto[j].push_back(_inputs_sto[j][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]); // _inputs_sto[j].push_back(_inputs_sto[j][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)]);
//         _outputs_sto.push_back(_outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)]); // _outputs_sto.push_back(_outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)]);
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
//       // for (dof_id_type i = 0; i < _inputs_sto[0].size(); ++i)
//       // {
//       //   std::cout << _inputs_sto[0][i] << "," << _inputs_sto[1][i] << "," << _outputs_sto[i] << std::endl;
//       // }
//       // for (dof_id_type i = 0; i < _output_limits.size(); ++i)
//       //   std::cout << _output_limits[i] << std::endl;
//     }
//     _check_even = _step;
//     return _new_sample_vec[col_index];
//   }
// }

Real
SubsetSimulation::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step <= _num_samplessub)
  {
    _subset = std::floor(_step / _num_samplessub);
    if (_step > 1 && col_index == 0 && _check_even != _step)
    {
      for (dof_id_type j = 0; j < _distributions.size(); ++j)
        _inputs_sto[j].push_back((*_inputs_ptr[j])[row_index-offset]);
      _outputs_sto.push_back(std::abs((*_values_ptr[0])[row_index-offset]));
    }
    _check_even = _step;
    return _distributions[col_index]->quantile(getRand(_step));
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
        _output_sorted = SubsetSimulation::sortOUTPUT(_outputs_sto, _num_samplessub, _subset, _subset_probability);
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
        {
          _inputs_sorted[j].resize(std::floor(_num_samplessub * _subset_probability));
          _inputs_sorted[j] = SubsetSimulation::sortINPUT(_inputs_sto[j], _outputs_sto, _num_samplessub, _subset, _subset_probability);
        }
        _output_limits.push_back(SubsetSimulation::computeMIN(_output_sorted));
      }
      if ( std::abs((*_values_ptr[0])[row_index-offset]) >= _output_limits[_subset-1])
      {
        for (dof_id_type j = 0; j < _distributions.size(); ++j)
          _inputs_sto[j].push_back((*_inputs_ptr[j])[row_index-offset]);
        _outputs_sto.push_back(std::abs((*_values_ptr[0])[row_index-offset]));
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
      // for (dof_id_type i = 0; i < _inputs_sto[0].size(); ++i)
      // {
      //   std::cout << _inputs_sto[0][i] << "," << _inputs_sto[1][i] << "," << _outputs_sto[i] << std::endl;
      // }
      // for (dof_id_type i = 0; i < _output_limits.size(); ++i)
      //   std::cout << _output_limits[i] << std::endl;
    }
    _check_even = _step;
    return _new_sample_vec[col_index];
  }
}
