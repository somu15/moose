//* This file is part of the MOOSE framework
//* https://www.mooseframework.org
//*
//* All rights reserved, see COPYRIGHT for full restrictions
//* https://github.com/idaholab/moose/blob/master/COPYRIGHT
//*
//* Licensed under LGPL 2.1, please see LICENSE for details
//* https://www.gnu.org/licenses/lgpl-2.1.html

#include "Gamma.h"
#include "math.h"
#include "libmesh/utility.h"

registerMooseObject("StochasticToolsApp", Gamma);

// const std::array<Real, 6> Gamma::_a = {
//     {-0.322232431088, -1.0, -0.342242088547, -0.0204231210245, -0.0000453642210148}};
//
// const std::array<Real, 6> Gamma::_b = {
//     {0.099348462606, 0.588581570495, 0.531103462366, 0.10353775285, 0.0038560700634}};

InputParameters
Gamma::validParams()
{
  InputParameters params = Distribution::validParams();
  params.addClassDescription("Gamma distribution");
  params.addRequiredRangeCheckedParam<Real>(
      "alpha", "alpha > 0", "Parameter alpha.");
  params.addRequiredRangeCheckedParam<Real>(
      "beta", "beta > 0", "Parameter beta.");
  return params;
}

Gamma::Gamma(const InputParameters & parameters)
  : Distribution(parameters),
    _alpha(getParam<Real>("alpha")),
    _beta(getParam<Real>("beta"))
{
}

Real
Gamma::pdf(const Real & x, const Real & alpha, const Real & beta)
{
  return std::pow(beta, alpha) / std::tgamma(alpha) * std::pow(x, alpha - 1) * std::exp(-beta * x);
}

Real
Gamma::cdf(const Real & x, const Real & alpha, const Real & beta)
{
  // Approximation from "Handbook of Mathematical Functions" by Abramowitz and Stegun
  Real bx = beta * x;
  Real incgamma = std::pow(bx, alpha - 1) * std::exp(-bx) * (1 + (alpha - 1) / bx + ((alpha - 1) * (alpha - 2)) / std::pow(bx, 2) + ((alpha - 1) * (alpha - 2) * (alpha - 3)) / std::pow(bx, 3) + ((alpha - 1) * (alpha - 2) * (alpha - 3) * (alpha - 4)) / std::pow(bx, 4) + ((alpha - 1) * (alpha - 2) * (alpha - 3) * (alpha - 4) * (alpha - 5)) / std::pow(bx, 5));
  return (1 - incgamma / std::tgamma(alpha));
}

Real
Gamma::quantile(const Real & p, const Real & alpha, const Real & beta)
{
  std::gamma_distribution(generator);
  Real bx;
  // Approximation from "EFFICIENT AND ACCURATE ALGORITHMS FOR THE COMPUTATION AND INVERSION OF THE INCOMPLETE GAMMA FUNCTION RATIOS"
  if (p < 0.02){
    Real r = std::pow((p * std::tgamma(1+alpha)), 1/alpha);
    Real c2 = 1 / (alpha + 1);
    Real c3 = (3 * alpha + 5) / (2 * std::pow((alpha + 1), 2) * (alpha + 2));
    Real c4 = (8 * std::pow(alpha, 2) + 33 * alpha + 31) / (3 * std::pow((alpha + 1), 3) * (alpha + 2) * (alpha + 3));
    Real c5 = (125 * std::pow(alpha, 4) + 1179 * std::pow(alpha, 3) + 3971 * std::pow(alpha, 2) + 5661 * alpha + 2888) / (24 * std::pow((alpha + 1), 4) * std::pow((alpha + 2), 2) * (alpha + 3) * (alpha + 4));
    bx = r + c2 * std::pow(r, 2) + c3 * std::pow(r, 3) + c4 * std::pow(r, 4) + c5 * std::pow(r, 5);
  } else if (alpha < 0.1){
    Real xl = std::pow((p * std::tgamma(alpha + 1)), 1/alpha);
    Real xu = -std::log(1 - (p * std::tgamma(alpha + 1)));
    bx = (xl + xu) / 2;
  } else {
    Real val1 = 1 - 2 * (1 - p);
    // Inverse error function approximation below by "A handy approximation for the error function and its inverse"
    Real val2 = 0.5 * std::sqrt(M_PI) * (val1 + std::pow(val1, 3) * M_PI / 12 + std::pow(val1, 5) * 7 * std::pow(M_PI, 2) / 480 + std::pow(val1, 7) * 127 * std::pow(M_PI, 3) / 40320 + std::pow(val1, 9) * 4369 * std::pow(M_PI, 4) / 5806080);
    Real etao = val2 / (std::sqrt(alpha / 2));
    // Assuming eta = gamma - 1
    bx = alpha * (etao + 1);
  }
  return bx / beta;
}

Real
Gamma::pdf(const Real & x) const
{
  TIME_SECTION(_perf_pdf);
  return pdf(x, _alpha, _beta);
}

Real
Gamma::cdf(const Real & x) const
{
  TIME_SECTION(_perf_cdf);
  return cdf(x, _alpha, _beta);
}

Real
Gamma::quantile(const Real & p) const
{
  TIME_SECTION(_perf_quantile);
  return quantile(p, _alpha, _beta);
}
