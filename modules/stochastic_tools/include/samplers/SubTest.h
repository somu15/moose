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
 * A class used to perform Subset Simulation Sampling
 */
class SubTest : public Sampler, public ReporterInterface
{
public:
  static InputParameters validParams();

  SubTest(const InputParameters & parameters);

  std::vector<Real> _outputs_sto;

  std::vector<std::vector<Real>> _inputs_sto;

protected:
  /// Return the sample for the given row and column
  virtual Real computeSample(dof_id_type row_index, dof_id_type col_index) override;

  // Real computeSTD(const std::vector<Real> & data);
  //
  // Real computeMEAN(const std::vector<Real> & data);

  Real computeMIN(const std::vector<Real> & data);
  std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);
  std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

  /// Storage for distribution objects to be utilized
  std::vector<Distribution const *> _distributions;

  /// Distribution names
  const std::vector<DistributionName> & _distribution_names;

  const int & _num_samplessub;

  const Real & _subset_probability;

  const std::vector<Real> & _proposal_std;

  // std::vector<std::vector<Real>> _inputs_sto;
  //
  // std::vector<Real> _outputs_sto;

private:

  Real _new_sample_sca;

  std::vector<Real> _new_sample_vec;

  bool _consecutive_indicator;

  Real _acceptance_ratio;

  // bool _values_distributed;

  // bool _values_distributed1;

  std::vector<const VectorPostprocessorValue *> _values_ptr;

  const int & _step;

  unsigned int _index1;

  unsigned int _subset;

  int _ind_sto;
  std::vector<Real> _markov_seed;
  unsigned int _count;
  int _check_even;
  unsigned int _count_max;
  std::vector<Real> _output_sorted;
  std::vector<std::vector<Real>> _inputs_sorted;

  /// Storage of the previous sample to propose the next sample
  std::vector<std::vector<Real>> _prev_val;

  std::vector<Real> _output_limits;

  Real _rnd1;

  /// PerfGraph timer
  const PerfID _perf_compute_sample;
};
