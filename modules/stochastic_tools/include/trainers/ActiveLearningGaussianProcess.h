//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningGaussianProcess.h"
#include "Standardizer.h"
#include <Eigen/Dense>

#include "StochasticToolsApp.h"
#include "LoadSurrogateDataAction.h"

#include "SurrogateModelInterface.h"
#include "SurrogateTrainer.h"
#include "MooseRandom.h"

#include "Distribution.h"

#include "CovarianceFunctionBase.h"
#include "CovarianceInterface.h"

#include "GaussianProcessHandler.h"

class ActiveLearningGaussianProcess : public SurrogateTrainerBase,
                                      public CovarianceInterface,
                                      public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  ActiveLearningGaussianProcess(const InputParameters & parameters);

  virtual void initialize() final{};
  virtual void execute() final{};
  virtual void reTrain(const std::vector<std::vector<Real>> & inputs,
                       const std::vector<Real> & outputs) const final;

  virtual void getLengthScales(std::vector<Real> & length_scales) const final;

  StochasticTools::GaussianProcessHandler & gpHandler() { return _gp_handler; }
  const StochasticTools::GaussianProcessHandler & getGPHandler() const { return _gp_handler; }

private:

  /// Name for the meta data associated with training
  const std::string _model_meta_data_name;

  /// The GP handler
  StochasticTools::GaussianProcessHandler & _gp_handler;

  /// Paramaters (x) used for training, along with statistics
  RealEigenMatrix & _training_params;

  /// Switch for training param (x) standardization
  bool _standardize_params;

  /// Switch for training data(y) standardization
  bool _standardize_data;

  /// Dynamically change the Adam optimization settings based on the training data size
  const std::vector<unsigned int> & _learning_scheduler_size;
  /// Dynamically change the Adam optimization initialization
  const std::vector<bool> & _learning_scheduler_initialize;
  /// Dynamically change the Adam optimization iterations
  const std::vector<unsigned int> & _learning_scheduler_iterations;
  /// Dynamically change the Adam optimization batch size
  const std::vector<unsigned int> & _learning_scheduler_batch_size;
  /// Dynamically change the Adam optimization learning rate
  const std::vector<Real> & _learning_scheduler_learning_rate;

  /// Struct holding parameters necessary for parameter tuning
  // StochasticTools::GaussianProcessHandler::GPOptimizerOptions & _optimization_opts;

  // Keep track of the scheduler tuning options for Adam
  unsigned int _scheduler_count;
};
