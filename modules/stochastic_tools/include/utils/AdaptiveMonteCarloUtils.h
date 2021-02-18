/*************************************************/
/*           DO NOT MODIFY THIS HEADER           */
/*                                               */
/*                     MASTODON                  */
/*                                               */
/*    (c) 2015 Battelle Energy Alliance, LLC     */
/*            ALL RIGHTS RESERVED                */
/*                                               */
/*   Prepared by Battelle Energy Alliance, LLC   */
/*     With the U. S. Department of Energy       */
/*                                               */
/*     See COPYRIGHT for full restrictions       */
/*************************************************/

#ifndef ADAPTIVEMONTECARLOUTILS_H
#define ADAPTIVEMONTECARLOUTILS_H

// MOOSE includes
// #include "GeneralVectorPostprocessor.h"

// Forward Declarations
namespace AdaptiveMonteCarloUtils
{
/**
 *  The responseSpectrum function calculates the response spectrum for a
 *  given acceleration history.
 */
 std::vector<Real> sortINPUT(const std::vector<Real> & inputs, const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

 std::vector<Real> sortOUTPUT(const std::vector<Real> & outputs, const int & samplessub, const unsigned int & subset, const Real & subset_prob);

 Real computeMIN(const std::vector<Real> & data);

} // namespace AdaptiveMonteCarloUtils
#endif
