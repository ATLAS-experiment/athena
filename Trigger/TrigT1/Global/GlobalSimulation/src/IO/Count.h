/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  Count.h
//  TopoCore
//  Created by Carlos Moreno on 15/01/21.
// Adapted for GlobalL1TopoSim

#ifndef GLOBALSIM_COUNT_H
#define GLOBALSIM_COUNT_H

#include "GaudiKernel/StatusCode.h"

#include "AthenaKernel/CLASS_DEF.h"


#include <ostream>
#include <vector>
#include <bitset>

namespace GlobalSim {
  
  class Count {
  public:

     Count(unsigned int firstBit = 0,
	   unsigned int nBits = 1, 
	   unsigned int sizeCount = 1 ) :
         m_firstBit(firstBit),
 	 m_nBits(nBits),
	 m_sizeCount(sizeCount)
     {};

     // Destructor
     ~Count(){};

     // setters
     StatusCode reset() { m_count = 0; return StatusCode::SUCCESS; }
     
     void setFirstBit(unsigned int startBit) { m_firstBit = startBit; }
     void setNBits(unsigned int nBits) { m_nBits = nBits; }
     void setSizeCount(unsigned int sizeCount);// { m_sizeCount = sizeCount; }
     
     // getters
     // TO-DO add function to get count bits for a determined position
     // TO-DO make ready for executing multiple algorithms in one instance
     
     const std::bitset<128>& getCountBits() const { return m_count; }
     unsigned int firstBit() const { return m_firstBit; }
     unsigned int lastBit() const { return m_firstBit + m_nBits - 1; }
     unsigned int nBits() const { return m_nBits; }
     unsigned int sizeCount() const { return m_sizeCount; }
     
  private:
     std::bitset<128> m_count {0};
     unsigned int m_firstBit {0};
     unsigned int m_nBits {1};
     unsigned int m_sizeCount {1};

  };

}
CLASS_DEF( GlobalSim::Count , 165533003 , 1 )
std::ostream & operator<<(std::ostream&, const GlobalSim::Count&);

#endif
