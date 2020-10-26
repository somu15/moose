//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Dummy.h"
#include "Distribution.h"
#include "Normal.h"

registerMooseObjectAliased("StochasticToolsApp", Dummy, "Dummy");
registerMooseObjectReplaced("StochasticToolsApp",
                            Dummy,
                            "07/01/2020 00:00",
                            MonteCarlo);

InputParameters
Dummy::validParams()
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
  params.addRequiredParam<std::vector<Real>>(
      "proposal_std",
      "Standard deviations of the proposal distributions");
  params.addParam<std::vector<Real>>(
      "vpp_limits",
      "Limit values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "seeds",
      "Seed input values to get the MCMC sampler started");
  params.addParam<Real>(
      "num_samples_adaptive",
      "Number of samples after which adaptive MCMC is activated");
  return params;
}

Dummy::Dummy(const InputParameters & parameters)
  : Sampler(parameters), VectorPostprocessorInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _vpp_names(getParam<std::vector<std::string>>("vpp_names")),
    _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _vpp_limits(getParam<std::vector<Real>>("vpp_limits")),
    _seeds(getParam<std::vector<Real>>("seeds")),
    _num_samples_adaptive(getParam<Real>("num_samples_adaptive")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep()),
    _perf_compute_sample(registerTimedSection("computeSample", 4))
{
  for (const DistributionName & name : _distribution_names)
    _distributions.push_back(&getDistributionByName(name));

  setNumberOfRows(getParam<dof_id_type>("num_rows"));
  setNumberOfCols(_distributions.size());
  _new_sample_vec.resize(_distributions.size());
  _new_sample_sca = 0.0;
  _acceptance_ratio = 0.0;
  _consecutive_indicator = 0;
  _sum_inputs.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
  _diff_inputs.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
  _store_sample.resize(_distributions.size() * getParam<dof_id_type>("num_rows"));
}

void
Dummy::sampleSetUp()
{
  if (isParamValid("results_vpp"))
  {
    _values_distributed = isVectorPostprocessorDistributed("results_vpp");
    _values_ptr.resize(_vpp_names.size() + 1);
    for (unsigned int i = 0; i < _vpp_names.size(); ++i)
    {
      _values_ptr[i] = getVectorPostprocessorValue(
          "results_vpp", _vpp_names[i], !_values_distributed);
    }
  }

  _values_distributed1 = isVectorPostprocessorDistributed("inputs_vpp");
  _inputs_ptr.resize(_inputs_names.size() + 1);
  for (unsigned int i = 0; i < _inputs_names.size(); ++i)
  {
    _inputs_ptr[i] = getVectorPostprocessorValue(
        "inputs_vpp", _inputs_names[i], !_values_distributed1);
  }
}

// Real
// Dummy::computeSample(dof_id_type row_index, dof_id_type col_index)
// {
//   TIME_SECTION(_perf_compute_sample);
//   dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
//   if (_step > 1)
//   {
//     if (isParamValid("results_vpp"))
//     {
//       Real density_new = 1.0;
//       Real density_old = 1.0;
//       bool indicator_vpp = 1;
//       if (col_index == 0)
//       {
//         for (dof_id_type i = 0; i < _distributions.size(); ++i)
//         {
//           tmp1 = _sum_inputs[row_index * _distributions.size() + col_index];
//           _sum_inputs[row_index * _distributions.size() + col_index] = _sum_inputs[row_index * _distributions.size() + col_index] + (_inputs_ptr[col_index][row_index-offset] - _sum_inputs[row_index * _distributions.size() + col_index]) / (_step - 1);
//           tmp2 = ((_step-2) * Utility::pow<2>(_diff_inputs[row_index * _distributions.size() + col_index]) / (_step-1) + Utility::pow<2>(_inputs_ptr[col_index][row_index-offset] - _sum_inputs[row_index * _distributions.size() + col_index]) / (_step-1));
//           _diff_inputs[row_index * _distributions.size() + col_index] = std::pow(tmp2, 0.5);
//           if (isParamValid("num_samples_adaptive") && _step > _num_samples_adaptive)
//           {
//             _proposal_std[i] = _diff_inputs[row_index * _distributions.size() + col_index];
//           }
//           std::cout << _proposal_std[i] << std::endl;
//           _new_sample_vec[i] = Normal::quantile(getRand(), _inputs_ptr[i][row_index-offset], _proposal_std[i]);
//           density_new = density_new * _distributions[i]->pdf(_new_sample_vec[i]);
//           density_old = density_old * _distributions[i]->pdf(_inputs_ptr[i][row_index]);
//         }
//         for (dof_id_type i = 0; i < _vpp_names.size(); ++i)
//         {
//           indicator_vpp = indicator_vpp * ((std::abs(_values_ptr[i][row_index-offset]) >= std::abs(_vpp_limits[i])) ? 1 : 0);
//         }
//         // std::cout << density_new / density_old << std::endl;
//         density_new = density_new * indicator_vpp;
//         _acceptance_ratio = density_new / density_old;
//         if (_acceptance_ratio > getRand())
//         {
//           _consecutive_indicator = 1;
//           return _new_sample_vec[col_index];
//         } else
//         {
//           _consecutive_indicator = 0;
//           return _inputs_ptr[col_index][row_index-offset];
//         }
//       } else
//       {
//         if (_consecutive_indicator == 1)
//         {
//           return _new_sample_vec[col_index];
//         } else
//         {
//           return _inputs_ptr[col_index][row_index-offset];
//         }
//       }
//     } else
//     {
//       _new_sample_sca = Normal::quantile(getRand(), _inputs_ptr[col_index][row_index-offset], _proposal_std[col_index]);
//       _acceptance_ratio = _distributions[col_index]->pdf(_new_sample_sca) / _distributions[col_index]->pdf(_inputs_ptr[col_index][row_index-offset]);
//       if (_acceptance_ratio > getRand())
//       {
//         return _new_sample_sca;
//       } else
//       {
//         return _inputs_ptr[col_index][row_index-offset];
//       }
//     }
//   } else
//   {
//     return _seeds[col_index];
//   }
// }

Real
Dummy::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step > 1)
  {
    if (isParamValid("results_vpp"))
    {
      return _seeds[col_index];
    } else
    {
      // std::cout << getGlobalSamples() << std::endl;
      _new_sample_sca = Normal::quantile(getRand(), _inputs_ptr[col_index][row_index-offset], _proposal_std[col_index]);
      _acceptance_ratio = _distributions[col_index]->pdf(_new_sample_sca) / _distributions[col_index]->pdf(_inputs_ptr[col_index][row_index-offset]);
      if (_acceptance_ratio > getRand())
      {
        return _new_sample_sca;
      } else
      {
        return _inputs_ptr[col_index][row_index-offset];
      }
    }
  } else
  {
    return _seeds[col_index];
  }
}


// density_new = 1.0;
// density_old = 1.0;
// indicator_vpp = 1;
// if (col_index == 0)
// {
//   for (dof_id_type i = 0; i < _distributions.size(); ++i)
//   {
//     // tmp1 = _sum_inputs[row_index * _distributions.size() + col_index];
//     // _sum_inputs[row_index * _distributions.size() + col_index] = _sum_inputs[row_index * _distributions.size() + col_index] + (_inputs_ptr[col_index][row_index-offset] - _sum_inputs[row_index * _distributions.size() + col_index]) / (_step - 1);
//     // tmp2 = ((_step-2) * Utility::pow<2>(_diff_inputs[row_index * _distributions.size() + col_index]) / (_step-1) + Utility::pow<2>(_inputs_ptr[col_index][row_index-offset] - _sum_inputs[row_index * _distributions.size() + col_index]) / (_step-1));
//     // _diff_inputs[row_index * _distributions.size() + col_index] = std::pow(tmp2, 0.5);
//     // if (isParamValid("num_samples_adaptive") && _step > _num_samples_adaptive)
//     // {
//     //   _proposal_std[i] = _diff_inputs[row_index * _distributions.size() + col_index];
//     // }
//     // std::cout << _proposal_std[i] << std::endl;
//     density_new = density_new * _distributions[i]->pdf(_store_sample[row_index * _distributions.size() + col_index]);
//     density_old = density_old * _distributions[i]->pdf(_inputs_ptr[i][row_index]);
//     _new_sample_vec[i] = Normal::quantile(getRand(), _inputs_ptr[i][row_index-offset], _proposal_std[i]);
//     _store_sample[row_index * _distributions.size() + col_index] = _new_sample_vec[i];
//   }
//   for (dof_id_type i = 0; i < _vpp_names.size(); ++i)
//   {
//     indicator_vpp = indicator_vpp * ((std::abs(_values_ptr[i][row_index-offset]) >= std::abs(_vpp_limits[i])) ? 1 : 0);
//   }
//   // std::cout << density_new / density_old << std::endl;
//   density_new = density_new * indicator_vpp;
//   _acceptance_ratio = density_new / density_old;
//   if (_acceptance_ratio > getRand())
//   {
//     _consecutive_indicator = 1;
//     return _new_sample_vec[col_index];
//   } else
//   {
//     _consecutive_indicator = 0;
//     return _inputs_ptr[col_index][row_index-offset];
//   }
// } else
// {
//   if (_consecutive_indicator == 1)
//   {
//     return _new_sample_vec[col_index];
//   } else
//   {
//     return _inputs_ptr[col_index][row_index-offset];
//   }
// }
