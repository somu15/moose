//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#pragma once

#include "Distribution.h"

/**
 * A class used to generate a KernelDensity1D distribution
 */
class KernelDensity1D : public Distribution
{
public:
  static InputParameters validParams();

  KernelDensity1D(const InputParameters & parameters);

  virtual Real pdf(const Real & x) const override;
  virtual Real cdf(const Real & x) const override;
  virtual Real quantile(const Real & p) const override;

  static Real pdf(const Real & x, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction);
  static Real cdf(const Real & x, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction);
  static Real quantile(const Real & p, const Real & bandwidth, const std::vector<Real> & data, const MooseEnum & kernelfunction);

protected:

  const MooseEnum & _bandwidthrule;
  const MooseEnum & _dataformat;
  const MooseEnum & _kernelfunction;

  /// The bandwith parameter which controls the smoothness of the distribution
  Real _bandwidth;

  /// Data and file_name helps get the vector data for building the kernel density
  bool _data_set;
  bool _file_name_set;

  std::vector<Real> _data;
  const FileName _file_name;

};
