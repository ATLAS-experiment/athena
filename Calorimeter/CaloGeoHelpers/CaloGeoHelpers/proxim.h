/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CALOGEOHELPER_PROXIM
#define CALOGEOHELPER_PROXIM

// inline function for handling two phi angles, to avoid 2PI wrap around. 
//

#include <math.h> 
#include <numbers>



inline double proxim(double b,double a)
{
  using std::numbers::pi;
  const double aplus = a + pi;
  const double aminus = a - pi;
  if (b > aplus) [[unlikely]] {
    do {
      b -= 2*pi;
    } while(b > aplus);
  }
  else if (b < aminus) [[unlikely]] {
    do {
      b += 2*pi;
    } while(b < aminus);
  }
  return b;
}


#endif
