//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubsetSimReporter.h"

registerMooseObject("MooseApp", SubsetSimReporter);

InputParameters
SubsetSimReporter::validParams()
{
  InputParameters params = GeneralReporter::validParams();
  params.addClassDescription("Reporter with constant values to be accessed by other objects, can "
                             "be modified using transfers.");

  params += addReporterTypeParams<int>("integer");
  params += addReporterTypeParams<Real>("real");
  params += addReporterTypeParams<std::string>("string");

  return params;
}

SubsetSimReporter::SubsetSimReporter(const InputParameters & parameters)
  : GeneralReporter(parameters),
    _int(declareSubsetSimReporterValues<int>("integer")),
    _real(declareSubsetSimReporterValues<Real>("real")),
    _string(declareSubsetSimReporterValues<std::string>("string")),
    _int_vec(declareConstantVectorReporterValues<int>("integer")),
    _real_vec(declareConstantVectorReporterValues<Real>("real")),
    _string_vec(declareConstantVectorReporterValues<std::string>("string")),
    _step(getCheckedPointerParam<FEProblemBase *>("_fe_problem_base")->timeStep())
{
}

void
SubsetSimReporter::execute()
{
  std::cout << "Here" << std::endl;
  std::cout << (*_real[0]) << std::endl;

  (*_real[0]) = 1.0;
  // Real x = 1.0;
  // _real[0] = &x;
}
