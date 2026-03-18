/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT2MBTSBITS_H
#define TRIGT2MBTSBITS_H

#include "AthenaKernel/CLASS_DEF.h"


#include <vector>
#include <string>

class MsgStream;

/** @class TrigT2MbtsBits
 
@author W. H. Bell <W.Bell@cern.ch>

A class to contain the DSP energies and times of each MBTS counter.
*/  
class TrigT2MbtsBits {

 public:
  TrigT2MbtsBits() = default;
  TrigT2MbtsBits(const std::vector<float>& triggerEnergies, const std::vector<float>& triggerTimes);
  ~TrigT2MbtsBits() = default;
  TrigT2MbtsBits(const TrigT2MbtsBits&) = default;
  TrigT2MbtsBits& operator=(const TrigT2MbtsBits&) = default;
  TrigT2MbtsBits(TrigT2MbtsBits&&) noexcept = default;
  TrigT2MbtsBits& operator=(TrigT2MbtsBits&&) noexcept = default;
  
  /** Return the trigger energies of each counter */
  const std::vector<float>& triggerEnergies(void) const { return m_triggerEnergies; }

  /** Return the relative times of the triggers */
  const std::vector<float>& triggerTimes(void) const { return m_triggerTimes; }

  /**  Prints out data members to std::cout */
  void print(void) const;
  
  /**  Prints out data members to MsgStream */
  void print(MsgStream& log) const;

  /** A data member to contain the number of MBTS counters */
  static constexpr int NUM_MBTS = 32;

 private:
  std::vector<float> m_triggerEnergies{TrigT2MbtsBits::NUM_MBTS,0.f};
  std::vector<float> m_triggerTimes{TrigT2MbtsBits::NUM_MBTS,0.f};
};

/// Helper function for printing the object
std::string str(const TrigT2MbtsBits& trigT2MbtsBits);

/// Helper operator for printing the object
MsgStream& operator<< (MsgStream& m, const TrigT2MbtsBits& trigT2MbtsBits);

CLASS_DEF( TrigT2MbtsBits , 100986084 , 1 )
#endif
