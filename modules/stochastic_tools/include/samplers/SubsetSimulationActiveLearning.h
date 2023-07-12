//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "ParallelSubsetSimulation.h"
#include "ReporterInterface.h"

/**
 * A class used to perform serial Subset Simulation using Markov Chains
 * and Gaussian Process active learning
 */
class SubsetSimulationActiveLearning : public ParallelSubsetSimulation
{
public:
  static InputParameters validParams();

  SubsetSimulationActiveLearning(const InputParameters & parameters);
};
