//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Dummy_MCMC_I.h"
#include "Distribution.h"
#include "Normal.h"
#include "TruncatedNormal.h"
#include "Uniform.h"
#include "KernelDensity1D.h"

registerMooseObjectAliased("StochasticToolsApp", Dummy_MCMC_I, "Dummy_MCMC_I");
registerMooseObjectReplaced("StochasticToolsApp",
                            Dummy_MCMC_I,
                            "07/01/2020 00:00",
                            MonteCarlo);

InputParameters
Dummy_MCMC_I::validParams()
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
  params.addParam<Real>(
      "vpp_limits",
      "Limit values of the VPPs");
  params.addRequiredParam<std::vector<Real>>(
      "seeds",
      "Seed input values to get the MCMC sampler started");
  params.addParam<Real>(
      "num_samples_adaptive",
      "Number of samples after which adaptive MCMC is activated");
  params.addParam<unsigned int>(
      "num_samples_chain",
      "Number of samples");
  params.addParam<int>(
      "num_samples_train",
      "Number of samples");
  return params;
}

Dummy_MCMC_I::Dummy_MCMC_I(const InputParameters & parameters)
  : Sampler(parameters), VectorPostprocessorInterface(this),
    _distribution_names(getParam<std::vector<DistributionName>>("distributions")),
    _vpp_names(getParam<std::vector<std::string>>("vpp_names")),
    _inputs_names(getParam<std::vector<std::string>>("inputs_names")),
    _proposal_std(getParam<std::vector<Real>>("proposal_std")),
    _vpp_limits(getParam<Real>("vpp_limits")),
    _seeds(getParam<std::vector<Real>>("seeds")),
    _num_samples_adaptive(getParam<Real>("num_samples_adaptive")),
    _num_samples_chain(getParam<unsigned int>("num_samples_chain")),
    _num_samples_train(getParam<int>("num_samples_train")),
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
  _inputs_sto.resize(_distributions.size());
  for (unsigned int i = 0; i < _distributions.size(); ++i)
  {
    _inputs_sto[i].resize((_num_samples_train) * (getParam<dof_id_type>("num_rows")));
  }
  _outputs_sto.resize((_num_samples_train) * (getParam<dof_id_type>("num_rows")));
  _index1 = 0;
  _rnd1 = 0.0;
  _impMCMC.resize(_distributions.size());
  _impRatio = 0.0;
  setNumberOfRandomSeeds(100000);
}

void
Dummy_MCMC_I::sampleSetUp()
{
  _values_distributed = isVectorPostprocessorDistributed("results_vpp");
  _values_ptr.resize(_vpp_names.size() + 1);
  for (unsigned int i = 0; i < _vpp_names.size(); ++i)
  {
    _values_ptr[i] = getVectorPostprocessorValue(
        "results_vpp", _vpp_names[i], !_values_distributed);
  }
  _values_distributed1 = isVectorPostprocessorDistributed("inputs_vpp");
  _inputs_ptr.resize(_inputs_names.size() + 1);
  for (unsigned int i = 0; i < _inputs_names.size(); ++i)
  {
    _inputs_ptr[i] = getVectorPostprocessorValue(
        "inputs_vpp", _inputs_names[i], !_values_distributed1);
  }
}


Real
Dummy_MCMC_I::computeSTD(const std::vector<Real> & data)
{
  Real sum1 = 0.0, sq_diff1 = 0.0;
  for (unsigned int i = 3; i < data.size(); ++i)
  {
    sum1 += data[i];
  }
  for (unsigned int i = 3; i < data.size(); ++i)
  {
    sq_diff1 += std::pow((data[i]-sum1/data.size()), 2);
  }
  return std::pow(sq_diff1 / data.size(), 0.5);
}

Real
Dummy_MCMC_I::computeMEAN(const std::vector<Real> & data)
{
  Real sum1 = 0.0;
  for (unsigned int i = 3; i < data.size(); ++i)
  {
    sum1 += data[i];
  }
  return (sum1 / data.size());
}

Real
Dummy_MCMC_I::computeMIN(const std::vector<Real> & data)
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

Real
Dummy_MCMC_I::computeMAX(const std::vector<Real> & data)
{
  Real tmp = data[0];
  for (unsigned int i = 0; i < data.size(); ++i)
  {
    if(tmp<data[i])
    {
       tmp=data[i];
    }
  }
  return tmp;
}

Real
Dummy_MCMC_I::computeSample(dof_id_type row_index, dof_id_type col_index)
{
  TIME_SECTION(_perf_compute_sample);
  dof_id_type offset = _values_distributed ? getLocalRowBegin() : 0;
  if (_step > 2)
  {
    if (isParamValid("vpp_limits") && _step > 2 && _step <= _num_samples_train)
    {
      if (col_index == (0))
      {

        Real density_new = 1.0;
        Real density_old = 1.0;

        for (dof_id_type i = 0; i < _distributions.size(); ++i)
        {
          _inputs_sto[i][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] = _inputs_ptr[i][row_index-offset];
          density_new = density_new * _distributions[i]->pdf(_inputs_sto[i][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)]);
          density_old = density_old * _distributions[i]->pdf(_inputs_sto[i][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)]);
        }
        _outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)] = _values_ptr[0][row_index-offset];

        bool indicator_vpp = ((std::abs(_outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)]) >= std::abs(_vpp_limits)) ? 1 : 0);

        _acceptance_ratio = std::log(density_new * indicator_vpp) - std::log(density_old);

        Real tmp11 = std::log(getRand());

        if (_acceptance_ratio < tmp11)
        {
          for (dof_id_type i = 0; i < _distributions.size(); ++i)
          {
            _inputs_sto[i][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] = _inputs_sto[i][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)];
          }
          _outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-2)] = _outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-3)];
        }

        if (_step > _num_samples_adaptive)
          _proposal_std[col_index] = Dummy_MCMC_I::computeSTD(_inputs_sto[col_index]);

        Real bound_i = 6 * _proposal_std[col_index] * std::pow(_num_samples_train, -0.2);
        // return Normal::quantile(getRand(_step), _inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)], _proposal_std[col_index]);
        return Uniform::quantile(getRand(_step), (_inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] - bound_i/2), (_inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] + bound_i/2));
      } else
      {
        if (_step > _num_samples_adaptive)
          _proposal_std[col_index] = Dummy_MCMC_I::computeSTD(_inputs_sto[col_index]);


        Real bound_i = 6 * _proposal_std[col_index] * std::pow(_num_samples_train, -0.2);

        return Uniform::quantile(getRand(_step), (_inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] - bound_i/2), (_inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] + bound_i/2));
        // return Normal::quantile(getRand(_step), _inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)], _proposal_std[col_index]);
      }
    } else if (isParamValid("vpp_limits") && _step > 2 && _step > _num_samples_train)
    {

      if (col_index == 0)
      {
        _rnd1 = getRand(_step);
        _index1 = std::round((0.02 + 0.98*_rnd1) * (_inputs_sto[col_index].size() - 1));
        Real tmp2 = 1.0;
        Real tmp3 = 1.0;
        for (unsigned int ii = 0; ii < (_distributions.size()); ++ii)
        {
          Real rand2 = Normal::quantile(getRand(_step), _inputs_sto[col_index][_index1], Dummy_MCMC_I::computeSTD(_inputs_sto[ii])); // Dummy_MCMC_I::computeSTD(_inputs_sto[ii])
          tmp2 = Normal::pdf(rand2, Dummy_MCMC_I::computeMEAN(_inputs_sto[ii]), Dummy_MCMC_I::computeSTD(_inputs_sto[ii])) / Normal::pdf(_inputs_ptr[ii][row_index-offset], Dummy_MCMC_I::computeMEAN(_inputs_sto[ii]), Dummy_MCMC_I::computeSTD(_inputs_sto[ii]));
          if (tmp2 > getRand(_step))
            _impMCMC[ii] = rand2;
          else
            _impMCMC[ii] = _inputs_ptr[ii][row_index-offset];

          tmp3 = tmp3 * _distributions[ii]->pdf(_impMCMC[ii]) / Normal::pdf(_impMCMC[ii], Dummy_MCMC_I::computeMEAN(_inputs_sto[ii]), Dummy_MCMC_I::computeSTD(_inputs_sto[ii]));
        }
        _impRatio += tmp3;
        std::cout << tmp3 << std::endl;
      }

      // return Normal::quantile(_rnd1, Dummy_MCMC_I::computeMEAN(_inputs_sto[col_index]), Dummy_MCMC_I::computeSTD(_inputs_sto[col_index])); //
      return _impMCMC[col_index];
    } else
    {
      _new_sample_sca = Normal::quantile(getRand(_step), _inputs_ptr[col_index][row_index-offset], _proposal_std[col_index]);
      _acceptance_ratio = _distributions[col_index]->pdf(_new_sample_sca) / _distributions[col_index]->pdf(_inputs_ptr[col_index][row_index-offset]);
      _inputs_sto[col_index][(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] = _inputs_ptr[col_index][row_index-offset] ? isParamValid("vpp_limits") : 1;
      _outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step-1)] = _values_ptr[0][row_index-offset] ? isParamValid("vpp_limits") : 1;
      if (_acceptance_ratio > getRand(_step))
      {
        return _new_sample_sca;
      } else
      {
        return _inputs_ptr[col_index][row_index-offset];
      }
    }
  } else
  {
    // std::cout << _values_ptr[0].size() << std::endl;
    if (_step > 1)
    {
      std::cout << "Error" << std::endl;
      std::cout << _inputs_ptr[col_index][row_index-offset] << std::endl;
      std::cout << _values_ptr[0].size() << std::endl;
      _outputs_sto[(row_index-offset) * getParam<dof_id_type>("num_rows") + (_step)] = _values_ptr[0][row_index-offset];
    }
    _inputs_sto[col_index][row_index * getParam<dof_id_type>("num_rows") + (_step)] = _seeds[col_index];
    return _seeds[col_index];
  }
}
