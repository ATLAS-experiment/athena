/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/*********************************
 * DeltaRSqrIncl2.cxx
 * Based on same-named code in L1TopoSimulation
 *
 * @brief algorithm calculates the R2-distance between two lists and
 * applies dR criteria
 *
 * @param NumberLeading
 **********************************/


#include "DeltaRSqrIncl2.h"

#include "L1TopoSimulationUtils/Kinematics.h" // calcDeltaR2BW()

#include "L1TopoEvent/CompositeTOB.h"

#include "../IO/GenericTOBArray.h"
#include "../IO/Decision.h"

#include "GaudiKernel/StatusCode.h"
#include <sstream>

namespace GlobalSim {
  DeltaRSqrIncl2::DeltaRSqrIncl2(const std::string & name,
				 unsigned int maxTOB1,
				 unsigned int maxTOB2,
				 const std::vector<unsigned int>& minET1,
				 const std::vector<unsigned int>& minET2,
				 const std::vector<unsigned int>& deltaRMin,
				 const std::vector<unsigned int>& deltaRMax,
				 unsigned int NumResultBits):

    m_name{name},
    m_MaxTOB1{maxTOB1},
    m_MaxTOB2{maxTOB2},
    m_MinET1{minET1},
    m_MinET2{minET2},
    m_DeltaRMin{deltaRMin},
    m_DeltaRMax{deltaRMax},
    m_NumResultBits{NumResultBits}{
  }

  StatusCode
  DeltaRSqrIncl2::run(const GlobalSim::GenericTOBArray& tobArray1,
		      const GlobalSim::GenericTOBArray& tobArray2,
		      std::vector<GenericTOBArray>& output,
		      Decision& decision) {

    std::size_t count1{0u};
    std::size_t count2{0u};

    output.clear();
    output.reserve(m_NumResultBits);
    for (std::size_t i = 0; i != m_NumResultBits; ++i) {
      std::stringstream ss;
      ss << "SimpleCone bit " << i;
      output.push_back(GenericTOBArray(ss.str()));
    }


    // consider all tob pairs formed by 1 tob from tobArray1 and
    // 1 tob formed from tobArray2 up to the number of leading tobs
    for(GenericTOBArray::const_iterator i_tob1 = tobArray1.begin(); 
	i_tob1 != tobArray1.end();
	++i_tob1) {
      
      if (++count1 == m_MaxTOB1) {break;}
      const auto& tob1 = *i_tob1;
      
      for(GenericTOBArray::const_iterator i_tob2 = tobArray2.begin(); 
	  i_tob2 != tobArray2.end();
	  ++i_tob2) {
	  
	if (++count2 == m_MaxTOB2) {break;}
	
	const auto& tob2 = *i_tob2;
	
	// test DeltaR2Min, DeltaR2Max
	unsigned int deltaR2 = TSU::Kinematics::calcDeltaR2(tob1, tob2);
	
	for(unsigned int i=0; i<m_NumResultBits; ++i) {
	  bool accept = false;
	  if( tob1->Et() <= m_MinET1[i]) continue; // ET cut
	  if( tob2->Et() <= m_MinET2[i]) continue; // ET cut
	  accept = deltaR2 >= m_DeltaRMin[i] && deltaR2 <= m_DeltaRMax[i];
	  
	  if(accept) {
	    decision.setBit(i, true);
	    output[i].push_back(TCS::CompositeTOB(tob1, tob2));
	  }
	  
	  if (accept) {
	    m_DeltaRSqPass[i].push_back(deltaR2);
	  } else {
	    m_DeltaRSqFail[i].push_back(deltaR2);
	  }
	}
      }
    }
      
    return StatusCode::SUCCESS;
  }

  unsigned int DeltaRSqrIncl2::numResultBits() const {
    return m_NumResultBits;
  }
  
  const std::vector<double>&
  DeltaRSqrIncl2::deltaRSqPassByBit(std::size_t bit) const {
    return m_DeltaRSqPass.at(bit);
  }
  
  const std::vector<double>&
  DeltaRSqrIncl2::deltaRSqFailByBit(std::size_t bit) const {
    return m_DeltaRSqFail.at(bit);
  }


  

  std::string DeltaRSqrIncl2::toString() const{
    std::stringstream ss;
    ss << "DeltaRSqrIncl2. "
       << " name: " << m_name
       << " MaxTOB1 " << m_MaxTOB1
       << " MaxTOB2 " << m_MaxTOB2
       << " n result bits " << m_NumResultBits;

	 
    ss << "\n min deltaR [" << m_DeltaRMin.size() << "]\n";

    for(const auto& v: m_DeltaRMin){
      ss << " " << v << " ";
    }
       
    ss << "\n max deltaR[" << m_DeltaRMax.size() << "]:\n";

    for(const auto& v: m_DeltaRMax){
      ss << " " << v << " ";
    }

    ss << "\n min ET1 [" << m_MinET1.size() << "]:\n";
    for(const auto& v: m_MinET1){
      ss << " " << v << " ";
    }
       
    ss << "\n min ET2 [" << m_MinET2.size() << "]:\n";


    for(const auto& v: m_MinET2){
      ss << " " << v << " ";
    }

    return ss.str();
  }
}

