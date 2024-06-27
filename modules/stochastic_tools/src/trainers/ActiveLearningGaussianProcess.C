//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveLearningGaussianProcess.h"

#include <petsctao.h>
#include <petscdmda.h>

#include "libmesh/petsc_vector.h"
#include "libmesh/petsc_matrix.h"

#include <math.h>

registerMooseObject("StochasticToolsApp", ActiveLearningGaussianProcess);

InputParameters
ActiveLearningGaussianProcess::validParams()
{
  InputParameters params = SurrogateTrainerBase::validParams();
  params.addClassDescription(
      "Permit re-training Gaussian Process surrogate model for active learning.");
  params.addRequiredParam<UserObjectName>("covariance_function", "Name of covariance function.");
  params.addParam<bool>(
      "standardize_params", true, "Standardize (center and scale) training parameters (x values)");
  params.addParam<bool>(
      "standardize_data", true, "Standardize (center and scale) training data (y values)");
  MooseEnum tuning_type("tao adam adamW none", "none");
  params.addRequiredParam<MooseEnum>(
      "tuning_algorithm", tuning_type, "Hyper parameter optimizaton algorithm");
  // params.addParam<unsigned int>("iter_adam", 1000, "Tolerance value for Adam optimization");
  // params.addParam<unsigned int>("batch_size", 0, "The batch size for Adam optimization");
  // params.addParam<Real>("learning_rate_adam", 0.001, "The learning rate for Adam optimization");
  params.addParam<std::string>(
      "tao_options", "", "Command line options for PETSc/TAO hyperparameter optimization");
  params.addParam<bool>(
      "show_optimization_details", false, "Switch to show TAO or Adam solver results");
  params.addParam<unsigned int>("show_loss_every",
                                std::numeric_limits<unsigned int>::max(),
                                "Show loss value every nth iteration for Adam or AdamW");
  params.addParam<std::vector<std::string>>(
      "tune_parameters", {}, "Select hyperparameters to be tuned");
  params.addParam<std::vector<Real>>(
      "tuning_min", std::vector<Real>(), "Minimum allowable tuning value");
  params.addParam<std::vector<Real>>(
      "tuning_max", std::vector<Real>(), "Maximum allowable tuning value");
  params.addParam<std::vector<unsigned int>>(
      "learning_scheduler_size",
      std::vector<unsigned int>({std::numeric_limits<unsigned int>::max()}),
      "Dynamically change the Adam optimization settings based on the training data size. Enter "
      "the training data sizes and if these sizes are exceeded the corresponding optimization "
      "options will be applied.");
  params.addParam<std::vector<bool>>(
      "learning_scheduler_initialize",
      std::vector<bool>({true}),
      "Dynamically change the Adam optimization initialization. True: Initialize at the provided "
      "starting values. False: Initialize at the previous iteration of active learning.");
  params.addParam<std::vector<unsigned int>>("learning_scheduler_iterations",
                                             std::vector<unsigned int>({1000}),
                                             "Dynamically change the Adam optimization iterations");
  params.addParam<std::vector<unsigned int>>("learning_scheduler_batch_size",
                                             std::vector<unsigned int>({0}),
                                             "Dynamically change the Adam optimization batch size");
  params.addParam<std::vector<Real>>("learning_scheduler_learning_rate",
                                     std::vector<Real>({0.001}),
                                     "Dynamically change the Adam optimization learning rate");
  return params;
}

ActiveLearningGaussianProcess::ActiveLearningGaussianProcess(const InputParameters & parameters)
  : SurrogateTrainerBase(parameters),
    CovarianceInterface(parameters),
    SurrogateModelInterface(this),
    _gp_handler(declareModelData<StochasticTools::GaussianProcessHandler>("_gp_handler")),
    _training_params(declareModelData<RealEigenMatrix>("_training_params")),
    _standardize_params(getParam<bool>("standardize_params")),
    _standardize_data(getParam<bool>("standardize_data")),
    _learning_scheduler_size(getParam<std::vector<unsigned int>>("learning_scheduler_size")),
    _learning_scheduler_initialize(getParam<std::vector<bool>>("learning_scheduler_initialize")),
    _learning_scheduler_iterations(
        getParam<std::vector<unsigned int>>("learning_scheduler_iterations")),
    _learning_scheduler_batch_size(
        getParam<std::vector<unsigned int>>("learning_scheduler_batch_size")),
    _learning_scheduler_learning_rate(
        getParam<std::vector<Real>>("learning_scheduler_learning_rate"))
{
  if (getParam<unsigned int>("batch_size") > 0 && _optimization_opts.opt_type == "tao")
    paramError("batch_size",
               "Mini-batch sampling is not compatible with the TAO optimization library. Please "
               "use Adam/AdamW optimization.");

  _gp_handler.initialize(
      getCovarianceFunctionByName(getParam<UserObjectName>("covariance_function")),
      getParam<std::vector<std::string>>("tune_parameters"),
      getParam<std::vector<Real>>("tuning_min"),
      getParam<std::vector<Real>>("tuning_max"));
  
  if (!(_learning_scheduler_size.size() == _learning_scheduler_initialize.size() ==
        _learning_scheduler_iterations.size() == _learning_scheduler_batch_size.size() ==
        _learning_scheduler_learning_rate.size()))
    mooseError("The size of the vectors for learning_scheduler options should all be the same.");

  if (!(_learning_scheduler_size[0] == 0))
    paramError("learning_scheduler_size", "The first value of learning_scheduler_size should be 0.");

  if (!std::is_sorted(_learning_scheduler_size.begin(), _learning_scheduler_size.end()))
    paramError("learning_scheduler_size",
               "The training data sizes provided should be strictly in an ascending order.");

  _scheduler_count = 0;
}

void
ActiveLearningGaussianProcess::reTrain(const std::vector<std::vector<Real>> & inputs,
                                       const std::vector<Real> & outputs) const
{
  // Addtional error check for each re-train call of the GP surrogate
  if (inputs.size() != outputs.size())
    mooseError("Number of inputs (",
               inputs.size(),
               ") does not match number of outputs (",
               outputs.size(),
               ").");
  if (inputs.empty())
    mooseError("There is no data for retraining.");

  RealEigenMatrix training_data;
  _training_params.setZero(outputs.size(), inputs[0].size());
  training_data.setZero(outputs.size(), 1);

  for (unsigned int i = 0; i < outputs.size(); ++i)
  {
    training_data(i, 0) = outputs[i];
    for (unsigned int j = 0; j < inputs[i].size(); ++j)
      _training_params(i, j) = inputs[i][j];
  }

  // Standardize (center and scale) training params
  if (_standardize_params)
    _gp_handler.standardizeParameters(_training_params);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.paramStandardizer().set(0, 1, inputs[0].size());

  // Standardize (center and scale) training data
  if (_standardize_data)
    _gp_handler.standardizeData(training_data);
  // if not standardizing data set mean=0, std=1 for use in surrogate
  else
    _gp_handler.dataStandardizer().set(0, 1, inputs[0].size());

  // Update the scheduler to set the right options for tuning Adam, if necessary
  if (outputs.size() >= _learning_scheduler_size[_scheduler_count] &&
      _scheduler_count < _learning_scheduler_iterations.size())
  {
    StochasticTools::GaussianProcessHandler::GPOptimizerOptions & _optimization_opts;
    _optimization_opts = StochasticTools::GaussianProcessHandler::GPOptimizerOptions(
        getParam<MooseEnum>("tuning_algorithm"),
        getParam<std::string>("tao_options"),
        getParam<bool>("show_optimization_details"),
        getParam<bool>("show_optimization_details") ? getParam<unsigned int>("show_loss_every")
                                                    : std::numeric_limits<unsigned int>::max(),
        _learning_scheduler_iterations[_scheduler_count],
        _learning_scheduler_batch_size[_scheduler_count],
        _learning_scheduler_learning_rate[_scheduler_count],
        _learning_scheduler_initialize[_scheduler_count]);
    ++_scheduler_count;
  }

  // Setup the covariance
  _gp_handler.setupCovarianceMatrix(_training_params, training_data, _optimization_opts);
}

void
ActiveLearningGaussianProcess::getLengthScales(std::vector<Real> & length_scales) const
{
  length_scales = _gp_handler.getScales();
}
