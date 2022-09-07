//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ActiveLearningReporterBase.h"
#include "ActiveLearningGaussianProcess.h"
#include "GaussianProcess.h"
#include "SurrogateModelInterface.h"

class ActiveLearningGPDecision : public ActiveLearningReporterTempl<Real>, public SurrogateModelInterface
{
public:
  static InputParameters validParams();
  ActiveLearningGPDecision(const InputParameters & parameters);

protected:

  /**
   * This evaluates the inputted function to determine whether a multiapp solve is
   * necessary/allowed, otherwise it replaces the "transferred" quantity with a
   * default value.
   */
  virtual bool needSample(const std::vector<Real> & row,
                          dof_id_type local_ind,
                          dof_id_type global_ind,
                          Real & val) override;

private:
  
  /// Track the current step of the main App
  const int & _step;

  /// The active learning GP trainer that permits re-training
  const ActiveLearningGaussianProcess * const _al_gp;

  // /// The GP evaluator
  // const GaussianProcess * const _gp_evaluator;

  /// The adaptive Monte Carlo sampler
  Sampler & _sampler;

  std::vector<bool> & _flag_sample;

  const int _n_train;

  std::vector<std::vector<Real>> & _inputs;

  std::vector<Real> & _gp_mean;

  std::vector<Real> & _gp_std;

  std::vector<std::vector<Real>> _inputs_sto;

  std::vector<Real> _outputs_sto;

  std::vector<std::vector<Real>> _inputs_prev;

  std::vector<bool> _decision;

  std::vector<Real> _len_sto;

  Real _amp_sto;

  unsigned int _track_gp_fails;

  unsigned int _allowed_gp_fails;

  libMesh::Parallel::Communicator _local_comm;
};
