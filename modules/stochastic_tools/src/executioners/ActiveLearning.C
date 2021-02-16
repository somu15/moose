// MOOSE includes
#include "ActiveLearning.h"

registerMooseObject("StochasticToolsApp", ActiveLearning);

InputParameters
ActiveLearning::validParams()
{
  InputParameters params = Steady::validParams();
  params += ActiveLearningSolve::validParams();
  params.addClassDescription("Executioner for active learning Monte Carlo problems.");
  return params;
}

ActiveLearning::ActiveLearning(const InputParameters & parameters) : Steady(parameters), _al_solve(this)
{
  _al_solve.setInnerSolve(_picard_solve);
}

void
ActiveLearning::execute()
{
  _problem.outputStep(EXEC_INITIAL);

  preExecute();

  _problem.advanceState();

  // first step in any steady state solve is always 1 (preserving backwards compatibility)
  _time_step = 1;

  _problem.timestepSetup();

  _al_solve.solve();

  // need to keep _time in sync with _time_step to get correct output
  _time = _time_step;
  _problem.outputStep(EXEC_TIMESTEP_END);
  _time = _system_time;

  _problem.execMultiApps(EXEC_FINAL);
  _problem.finalizeMultiApps();
  _problem.execute(EXEC_FINAL);
  _time = _time_step;
  _problem.outputStep(EXEC_FINAL);
  _time = _system_time;

  postExecute();
}
