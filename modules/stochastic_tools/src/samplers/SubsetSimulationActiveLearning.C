//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "SubsetSimulationActiveLearning.h"

registerMooseObject("StochasticToolsApp", SubsetSimulationActiveLearning);

InputParameters
SubsetSimulationActiveLearning::validParams()
{
  InputParameters params = ParallelSubsetSimulation::validParams();
  params.addClassDescription("Serial Subset Simulation using Markov Chains and Gaussian Process active learning.");
  return params;
}

SubsetSimulationActiveLearning::SubsetSimulationActiveLearning(const InputParameters & parameters)
  : ParallelSubsetSimulation(parameters)
{
  // Fixing the number of rows to one for serial subset simulation
  const dof_id_type nchains = 1;
  setNumberOfRows(nchains);
  if ((_num_samplessub / nchains) % _count_max > 0)
    mooseError("Number of model evaluations per chain per subset (",
               _num_samplessub / nchains,
               ") should be a multiple of requested chain length (",
               _count_max,
               ").");
}
