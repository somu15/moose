#include "ActiveLearningFullSolveMultiApp.h"

// Active learning
#include "StochasticToolsAppTypes.h"

#include "FEProblemBase.h"

registerMooseObject("StochasticToolsApp", ActiveLearningFullSolveMultiApp);

InputParameters
ActiveLearningFullSolveMultiApp::validParams()
{
  InputParameters params = SamplerFullSolveMultiApp::validParams();
  params.addClassDescription("This is SamplerFullSolveMultiApp with some extra flags registered for active learning.");
  ExecFlagEnum exec_enum = ExecFlagEnum();
  exec_enum.addAvailableFlags(EXEC_NONE, EXEC_SAMPLER, EXEC_SUBAPP1);
  params.addParam<ExecFlagEnum>(
      "execute_on", exec_enum, "List of flags indicating when this multiapp should solve.");

  params.addParam<bool>("reset_app", false, "Whether to reset app after each solve.");

  params.suppressParameter<std::vector<Point>>("positions");
  params.suppressParameter<bool>("output_in_position");
  params.suppressParameter<std::vector<FileName>>("positions_file");
  params.suppressParameter<Real>("move_time");
  params.suppressParameter<std::vector<Point>>("move_positions");
  params.suppressParameter<std::vector<unsigned int>>("move_apps");
  params.set<bool>("use_positions") = false;

  return params;
}

ActiveLearningFullSolveMultiApp::ActiveLearningFullSolveMultiApp(const InputParameters & parameters)
  : SamplerFullSolveMultiApp(parameters)
{
  // initilialize mpi for a single subapp per multiapp (this is the default that would usually
  // happen if use_positions = true but that doesn't make any sense for what I'm doing)
  init(1);
}

// void
// ActiveLearningFullSolveMultiApp::preTransfer(Real dt, Real target_time)
// {
//   // if (getParam<bool>("reset_app"))
//   // {
//   //   for (unsigned int i = 0; i < _my_num_apps; ++i)
//   //     resetApp(i, target_time);
//   //   _reset_happened = true;
//   //   _fe_problem.execute(EXEC_PRE_MULTIAPP_SETUP);
//   //   initialSetup();
//   // }
//   //
//   // SamplerFullSolveMultiApp::preTransfer(dt, target_time);
// }
