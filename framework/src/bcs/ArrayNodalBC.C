//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "ArrayNodalBC.h"

#include "Assembly.h"
#include "MooseVariableFE.h"
#include "SystemBase.h"
#include "NonlinearSystemBase.h"

template <>
InputParameters
validParams<ArrayNodalBC>()
{
  return validParams<NodalBCBase>();
}

ArrayNodalBC::ArrayNodalBC(const InputParameters & parameters)
  : NodalBCBase(parameters),
    MooseVariableInterface<RealEigenVector>(this,
                                            true,
                                            "variable",
                                            Moose::VarKindType::VAR_NONLINEAR,
                                            Moose::VarFieldType::VAR_FIELD_ARRAY),
    _var(*mooseVariable()),
    _current_node(_var.node()),
    _u(_var.nodalValue())
{
  addMooseVariableDependency(mooseVariable());
}

void
ArrayNodalBC::computeResidual()
{
  if (_var.isNodalDefined())
  {
    RealEigenVector res = computeQpResidual();

    for (auto tag_id : _vector_tags)
      if (_sys.hasVector(tag_id))
        _var.insertNodalValue(_sys.getVector(tag_id), res);
  }
}

void
ArrayNodalBC::computeJacobian()
{
  if (_var.isNodalDefined())
  {
    RealEigenVector cached_val = computeQpJacobian();

    dof_id_type cached_row = _var.nodalDofIndex();

    // Cache the user's computeQpJacobian() value for later use.
    for (auto tag : _matrix_tags)
      if (_sys.hasMatrix(tag))
        for (unsigned int i = 0; i < _var.count(); ++i)
          _fe_problem.assembly(0).cacheJacobianContribution(
              cached_row + i, cached_row + i, cached_val(i), tag);
  }
}

void
ArrayNodalBC::computeOffDiagJacobian(unsigned int jvar)
{
  MooseVariableFEBase & jv = _sys.getVariable(0, jvar);

  RealEigenMatrix cached_val = computeQpOffDiagJacobian(jv);

  dof_id_type cached_row = _var.nodalDofIndex();
  // Note: this only works for Lagrange variables...
  dof_id_type cached_col = _current_node->dof_number(_sys.number(), jvar, 0);

  // Cache the user's computeQpJacobian() value for later use.
  for (auto tag : _matrix_tags)
    if (_sys.hasMatrix(tag))
      for (unsigned int i = 0; i < _var.count(); ++i)
        for (unsigned int j = 0; j < jv.count(); ++j)
          _fe_problem.assembly(0).cacheJacobianContribution(
              cached_row + i, cached_col + j, cached_val(i, j), tag);
}

RealEigenVector
ArrayNodalBC::computeQpJacobian()
{
  return RealEigenVector::Ones(_var.count());
  ;
}

RealEigenMatrix
ArrayNodalBC::computeQpOffDiagJacobian(MooseVariableFEBase & jvar)
{
  if (jvar.number() == _var.number())
    return RealEigenMatrix::Identity(_var.count(), jvar.count());
  else
    return RealEigenMatrix::Zero(_var.count(), jvar.count());
}
