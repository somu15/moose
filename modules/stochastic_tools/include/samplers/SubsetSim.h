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
class SubsetSim : public Sampler, public VectorPostprocessorInterface // public Sampler, public VectorPostprocessorInterface
{
public:
  static InputParameters validParams();

  SubsetSim(const InputParameters & parameters);
  // virtual void initialSetup() override;
  virtual void sampleSetUp() override;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  Real computeMIN(const std::vector<Real> & data);
  std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);
  std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  /// VPP names
  const std::vector<std::string> & _vpp_names;

  /// Input names
  const std::vector<std::string> & _inputs_names;

  const unsigned int & _num_samplessub;

  const Real & _subset_probability;

  const std::vector<Real> & _proposal_std;

private:

  Real _new_sample_sca;

  std::vector<Real> _new_sample_vec;

  bool _consecutive_indicator;

  Real _acceptance_ratio;

  bool _values_distributed;

  bool _values_distributed1;

  std::vector<VectorPostprocessorValue> _values_ptr;

  std::vector<VectorPostprocessorValue> _inputs_ptr;

  const int & _step;

  std::vector<std::vector<Real>> _inputs_sto;

  std::vector<Real> _outputs_sto;

  unsigned int _index1;

  unsigned int _subset;

  // unsigned int _ind_max;
  int _ind_sto;
  std::vector<Real> _markov_seed;
  unsigned int _count;
  int _check_even;
  unsigned int _count_max;
  std::vector<Real> _output_sorted;
  std::vector<std::vector<Real>> _inputs_sorted;
  std::vector<Real> _output_limits;

  Real _rnd1;

  /// PerfGraph timer
  const PerfID _perf_compute_sample;
};
