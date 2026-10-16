/* emacs: this is -*- c++ -*- */
/**
 **     @file    BasicRandom.h
 **
 **       a Randomnumber generator with a data member TRandom3
 **       so that we can either use a shared generator
 **       or a dediceted one.
 **       NB: there will be seperate instances of this class
 **       but in the case of a shared generator, they will 
 **       all use the same TRandom3
 **       uses the root TRandom3 Mersenne Twistor generator, 
 **       623-diminsionally equidistributed uniform generator.                
 **       from ACM Trans. Mod. Comp. Sim. Vol 8-1 (1998) 3.
 **       This wrapper is to avoid global variables cluttering 
 **       up the place - root is bad enough as it is.
 **
 **     @author  mark sutton
 **     @date    Fri 27 Jan 2012 11:39:47 GMT 
 **
 **     Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 **/


#ifndef   BASICRANDOM_H
#define   BASICRANDOM_H

#include <iostream>

#include "TRandom3.h"


namespace Resplot { 

class BasicRandom {

public:

  BasicRandom(int shared=true, int seed=4357)  /// do we ever use a different seed ?? We probably should
    : m_shared(shared), m_random(0) { 
    if ( shared ) { 
      /// these are all shared now - we use a static thread_local
      /// so they are shared within each thread
      //      static TRandom3 r(seed);
      m_random = &s_random;
      if ( seed!=4357) m_random->SetSeed(seed);
    }
    else {
      //      m_random = new TRandom3(seed);
      m_random = &s_random;
      if ( seed ) m_random->SetSeed(seed);
    } 
  }
  BasicRandom(const BasicRandom &) = delete; //forbid copying
  BasicRandom operator =(const BasicRandom &) = delete; //forbid assignment
  
  //  virtual ~Random() { delete m_rand; } 
  virtual ~BasicRandom() {  if ( !m_shared ) delete m_random; } 
  
  double exp()     { return m_random->Exp(1); } 
  double gauss()   { return m_random->Gaus(); } 
  double uniform() { return m_random->Uniform(); } 


private:
  
  bool                                m_shared;
  static inline thread_local TRandom3 s_random = {4357};
  TRandom3*                           m_random;

};

}

#endif  /// BASICRANDOM_H 










