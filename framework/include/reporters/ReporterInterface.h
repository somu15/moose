//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html
#pragma once
#include "MooseTypes.h"
#include "ReporterData.h"

class InputParameters;
class FEProblemBase;

/**
 * Interface to allow object to consume Reporter values.
 */
class ReporterInterface
{
public:
  static InputParameters validParams();
  ReporterInterface(const MooseObject * moose_object);

protected:
  ///@{
  /**
   * Returns read-only reference to a Reporter value that is provided by an input parameter.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param param_name The name of the parameter that gives the name of the Reporter, which
   *                   must be a ReporterName parameter (i.e., getParam<ReporterName>(param_name)).
   * @param mode The mode that the object will consume the Reporter value
   * @pararm time_index (optional) If zero is provided the current value is returned. Use a positive
   *                    index to return previous values (1 = older, 2 = older, etc.). The maximum
   *                    number of old values is dictated by the ReporterData object.
   */
  template <typename T>
  const T & getReporterValue(const std::string & param_name, const std::size_t time_index = 0);
  template <typename T>
  const T & getReporterValue(const std::string & param_name,
                             ReporterMode mode,
                             const std::size_t time_index = 0);
  ///@}

  ///@{
  /**
   * Returns read-only reference to a Reporter value that is provided by name directly.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param reporter_name A ReporterName object that for the desired Reporter value.
   * @param mode The mode that the object will consume the Reporter value
   * @pararm time_index (optional) If zero is provided the current value is returned. Use a positive
   *                    index to return previous values (1 = older, 2 = older, etc.). The maximum
   *                    number of old values is dictated by the ReporterData object.
   */
  template <typename T>
  const T & getReporterValueByName(const ReporterName & reporter_name,
                                   const std::size_t time_index = 0);
  template <typename T>
  const T & getReporterValueByName(const ReporterName & reporter_name,
                                   ReporterMode mode,
                                   const std::size_t time_index = 0);
  ///@}

  ///@{
  /**
   * Return True if the Reporter value exists.
   * @tparam T The C++ type of the Reporter value being consumed
   * @param reporter_name A ReporterName object that for the desired Reporter value.
   */
  bool hasReporterValue(const std::string & param_name) const;
  bool hasReporterValueByName(const ReporterName & reporter_name) const;
  ///@}

  /**
   * A method that can be overridden to update the UO dependencies.
   *
   * This is needed because the get methods for this interface cannot be virtual because of the
   * template parameter. See GeneralUserObject for how it is utilized.
   */
  virtual void addReporterDependencyHelper(const ReporterName & /*state_name*/) {}

private:
  /// Parameters for the MooseObject inherting from this interface
  const InputParameters & _ri_params;

  /// Provides access to FEProblemBase::getReporterData
  FEProblemBase & _ri_fe_problem_base;

  /// Non-const access to Reporter data (keep it private please)
  ReporterData & _ri_reporter_data;

  /// The name of the object
  const std::string & _ri_name;

  /// A few helpers to avoid cyclic include problems
  const ReporterName & getReporterNameHelper(const std::string & param_name) const;
};

template <typename T>
const T &
ReporterInterface::getReporterValue(const std::string & param_name, const std::size_t time_index)
{
  return getReporterValue<T>(param_name, REPORTER_MODE_UNSET, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValue(const std::string & param_name,
                                    ReporterMode mode,
                                    const std::size_t time_index)
{
  const ReporterName & reporter_name = _ri_params.template get<ReporterName>(param_name);
  return getReporterValueByName<T>(reporter_name, mode, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValueByName(const ReporterName & reporter_name,
                                          const std::size_t time_index)
{
  return getReporterValueByName<T>(reporter_name, REPORTER_MODE_UNSET, time_index);
}

template <typename T>
const T &
ReporterInterface::getReporterValueByName(const ReporterName & reporter_name,
                                          ReporterMode mode,
                                          const std::size_t time_index)
{
  addReporterDependencyHelper(reporter_name);
  return _ri_reporter_data.getReporterValue<T>(reporter_name, _ri_name, mode, time_index);
}
