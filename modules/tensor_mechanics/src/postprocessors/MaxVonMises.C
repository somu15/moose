//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

// #include "MaxVonMises.h"
//
// // MOOSE includes
// #include "AuxiliarySystem.h"
// #include "MooseVariable.h"
//
// registerMooseObject("TensorMechanicsApp", MaxVonMises);
//
// InputParameters
// MaxVonMises::validParams()
// {
//   InputParameters params = NodalPostprocessor::validParams();
//   params.addClassDescription("MaxVonMises calculates the torque in 2D and 3D"
//                              "about a user-specified axis of rotation centered"
//                              "at a user-specied origin.");
//   params.addRequiredParam<std::vector<AuxVariableName>>("variable_name",
//                                                          "Von Mises variable.");
//   params.set<bool>("use_displaced_mesh") = true;
//   return params;
// }
//
// MaxVonMises::MaxVonMises(const InputParameters & parameters)
//   : NodalPostprocessor(parameters),
//     _aux(_fe_problem.getAuxiliarySystem()),
//     _max_value(0.0)
// {
//   std::vector<AuxVariableName> reacts =
//       getParam<std::vector<AuxVariableName>>("variable_name");
//   _react = &_aux.getFieldVariable<Real>(_tid, reacts[0]).dofValues();
// }
//
// void
// MaxVonMises::initialize()
// {
// }
//
// void
// MaxVonMises::execute()
// {
//   // for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
//   //   _max_value = std::max(_max_value, (*_react)[_qp]);
//   _max_value = std::max(_max_value, (*_react)[_qp]);
// }
//
// Real
// MaxVonMises::getValue()
// {
//   gatherSum(_max_value);
//   return _max_value;
// }
//
// void
// MaxVonMises::threadJoin(const UserObject & y)
// {
//   const MaxVonMises & pps = static_cast<const MaxVonMises &>(y);
//   _max_value += pps._max_value;
// }


#include "MaxVonMises.h"

// MOOSE includes
#include "AuxiliarySystem.h"
#include "MooseVariable.h"

registerMooseObject("TensorMechanicsApp", MaxVonMises);

InputParameters
MaxVonMises::validParams()
{
  InputParameters params = ElementPostprocessor::validParams();
  params.addClassDescription("MaxVonMises.");
  params.addRequiredParam<std::vector<AuxVariableName>>("variable_name",
                                                         "Von Mises variable.");
  params.set<bool>("use_displaced_mesh") = true;
  return params;
}

MaxVonMises::MaxVonMises(const InputParameters & parameters)
  : ElementPostprocessor(parameters),
    _aux(_fe_problem.getAuxiliarySystem()),
    _max_value(0.0)
{
  std::vector<AuxVariableName> reacts =
      getParam<std::vector<AuxVariableName>>("variable_name");
  _react = &_aux.getFieldVariable<Real>(_tid, reacts[0]).dofValues();
}

void
MaxVonMises::initialize()
{
}

void
MaxVonMises::execute()
{

  for (unsigned int _qp = 0; _qp < _qrule->n_points(); _qp++)
    _max_value = std::max(_max_value, (*_react)[_qp]);
}

Real
MaxVonMises::getValue()
{
  // gatherSum(_max_value);
  return _max_value;
}

void
MaxVonMises::threadJoin(const UserObject & y)
{
  const MaxVonMises & pps = static_cast<const MaxVonMises &>(y);
  _max_value += pps._max_value;
}
