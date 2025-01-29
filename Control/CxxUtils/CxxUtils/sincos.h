// This file's extension implies that it's C, but it's really -*- C++ -*-.
/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file CxxUtils/sincos.h
 * @author scott snyder
 * @date Nov 2008, from older D0 code.
 * @brief Helper to simultaneously calculate sin and cos of the same angle.
 */


#ifndef CXXUTILS_SINCOS_H
#define CXXUTILS_SINCOS_H


#include <cmath>


namespace CxxUtils {


/**
 * @brief Helper to simultaneously calculate sin and cos of the same angle.
 *
 * Instantiate an instance of this object, passing the angle to the
 * constructor.  The sin and cos are then available as the sn and cs
 * members.  In addition, the apply() method may be used to calculate
 * a*sin(x) + b*cos(x).
 *
 * Implementation notes:
 * 
 * This used to inline the fsincos x87 instruction.  However, with
 * current compilers on x86_64, it's faster to just use sincos().
 * We keep this around since sincos() is a GNU extension and we
 * don't want to have to put #ifdefs everywhere we use this.
 */
struct sincos
{
  /// Calculate sine and cosine of x.
  sincos (double x)
#if defined(__USE_GNU)
  // Version using the GNU sincos() function.
  { ::sincos(x, &sn, &cs); }
#else
  // Generic version.
  : sn (std::sin (x)), cs (std::cos (x)) {}
#endif

  /// @f$\sin(x)@f$
  double sn;

  /// @f$\cos(x)@f$
  double cs;

  /// @f$a\sin(x) + b\cos(x)@f$
  double apply (double a, double b) const { return a*sn + b*cs; }

  /// @f$a\sin^2(x) + b\sin(x)\cos(x) + c\cos^2(x)@f$
  double apply2 (double a, double b, double c) const
  { return a*sn*sn + b*sn*cs + c*cs*cs; }
};


} // namespace CxxUtils


#endif //not CXXUTILS_SINCOS_H
