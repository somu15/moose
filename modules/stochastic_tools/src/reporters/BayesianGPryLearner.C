//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "BayesianGPryLearner.h"
#include "Sampler.h"
#include "DenseMatrix.h"
#include "AdaptiveMonteCarloUtils.h"
#include "StochasticToolsUtils.h"
#include "Normal.h"

registerMooseObject("StochasticToolsApp", BayesianGPryLearner);

InputParameters
BayesianGPryLearner::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params += LikelihoodInterface::validParams();
  params.addClassDescription("Fast Bayesian inference with the GPry algorithm by El Gammal et al. "
                             "2023: NN and GP training step.");
  params.addRequiredParam<ReporterName>("output_value",
                                        "Value of the model output from the SubApp.");
  params.addRequiredParam<ReporterName>("output_value1",
                                        "Value of the model output1 from the SubApp.");
  params.addParam<ReporterValueName>(
      "output_comm", "output_comm", "Modified value of the model output from this reporter class.");
  params.addParam<ReporterValueName>(
      "output_comm1", "output_comm1", "Modified value of the model output from this reporter class.");
  params.addRequiredParam<SamplerName>("sampler", "The sampler object.");
  params.addRequiredParam<UserObjectName>("al_gp", "Active learning GP trainer.");
  params.addRequiredParam<UserObjectName>("gp_evaluator", "Evaluate the trained GP.");
  params.addParam<ReporterValueName>(
      "optimal_inputs",
      "optimal_inputs",
      "The sorted sample indices in order of importance to evaluate the subApp.");
  params.addParam<ReporterValueName>(
      "noise", "noise", "Model noise term to pass to Likelihoods object.");
  params.addParam<ReporterValueName>(
      "acquisition_function",
      "acquisition_function",
      "The values of the acquistion function in the current iteration.");
  params.addParam<ReporterValueName>(
      "convergence_value",
      "convergence_value",
      "Value to measure convergence of the GPry algorithm.");
  params.addRequiredParam<std::vector<UserObjectName>>("likelihoods", "Names of likelihoods.");
  return params;
}

BayesianGPryLearner::BayesianGPryLearner(const InputParameters & parameters)
  : GeneralReporter(parameters),
    LikelihoodInterface(parameters),
    // CovarianceInterface(parameters),
    SurrogateModelInterface(this),
    // _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _output_value(getReporterValue<std::vector<Real>>("output_value", REPORTER_MODE_DISTRIBUTED)),
    _output_comm(declareValue<std::vector<Real>>("output_comm")),
    _output_value1(getReporterValue<std::vector<Real>>("output_value1", REPORTER_MODE_DISTRIBUTED)), 
    _output_comm1(declareValue<std::vector<Real>>("output_comm1")),
    _sampler(getSampler("sampler")),
    _gpry_sampler(dynamic_cast<const BayesianGPrySampler *>(&_sampler)),
    _optimal_inputs(declareValue<std::vector<std::vector<Real>>>("optimal_inputs")),
    // _inputs_all(_gpry_sampler->getSampleTries()),
    // _var_all(_gpry_sampler->getVarSampleTries()),
    _al_gp(getUserObject<ActiveLearningGaussianProcess>("al_gp")),
    _gp_eval(getSurrogateModel<GaussianProcess>("gp_evaluator")),
    _new_var_samples(_gpry_sampler->getVarSamples()),
    _priors(_gpry_sampler->getPriors()),
    _var_prior(_gpry_sampler->getVarPrior()),
    _noise(declareValue<Real>("noise")),
    _acquisition_function(declareValue<std::vector<Real>>("acquisition_function")),
    _convergence_value(declareValue<Real>("convergence_value")),
    _check_step(std::numeric_limits<int>::max()),
    _local_comm(_sampler.getLocalComm())
{
  // Check whether the selected sampler is an adaptive sampler or not
  if (!_gpry_sampler)
    paramError("sampler", "The selected sampler is not of type BayesianGPrySampler.");
  
  // Filling the `likelihoods` vector with the user-provided distributions.
  for (const UserObjectName & name : getParam<std::vector<UserObjectName>>("likelihoods"))
    _likelihoods.push_back(getLikelihoodFunctionByName(name));
  
  // Fetching the sampler characteristics
  _num_samples = _sampler.getNumberOfRows();
  _props = _gpry_sampler->getNumParallelProposals();
  _num_confg_values = _gpry_sampler->getNumberOfConfigValues();
  _num_confg_params = _gpry_sampler->getNumberOfConfigParams();

  _optimal_inputs.resize(_props, std::vector<Real>(_priors.size() + 1, 0.0));
  _gp_outputs_try.resize(10000);
  _gp_std_try.resize(10000);
  _acquisition_function.resize(10000);
  _length_scales.resize(_priors.size());
  _proposed_input.resize(_priors.size());
  _proposed_var = 0.0;

  _eval_points = 10000;
  _eval_outputs_current.resize(_eval_points);
  _eval_outputs_previous.resize(_eval_points);
  _eval_inputs_density.resize(_eval_points, 1.0);
  if (_var_prior)
    _eval_inputs.resize(_eval_points, std::vector<Real>(_priors.size() + 1, 0.0));
  else
    _eval_inputs.resize(_eval_points, std::vector<Real>(_priors.size(), 0.0));
}

// void
// BayesianGPryLearner::setupCovariance(UserObjectName covar_name)
// {
//   if (_gp_handler.getCovarFunctionPtr() != nullptr)
//     ::mooseError("Attempting to redefine covariance function using setupCovariance.");
//   _gp_handler.linkCovarianceFunction(getCovarianceFunctionByName(covar_name));
// }

void
BayesianGPryLearner::setupNNGPData(const std::vector<Real> & log_posterior,
                                   const DenseMatrix<Real> & data_in)
{
  std::vector<Real> tmp;
  if (_var_prior)
    tmp.resize(_priors.size() + 1);
  else
    tmp.resize(_priors.size());
  for (unsigned int i = 0; i < log_posterior.size(); ++i)
  {
    for (unsigned int j = 0; j < _priors.size(); ++j)
      tmp[j] = data_in(i, j);
    if (_var_prior)
      tmp[_priors.size()] = _new_var_samples[i];
    if (std::exp(log_posterior[i]) > 0.0)
    {
      _gp_inputs.push_back(tmp);
      _gp_outputs.push_back(log_posterior[i]);
    }
  }
}

void
BayesianGPryLearner::computeLogPosterior(std::vector<Real> & log_posterior,
                                         const DenseMatrix<Real> & input_matrix)
{
  std::vector<Real> out1(_num_confg_values);
  std::vector<Real> out11(_num_confg_values);
  for (unsigned int i = 0; i < _props; ++i)
  {
    log_posterior[i] = 0.0;
    for (unsigned int j = 0; j < _priors.size(); ++j)
      log_posterior[i] += std::log(_priors[j]->pdf(input_matrix(i, j)));
    for (unsigned int j = 0; j < _num_confg_values; ++j)
    {
      out1[j] = _output_comm[j * _props + i];
      out11[j] = _output_comm1[j * _props + i];
    }
    if (_var_prior)
    {
      log_posterior[i] += std::log(_var_prior->pdf(_new_var_samples[i]));
      _noise = std::sqrt(_new_var_samples[i]);
      log_posterior[i] += std::log(_likelihoods[0]->function(out1));
      log_posterior[i] += std::log(_likelihoods[1]->function(out11));
    }
    else
    {
      log_posterior[i] += std::log(_likelihoods[0]->function(out1));
      log_posterior[i] += std::log(_likelihoods[1]->function(out11));
    }
  }
}

void
BayesianGPryLearner::acqWithCorrelations2(Real & acq,
                                         const unsigned int & current_index,
                                         const std::vector<Real> & input,
                                         const Real & var)
{
  std::vector<Real> input_regular;
  input_regular.resize(_priors.size());
  Real var_regular;
  fromStandardNormal(input_regular, input);
  fromStandardNormalVar(var_regular, var);
  Real gp_mean, gp_std;
  std::vector<Real> input_and_var = input_regular;
  if (_var_prior)
    input_and_var.push_back(var_regular);
  gp_mean = _gp_eval.evaluate(input_and_var, gp_std);
  
  acq = -std::exp(2.0 * std::pow(_priors.size(), -0.85) * gp_mean) * (std::exp(gp_std) - 1.0);
  Real correlation = 0.0;
  for (unsigned int i = 0; i < current_index; ++i)
  {
    computeCorrelation(input_and_var, _optimal_inputs[i], correlation);
    acq = acq * correlation;
  }

  // if (std::exp(gp_mean) > 0.0)
  // {
  //   acq = -std::exp(2.0 * std::pow(_priors.size(), -0.85) * gp_mean) * (std::exp(gp_std) - 1.0);
  //   Real correlation = 0.0;
  //   for (unsigned int i = 0; i < current_index; ++i)
  //   {
  //     computeCorrelation(input_and_var, _optimal_inputs[i], correlation);
  //     acq = acq * correlation;
  //   }
  // }
  // else
  //   acq = 0.0;
}

void
BayesianGPryLearner::randomIndex(const unsigned int & exclude,
                                 unsigned int & req_index)
{
  req_index = exclude;
  while (req_index == exclude)
    req_index = _sampler.getRandl(_seed, 0, _props);
}

void
BayesianGPryLearner::randomIndexTwo(const unsigned int & exclude1,
                                    const unsigned int & exclude2,
                                    unsigned int & req_index1,
                                    unsigned int & req_index2)
{
  req_index1 = exclude1;
  while (req_index1 == exclude1)
    randomIndex(exclude2, req_index1);
  req_index2 = req_index1;
  while (req_index2 == req_index1 || req_index2 == exclude1)
    randomIndex(exclude2, req_index2);
}

void
BayesianGPryLearner::optimizeAcquisition(const unsigned int & main_index,
                                         const bool & randomize,
                                         const std::vector<Real> & seed_input,
                                         const Real & seed_var,
                                         std::vector<Real> & optimized_input)
{
  unsigned int iters = 0;
  unsigned int index1, index2, index3;
  Real cross_over = 0.9;
  Real mutation_factor = 0.8;
  std::vector<Real> proposed_input;
  Real rand_per_iteration;
  unsigned int randind_per_iteration;
  std::vector<std::vector<Real>> base_inputs;
  base_inputs.resize(_props, std::vector<Real>(_priors.size(), 0.0));
  std::vector<Real> base_var;
  base_var.resize(_props);
  if (!randomize)
    for (unsigned int i = 0; i < _props; ++i)
    {
      fillVectorStandardNormal(base_inputs[i]);
      base_var[i] = Normal::quantile(_sampler.getRand(_seed), 0.0, 1.0);
    }
  else
    for (unsigned int i = 0; i < _props; ++i)
    {
      fillVectorSeedStandardNormal(seed_input, base_inputs[i]);
      base_var[i] = Normal::quantile(
          _sampler.getRand(_seed), Normal::quantile(_var_prior->cdf(seed_var), 0.0, 1.0), 0.01);
    }
  Real acq_current, acq_new;
  while (iters < 5000)
  {
    for (unsigned int i = 0; i < _props; ++i)
    {
      randomIndex(i, index1);
      randomIndexTwo(i, index1, index2, index3);
      for (unsigned int j = 0; j < _priors.size(); ++j)
      {
        rand_per_iteration = _sampler.getRand(_seed);
        randind_per_iteration = _sampler.getRandl(_seed, 0, _priors.size());
        _proposed_input[j] =
            (rand_per_iteration < cross_over || randind_per_iteration == j)
                ? (base_inputs[index1][j] +
                   mutation_factor * (base_inputs[index2][j] - base_inputs[index3][j]))
                : base_inputs[i][j];
      }
      if (_var_prior)
      {
        rand_per_iteration = _sampler.getRand(_seed);
        randind_per_iteration = _sampler.getRandl(_seed, 0, _priors.size());
        _proposed_var =
            (rand_per_iteration < cross_over || randind_per_iteration == _priors.size())
                ? (base_var[index1] + mutation_factor * (base_var[index2] - base_var[index3]))
                : base_var[i];
      }
      acqWithCorrelations2(acq_current, main_index, base_inputs[i], base_var[i]);
      acqWithCorrelations2(acq_new, main_index, _proposed_input, _proposed_var);
      base_inputs[i] = (acq_new <= acq_current) ? _proposed_input : base_inputs[i];
      if (_var_prior)
        base_var[i] = (acq_new <= acq_current) ? _proposed_var : base_var[i];
      iters += i;
    }
  }
  // std::cout << Moose::stringify(base_var) << std::endl;
  unsigned int optimal_index = _sampler.getRandl(_seed, 0, _props);
  acqWithCorrelations2(acq_current, main_index, base_inputs[optimal_index], base_var[optimal_index]);
  _acquisition_function[main_index] = -acq_current;

  std::vector<Real> input_regular;
  input_regular.resize(_priors.size());
  Real var_regular;
  fromStandardNormal(input_regular, base_inputs[optimal_index]);
  fromStandardNormalVar(var_regular, base_var[optimal_index]);
  optimized_input = input_regular;
  optimized_input.push_back(var_regular);
}

void
BayesianGPryLearner::acqWithCorrelations(std::vector<Real> & acq,
                                         std::vector<unsigned int> & sorted,
                                         const std::vector<std::vector<Real>> & tmp_inps_var)
{
  Real correlation = 0.0;
  std::vector<size_t> ind;
  Moose::indirectSort(acq.begin(), acq.end(), ind);
  sorted[0] = ind[0];
  _acquisition_function[0] = -acq[ind[0]];
  for (unsigned int i = 0; i < tmp_inps_var.size() - 1; ++i)
  {
    for (unsigned int j = 0; j < tmp_inps_var.size(); ++j)
    {
      computeCorrelation(tmp_inps_var[j], tmp_inps_var[ind[0]], correlation);
      acq[j] = acq[j] * correlation;
    }
    Moose::indirectSort(acq.begin(), acq.end(), ind);
    sorted[i + 1] = ind[0];
    _acquisition_function[i + 1] = -acq[ind[0]];
  }
}

void
BayesianGPryLearner::fillVector(std::vector<Real> & vector)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = _priors[i]->quantile(_sampler.getRand(_seed));
}

void
BayesianGPryLearner::fillVectorStandardNormal(std::vector<Real> & vector)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] =
        Normal::quantile(_sampler.getRand(_seed), 0.0, 1.0);
}

void
BayesianGPryLearner::fillVectorSeedStandardNormal(const std::vector<Real> & seed_input,
                                                  std::vector<Real> & vector)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = Normal::quantile(
        _sampler.getRand(_seed), Normal::quantile(_priors[i]->cdf(seed_input[i]), 0.0, 1.0), 0.1);
}

void
BayesianGPryLearner::fromStandardNormal(std::vector<Real> & vector,
                                        const std::vector<Real> & vector_standard)
{
  for (unsigned int i = 0; i < _priors.size(); ++i)
    vector[i] = _priors[i]->quantile(
        Normal::cdf(vector_standard[i], 0.0, 1.0));
}

void
BayesianGPryLearner::fromStandardNormalVar(Real & value,
                                        const Real & value_standard)
{
  value = _var_prior->quantile(Normal::cdf(value_standard, 0.0, 1.0));
}

void
BayesianGPryLearner::computeCorrelation(const std::vector<Real> & input1,
                                        const std::vector<Real> & input2,
                                        Real & corr)
{
  corr = 0.0;
  for (unsigned int i = 0; i < input1.size() - 1; ++i)
    corr -= Utility::pow<2>(input1[i] - input2[i]) / (2 * Utility::pow<2>(_length_scales[i]));
  if (_var_prior)
    corr -= Utility::pow<2>(input1[_priors.size()] - input2[_priors.size()]) /
            (2 * Utility::pow<2>(_length_scales[_priors.size()]));
  corr = 1.0 * std::exp(corr); // 1.0 - std::exp(corr);
}

void
BayesianGPryLearner::computeGPOutput(std::vector<Real> & eval_outputs,
                                     const std::vector<std::vector<Real>> & eval_inputs)
{
  for (unsigned int i = 0; i < eval_outputs.size(); ++i)
    eval_outputs[i] = _gp_eval.evaluate(eval_inputs[i]);
}

void
BayesianGPryLearner::execute()
{
  if (_sampler.getNumberOfLocalRows() == 0 || _check_step == _t_step)
  {
    _check_step = _t_step;
    return;
  }

  DenseMatrix<Real> data_in(_sampler.getNumberOfRows(), _sampler.getNumberOfCols());
  for (dof_id_type ss = _sampler.getLocalRowBegin(); ss < _sampler.getLocalRowEnd(); ++ss)
  {
    const auto data = _sampler.getNextLocalRow();
    for (unsigned int j = 0; j < _sampler.getNumberOfCols(); ++j)
      data_in(ss, j) = data[j];
  }
  _local_comm.sum(data_in.get_values());
  _output_comm = _output_value;
  _local_comm.allgather(_output_comm);
  _output_comm1 = _output_value1;
  _local_comm.allgather(_output_comm1);

  // Compute the log_posterior values
  std::vector<Real> log_posterior(_props);
  computeLogPosterior(log_posterior, data_in);
  if (_t_step > 1)
  {
    setupNNGPData(log_posterior, data_in);
    std::cout << "Training data size " << _gp_outputs.size() << std::endl;
    std::cout << Moose::stringify(_gp_outputs) << std::endl;
    _al_gp.reTrain(_gp_inputs, _gp_outputs);
    _al_gp.getLengthScales(_length_scales);

    std::vector<Real> tmp;
    std::vector<std::vector<Real>> tmp_inps_var;
    if (_var_prior)
    {
      tmp.resize(_priors.size() + 1);
      tmp_inps_var.resize(_gp_outputs_try.size(), std::vector<Real>(_priors.size() + 1, 0.0));
    }
    else
    {
      tmp.resize(_priors.size());
      tmp_inps_var.resize(_gp_outputs_try.size(), std::vector<Real>(_priors.size(), 0.0));
    }
    for (unsigned int i = 0; i < _gp_outputs_try.size(); ++i)
    {
      for (unsigned int j = 0; j < _priors.size(); ++j)
        tmp[j] = _priors[j]->quantile(_sampler.getRand(_seed));
      if (_var_prior)
        tmp[_priors.size()] = _var_prior->quantile(_sampler.getRand(_seed));
      tmp_inps_var[i] = tmp;
      _gp_outputs_try[i] = _gp_eval.evaluate(tmp, _gp_std_try[i]);
    }
    Real psi = std::pow(_priors.size(), -0.85);
    std::vector<Real> acq;
    acq.resize(_gp_outputs_try.size());
    Real gp_mean;
    Real gp_std;
    for (unsigned int i = 0; i < _gp_outputs_try.size(); ++i)
    {
      gp_mean = _gp_outputs_try[i];
      gp_std = _gp_std_try[i];
      acq[i] = -std::exp(2.0 * psi * gp_mean) * (std::exp(gp_std) - 1.0);
    }
    std::vector<unsigned int> tmp_indices;
    tmp_indices.resize(_gp_outputs_try.size());
    acqWithCorrelations(acq, tmp_indices, tmp_inps_var);
    std::cout << Moose::stringify(tmp_indices) << std::endl;
    for (unsigned int i = 0; i < _props; ++i)
      _optimal_inputs[i] = tmp_inps_var[tmp_indices[i]];

    // _seed = _t_step > 0 ? (_t_step - 1) : 0;
    // std::vector<Real> seed_input;
    // Real seed_var;
    // for (unsigned int i = 0; i < _props; ++i)
    // {
    //   // std::cout << "DE optimizing for parallel index " << i+1 << " out of " << _props << " indices." << std::endl;
    //   std::vector<Real> optimal_input_var;
    //   if (_t_step <= 1000)
    //     optimizeAcquisition(i, false, seed_input, seed_var, optimal_input_var);
    //   else
    //   {
    //     const unsigned int select_ind = _sampler.getRandl(_seed, 0, _gp_outputs.size());
    //     seed_input = _gp_inputs[select_ind];
    //     seed_var = _gp_inputs[select_ind][_priors.size()];
    //     optimizeAcquisition(i, true, seed_input, seed_var, optimal_input_var);
    //   }
    //   _optimal_inputs[i] = optimal_input_var;
    //   std::cout << Moose::stringify(_optimal_inputs[i]) << std::endl;
    // }
    
    computeGPOutput(_eval_outputs_current, _eval_inputs);
    
    if (_t_step > 2)
    {
      _convergence_value = 0.0;
      for (unsigned int ii = 0; ii < _eval_points; ++ii)
        _convergence_value += std::abs(_eval_outputs_previous[ii] - _eval_outputs_current[ii]) / _eval_points; // std::log(_eval_inputs_density[ii])
    }
    std::cout << "_convergence_value " << _convergence_value << std::endl;
    _eval_outputs_previous = _eval_outputs_current;
  }
  else
  {
    for (unsigned int i = 0; i < _props; ++i)
    {
      fillVector(_optimal_inputs[i]);
      if (_var_prior)
        _optimal_inputs[i][_priors.size()] = _var_prior->quantile(_sampler.getRand(_seed));
    }
    for (unsigned int i = 0; i < _eval_points; ++i)
    {
      fillVector(_eval_inputs[i]);
      if (_var_prior)
        _eval_inputs[i][_priors.size()] = _var_prior->quantile(_sampler.getRand(_seed));
    }
    for (unsigned int j = 0; j < _eval_points; ++j)
    {
      for (unsigned int i = 0; i < _priors.size(); ++i)
        _eval_inputs_density[j] = _eval_inputs_density[j] * _priors[i]->pdf(_eval_inputs[j][i]);
      if (_var_prior)
        _eval_inputs_density[j] =
            _eval_inputs_density[j] * _var_prior->pdf(_eval_inputs[j][_priors.size()]);
    }
  }

  // Track the current step
  _check_step = _t_step;
}
