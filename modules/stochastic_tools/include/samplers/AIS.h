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
#include "ReporterInterface.h"

/**
 * A class used to perform Adaptive Importance Sampling
 */
class AIS : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  AIS(const InputParameters & parameters);

  std::vector<std::vector<Real>> _inputs_sto;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  Real computeSTD(const std::vector<Real> & data);

  Real computeMEAN(const std::vector<Real> & data);

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  const std::vector<Real> & _proposal_std;

  const std::vector<Real> & _seeds;

  const Real & _output_limit;

  const int & _num_samples_train;

  const Real & _std_factor;

private:

  // bool _values_distributed;

  Real _acceptance_ratio;

  const int & _step;

  int _check_even;

  std::vector<std::vector<Real>> _prev_val;

  Real _sum_R;

  /// PerfGraph timer
  const PerfID _perf_compute_sample;
};
