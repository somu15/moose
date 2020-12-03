//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Sampler.h"
#include "VectorPostprocessorInterface.h"

/**
 * A class used to perform Markov Cahin Monte Carlo Sampling using the Metropolis algorithm
 */
class Metropolis : public Sampler, public VectorPostprocessorInterface
{
public:
  static InputParameters validParams();

  Metropolis(const InputParameters & parameters);

  virtual void sampleSetUp() override;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Input names
  const std::vector<std::string> & _inputs_names;

  /// Standard deviations of the proposal distributions
  const std::vector<Real> & _proposal_std;

  /// Seed input values to get the MCMC sampler started
  const std::vector<Real> & _initial_values;

private:
  /// Check if the values are distributed
  bool _values_distributed;

  /// Storage to keep track of the sample index
  const int & _step;

  /// Storage for vector VectorPostprocessorValue
  std::vector<const VectorPostprocessorValue *> _inputs_ptr;

  const PerfID _perf_compute_sample;
};
