//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "GeneralReporter.h"

class AdaptiveMonteCarlo : public GeneralReporter
{
public:
  static InputParameters validParams();
  AdaptiveMonteCarlo(const InputParameters & parameters);
  // virtual void initialize() override {}
  virtual void initialize() override;
  virtual void finalize() override {}
  // virtual void execute() override {}
  virtual void execute() override;

protected:

  Real computeMIN(const std::vector<Real> & data);
  std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);
  std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

  /// This will add another type of reporter to the params
  template <typename T>
  static InputParameters addReporterTypeParams(const std::string & prefix, bool add_vector = true);

  ///@{
  /// Helper for declaring constant reporter values
  template <typename T>
  std::vector<T *> declareAdaptiveMonteCarloValues(const std::string & prefix);
  template <typename T>
  std::vector<std::vector<T> *> declareConstantVectorReporterValues(const std::string & prefix);
  ///@}

  /// Integer reporter data
  std::vector<int *> _int;
  /// Real reporter data
  std::vector<Real *> _real;
  // std::vector<Real *> _inputs;
  /// String reporter data
  std::vector<std::string *> _string;
  /// Vector of integers reporter data
  std::vector<std::vector<int> *> _int_vec;
  /// Vector of reals reporter data
  std::vector<std::vector<Real> *> _real_vec;
  /// Vector of strings reporter data
  std::vector<std::vector<std::string> *> _string_vec;

private:
  const int & _step;
  Sampler * _sampler;
  int _ind_sto;
  std::vector<Real> _markov_seed;
  unsigned int _count;
  unsigned int _count_max;
  std::vector<Real> _output_sorted;
  std::vector<Real> _outputs_sto;
  std::vector<std::vector<Real>> _inputs_sto;
  std::vector<std::vector<Real>> _inputs_sorted;
  unsigned int _subset;
  std::vector<Real> _output_limits;
};

template <typename T>
InputParameters
AdaptiveMonteCarlo::addReporterTypeParams(const std::string & prefix, bool add_vector)
{
  InputParameters params = emptyInputParameters();

  params.addParam<std::vector<ReporterValueName>>(prefix + "_names",
                                                  "Names for each " + prefix + " value.");
  params.addParam<std::vector<T>>(prefix + "_values", "Values for " + prefix + "s.");
  if (add_vector)
  {
    params.addParam<std::vector<ReporterValueName>>(
        prefix + "_vector_names", "Names for each vector of " + prefix + "s value.");
    params.addParam<std::vector<std::vector<T>>>(prefix + "_vector_values",
                                                 "Values for vectors of " + prefix + "s.");
  }

  return params;
}

template <typename T>
std::vector<T *>
AdaptiveMonteCarlo::declareAdaptiveMonteCarloValues(const std::string & prefix)
{
  std::string names_param(prefix + "_names");
  std::string values_param(prefix + "_values");
  std::vector<T *> data;

  if (isParamValid(names_param) && !isParamValid(values_param))
    paramError(names_param, "Must specify values using ", values_param);
  else if (!isParamValid(names_param) && isParamValid(values_param))
    paramError(values_param, "Use ", names_param, " to specify reporter names.");
  else if (!isParamValid(names_param) && !isParamValid(values_param))
    return data;

  auto & names = getParam<std::vector<ReporterValueName>>(names_param);
  auto & values = this->getParam<std::vector<T>>(values_param);
  if (names.size() != values.size())
    paramError(values_param,
               "Number of names specified in ",
               names_param,
               " must match number of values specified in ",
               values_param);

  for (unsigned int i = 0; i < names.size(); ++i)
    data.push_back(&this->declareValueByName<T>(names[i], values[i]));

  return data;
}

template <typename T>
std::vector<std::vector<T> *>
AdaptiveMonteCarlo::declareConstantVectorReporterValues(const std::string & prefix)
{
  return this->declareAdaptiveMonteCarloValues<std::vector<T>>(prefix + "_vector");
}
