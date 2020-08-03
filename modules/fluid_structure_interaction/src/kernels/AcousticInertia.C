//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "AcousticInertia.h"
#include "SubProblem.h"
#include "TimeIntegrator.h"
#include "NonlinearSystemBase.h"

registerMooseObject("MooseApp", AcousticInertia);

InputParameters
AcousticInertia::validParams()
{
  InputParameters params = TimeKernel::validParams();
  params.addClassDescription("Calculates the residual for the inertial force "
                             "($M \\cdot acceleration$) and the contribution of mass"
                             " dependent Rayleigh damping and HHT time "
                             " integration scheme ($\\eta \\cdot M \\cdot"
                             " ((1+\\alpha)velq2-\\alpha \\cdot vel-old) $)");
  params.set<bool>("use_displaced_mesh") = true;
  // params.addParam<MaterialPropertyName>("eta",
  //                                       0.0,
  //                                       "Name of material property or a constant real "
  //                                       "number defining the eta parameter for the "
  //                                       "Rayleigh damping.");
  // params.addParam<Real>("alpha",
  //                       0,
  //                       "alpha parameter for mass dependent numerical damping induced "
  //                       "by HHT time integration scheme");
  // params.addRequiredParam<Real>("invcosq",
  //                       "alpha parameter for mass dependent numerical damping induced "
  //                       "by HHT time integration scheme");
  params.addParam<MaterialPropertyName>(
      "density", "density", "Name of Material Property that provides the density");
  return params;
}

AcousticInertia::AcousticInertia(const InputParameters & parameters)
  : TimeKernel(parameters),
    _density(getMaterialProperty<Real>("density")),
    // _eta(getMaterialProperty<Real>("eta")),
    // _alpha(getParam<Real>("alpha")),
    // _invcosq(getParam<Real>("invcosq")),
    _time_integrator(*_sys.getTimeIntegrator())
{
    _u_dot_old = &(_var.uDotOld());
    _du_dot_du = &(_var.duDotDu());
    _du_dotdot_du = &(_var.duDotDotDu());

    addFEVariableCoupleableVectorTag(_time_integrator.uDotFactorTag());
    addFEVariableCoupleableVectorTag(_time_integrator.uDotDotFactorTag());

    _u_dot_factor = &_var.vectorTagValue(_time_integrator.uDotFactorTag());
    _u_dotdot_factor = &_var.vectorTagValue(_time_integrator.uDotDotFactorTag());

}

Real
AcousticInertia::computeQpResidual()
{
  if (_dt == 0)
    return 0;
  else
    return _test[_i][_qp] * _density[_qp] *
           ((*_u_dotdot_factor)[_qp] + (*_u_dot_factor)[_qp] -
            (*_u_dot_old)[_qp]);
}

Real
AcousticInertia::computeQpJacobian()
{
  if (_dt == 0)
    return 0;
  else
    return _test[_i][_qp] * _density[_qp] * (*_du_dotdot_du)[_qp] * _phi[_j][_qp] +
           _test[_i][_qp] * _density[_qp] * (*_du_dot_du)[_qp] *
               _phi[_j][_qp];
}
