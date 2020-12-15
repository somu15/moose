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
class Transient;
namespace libMesh
{
class EquationSystems;
class System;
class MeshBase;
}

/**
 * Report the time and iteration information for the simulation.
 */
class MeshInfo : public GeneralReporter
{
public:
  static InputParameters validParams();
  MeshInfo(const InputParameters & parameters);
  virtual void initialize() override {}
  virtual void finalize() override {}
  virtual void execute() override;

protected:
  const MultiMooseEnum & _items;

  // Reporter values to return
  unsigned int & _num_dofs;
  unsigned int & _num_dofs_nl;
  unsigned int & _num_dofs_aux;
  unsigned int & _num_elem;
  unsigned int & _num_node;
  unsigned int & _num_local_dofs;
  unsigned int & _num_local_dofs_nl;
  unsigned int & _num_local_dofs_aux;
  unsigned int & _num_local_elem;
  unsigned int & _num_local_node;

  // Used to allow for optional declare
  unsigned int _dummy_unsigned_int = 0;

  // Helper to perform optional declaration based on "_items"
  unsigned int & declareHelper(const std::string & item_name, const ReporterMode mode);

private:
  const libMesh::EquationSystems & _equation_systems;
  const libMesh::System & _nonlinear_system;
  const libMesh::System & _aux_system;
  const libMesh::MeshBase & _mesh;
};
