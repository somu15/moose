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
// #include "VectorPostprocessorData.h"
#include "VectorPostprocessorInterface.h"
#include "GeneralVectorPostprocessor.h"

/**
 * A class used to perform Monte Carlo Sampling
 */
class Dummy : public Sampler, public VectorPostprocessorInterface // public Sampler, public VectorPostprocessorInterface
{
public:
  static InputParameters validParams();

  Dummy(const InputParameters & parameters);
  // virtual void initialSetup() override;
  virtual void sampleSetUp() override;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  /// VPP names
  const std::vector<std::string> & _vpp_names;

  /// Input names
  const std::vector<std::string> & _inputs_names;

  // Sampler * _sampler;

  /// Standard deviations of the proposal distributions
  std::vector<Real> _proposal_std;

  /// Limit values of the VPPs
  const std::vector<Real> & _vpp_limits;

  /// Seed input values to get the MCMC sampler started
  const std::vector<Real> & _seeds;

  /// Number of samples after which adaptive MCMC is activated
  const Real & _num_samples_adaptive;

private:

  std::vector<Real> _sum_inputs;

  std::vector<Real> _diff_inputs;

  std::vector<Real> _store_sample;

  Real _new_sample_sca;

  std::vector<Real> _new_sample_vec;

  bool _consecutive_indicator;

  Real _acceptance_ratio;

  bool _values_distributed;

  bool _values_distributed1;

  std::vector<VectorPostprocessorValue> _values_ptr;

  std::vector<VectorPostprocessorValue> _inputs_ptr;

  // const VectorPostprocessorValue * _values_ptr = nullptr;

  const int & _step;

  std::vector<VectorPostprocessorValue *> _inputs_sto;

  std::vector<VectorPostprocessorValue *> _outputs_sto;

  // Real tmp1 = 0.0;
  //
  // Real tmp2 = 0.0;
  //
  // Real density_new = 1.0;
  //
  // Real density_old = 1.0;
  //
  // bool indicator_vpp = 1;

  /// PerfGraph timer
  const PerfID _perf_compute_sample;
};
