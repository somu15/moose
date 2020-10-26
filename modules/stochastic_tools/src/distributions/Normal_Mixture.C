//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Normal_Mixture.h"
#include "math.h"
#include "libmesh/utility.h"
#include "Normal.h"

registerMooseObject("StochasticToolsApp", Normal_Mixture);

InputParameters
Normal_Mixture::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Normal_Mixture distribution");
  params.addRequiredParam<std::vector<Real>>("mean", "Mean (or expectation) vector of the mixture distribution.");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "standard_deviation", "standard_deviation > 0", "Standard deviation vector of the mixture distribution ");
  params.addRequiredRangeCheckedParam<std::vector<Real>>(
      "weight", "weight > 0", "Weight vector of the mixture distribution ");
  return params;
}

Normal_Mixture::Normal_Mixture(const InputParameters & parameters)
  : Distribution(parameters),
    _mean(getParam<std::vector<Real>>("mean")),
    _standard_deviation(getParam<std::vector<Real>>("standard_deviation")),
    _weight(getParam<std::vector<Real>>("weight"))
{
}

Real
Normal_Mixture::pdf(const Real & x, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight)
{
  Real tmp1 = 0;
  for (unsigned int i = 0; i < weight.size(); ++i)
    tmp1 += weight[i] * Normal::pdf(x, mean[i], std_dev[i]);
  return tmp1;
}

Real
Normal_Mixture::cdf(const Real & x, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight)
{
  Real tmp1 = 0;
  for (unsigned int i = 0; i < weight.size(); ++i)
    tmp1 += weight[i] * Normal::cdf(x, mean[i], std_dev[i]);
  return tmp1;
}

Real
Normal_Mixture::quantile(const Real & p, const std::vector<Real> & mean, const std::vector<Real> & std_dev, const std::vector<Real> & weight)
{
  std::vector<Real> tmp1;
  unsigned int min_pos = 0;
  tmp1.resize(weight.size());
  for (unsigned int i = 0; i <weight.size(); ++i)
  {
    tmp1[i] = std::abs(weight[i] - p);
    if (tmp1[i] < tmp1[min_pos])
      min_pos = i;
  }
  return Normal::quantile(p, mean[min_pos], std_dev[min_pos]);
}

Real
Normal_Mixture::pdf(const Real & x) const
{
  TIME_SECTION(_perf_pdf);
  return pdf(x, _mean, _standard_deviation, _weight);
}

Real
Normal_Mixture::cdf(const Real & x) const
{
  TIME_SECTION(_perf_cdf);
  return cdf(x, _mean, _standard_deviation, _weight);
}

Real
Normal_Mixture::quantile(const Real & p) const
{
  TIME_SECTION(_perf_quantile);
  return quantile(p, _mean, _standard_deviation, _weight);
}
