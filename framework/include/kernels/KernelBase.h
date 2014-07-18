/****************************************************************/
/*               DO NOT MODIFY THIS HEADER                      */
/* MOOSE - Multiphysics Object Oriented Simulation Environment  */
/*                                                              */
/*           (c) 2010 Battelle Energy Alliance, LLC             */
/*                   ALL RIGHTS RESERVED                        */
/*                                                              */
/*          Prepared by Battelle Energy Alliance, LLC           */
/*            Under Contract No. DE-AC07-05ID14517              */
/*            With the U. S. Department of Energy               */
/*                                                              */
/*            See COPYRIGHT for full restrictions               */
/****************************************************************/

#ifndef KERNELBASE_H
#define KERNELBASE_H

#include "MooseObject.h"
#include "SetupInterface.h"
#include "CoupleableMooseVariableDependencyIntermediateInterface.h"
#include "FunctionInterface.h"
#include "UserObjectInterface.h"
#include "TransientInterface.h"
#include "MaterialPropertyInterface.h"
#include "RandomInterface.h"
#include "PostprocessorInterface.h"
#include "GeometricSearchInterface.h"
#include "BlockRestrictable.h"
#include "Assembly.h"
#include "MooseVariable.h"
#include "SubProblem.h"
#include "MooseTypes.h"
#include "Restartable.h"
#include "ZeroInterface.h"
#include "MeshChangedInterface.h"

// libMesh
#include "libmesh/fe.h"
#include "libmesh/quadrature.h"

class MooseMesh;
class Problem;
class SubProblem;
class KernelBase;

template<>
InputParameters validParams<KernelBase>();

class KernelBase :
  public MooseObject,
  public BlockRestrictable,
  public SetupInterface,
  public CoupleableMooseVariableDependencyIntermediateInterface,
  public FunctionInterface,
  public UserObjectInterface,
  public TransientInterface,
  public PostprocessorInterface,
  public MaterialPropertyInterface,
  public RandomInterface,
  protected GeometricSearchInterface,
  public Restartable,
  public ZeroInterface,
  public MeshChangedInterface
{
public:
  KernelBase(const std::string & name, InputParameters parameters);

  virtual ~KernelBase();

  /// Compute this Kernel's contribution to the residual
  virtual void computeResidual() = 0;

  /// Compute this Kernel's contribution to the diagonal Jacobian entries
  virtual void computeJacobian() = 0;

  /// Computes d-residual / d-jvar... storing the result in Ke.
  virtual void computeOffDiagJacobian(unsigned int jvar) = 0;

  /**
   * Computes jacobian block with respect to a scalar variable
   * @param jvar The number of the scalar variable
   */
  virtual void computeOffDiagJacobianScalar(unsigned int jvar) = 0;

  /// Returns the variable number that this Kernel operates on.
  MooseVariable & variable();

  /// Returns a reference to the subproblem for which this Kernel is active
  SubProblem & subProblem();

protected:
  /// Reference to this kernel's SubProblem
  SubProblem & _subproblem;

  /// Reference to this kernel's FEProblem
  FEProblem & _fe_problem;

  /// Reference to the EquationSystem object
  SystemBase & _sys;

  /// The thread ID for this kernel
  THREAD_ID _tid;

  Assembly & _assembly;
  MooseVariable & _var;
  MooseMesh & _mesh;

  const Elem * & _current_elem;

  /// Volume of the current element
  const Real & _current_elem_volume;

  unsigned int _qp;
  const MooseArray< Point > & _q_point;
  QBase * & _qrule;
  const MooseArray<Real> & _JxW;
  const MooseArray<Real> & _coord;

  /// current index for the test function
  unsigned int _i;

  /// current index for the shape function
  unsigned int _j;

  /// the current test function
  const VariableTestValue & _test;

  /// gradient of the test function
  const VariableTestGradient & _grad_test;

  /// the current shape functions
  const VariablePhiValue & _phi;

  /// gradient of the shape function
  const VariablePhiGradient & _grad_phi;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseVector<Number> _local_re;

  /// Holds residual entries as they are accumulated by this Kernel
  DenseMatrix<Number> _local_ke;

  /// The aux variables to save the residual contributions to
  bool _has_save_in;
  std::vector<MooseVariable*> _save_in;
  std::vector<AuxVariableName> _save_in_strings;

  /// The aux variables to save the diagonal Jacobian contributions to
  bool _has_diag_save_in;
  std::vector<MooseVariable*> _diag_save_in;
  std::vector<AuxVariableName> _diag_save_in_strings;
};

#endif /* KERNELBASE_H */
