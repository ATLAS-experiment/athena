/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  Decision.h
//  TopoCore
//  Created by Joerg Stelzer on 11/16/12.
//
// Adapted for GlobalL1TopoSim
#ifndef GLOBALSIM_DECISION_H
#define GLOBALSIM_DECISION_H


#include <iosfwd>

#include "AthenaKernel/CLASS_DEF.h"


namespace TCS {

}

namespace GlobalSim {

   class Decision {
   public:
      
      Decision(unsigned int firstBit = 0, unsigned int nBits = 1) :
         m_firstBit(firstBit),
         m_nBits(nBits),
         m_overflow(false)
      {};

      // destructor
      ~Decision(){};

      // setters
      StatusCode reset() { m_decision = 0; return StatusCode::SUCCESS; }
      void setBit(unsigned int index, bool value);

      void setFirstBit(unsigned int startBit) { m_firstBit = startBit; }
      void setNBits(unsigned int nBits) { m_nBits = nBits; }
      void setOverflow(bool value) { m_overflow = value; }
      void setAmbiguity(bool value) { m_ambiguity = value; }
      // getters
      bool bit(unsigned int index) const { return (m_decision & (0x1<<index)) != 0; }
      uint64_t decision() const { return m_decision; }
      unsigned int firstBit() const { return m_firstBit; }
      unsigned int lastBit() const { return m_firstBit + m_nBits - 1; }
      unsigned int nBits() const { return m_nBits; }
      bool overflow() const { return m_overflow; }
      bool ambiguity() const { return m_ambiguity; }

   private:
      uint32_t     m_decision {0};
      unsigned int m_firstBit {0};
      unsigned int m_nBits {1};
      bool         m_overflow;
     bool         m_ambiguity{false};

   };
   
}

CLASS_DEF( GlobalSim::Decision , 256155971 , 1 )

std::ostream & operator<<(std::ostream&, const GlobalSim::Decision &);

#endif
