#pragma once

#include "Steady.h"
#include "ActiveLearningSolve.h"

// System includes
#include <string>

// Forward declarations
class InputParameters;
class ActiveLearning;
class FEProblemBase;

class ActiveLearning : public Steady
{
public:
  static InputParameters validParams();

  ActiveLearning(const InputParameters & parameters);

  virtual void execute() override;

protected:
  ActiveLearningSolve _al_solve;
};
