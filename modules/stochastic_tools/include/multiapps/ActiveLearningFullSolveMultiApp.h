#pragma once
// // fixme lynn this is included because MultiApp probably needs it
// #include "libmesh/numeric_vector.h"
// MOOSE includes
#include "SamplerFullSolveMultiApp.h"

/**
 * This is SamplerFullSolveMultiApp with some extra flags registered for active learning.
 */
class ActiveLearningFullSolveMultiApp : public SamplerFullSolveMultiApp
{
public:
  static InputParameters validParams();
  ActiveLearningFullSolveMultiApp(const InputParameters & parameters);

  // virtual void preTransfer(Real dt, Real target_time) override;
};
