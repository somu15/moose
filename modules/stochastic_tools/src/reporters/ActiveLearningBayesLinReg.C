//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ActiveLearningBayesLinReg.h"

registerMooseObject("StochasticToolsApp", ActiveLearningBayesLinReg);

template <typename T>
InputParameters
ActiveLearningBayesLinRegTempl<T>::validParams()
{
  InputParameters params = ActiveLearningReporterTempl<T>::validParams();
  // params += FunctionParserUtils<false>::validParams();
  params.addClassDescription("Evaluates parsed function to determine if sample needs to be "
                             "evaluated, otherwise data is set to a default value.");
  params.addRequiredParam<ReporterName>("output_value", "Value of the model output from the SubApp.");
  return params;
}

template <typename T>
ActiveLearningBayesLinRegTempl<T>::ActiveLearningBayesLinRegTempl(
    const InputParameters & parameters)
  : ActiveLearningReporterTempl<T>(parameters),
  // GeneralReporter(this),
  _output_value(getReporterValue<std::vector<Real>>("output_value"))
{
}

template <typename T>
bool
ActiveLearningBayesLinRegTempl<T>::needSample(const std::vector<Real> & row,
                                              dof_id_type,
                                              dof_id_type,
                                              T & val)
{
  // std::cout << Moose::stringify(row) << std::endl;
  // val = 0.0;
  _test.resize(3,3);
  _test(0,0) = 1.0;
  _test(0,1) = 0.0;
  _test(0,2) = 0.0;
  _test(1,0) = 0.0;
  _test(1,1) = 1.0;
  _test(1,2) = 0.0;
  _test(2,0) = 0.0;
  _test(2,1) = 0.0;
  _test(2,2) = 1.0;

  _test_inv = _test.inverse();
  std::cout << Moose::stringify(_test_inv) << std::endl;
  return true;
}

// Explicit instantiation (more types can easily be added)
template class ActiveLearningBayesLinRegTempl<Real>;
