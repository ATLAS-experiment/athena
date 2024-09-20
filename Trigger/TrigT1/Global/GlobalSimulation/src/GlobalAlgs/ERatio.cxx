/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "ERatio.h"

#include "GaudiKernel/StatusCode.h"
#include <sstream>
#include <limits>

namespace GlobalSim {
  ERatio::ERatio(const std::string & name,
		 double minDeltaE,
		 double maxERCut) :
    m_name{name}, m_minDeltaE{minDeltaE}, m_maxERCut{maxERCut} {
  }

  StatusCode
  ERatio::run(const LArStripNeighborhood& nbhd, bool& result){

    result = false;

    std::size_t maxCellInd = nbhd.maxCellIndex();
    auto p_peakE = nbhd.phi_center().at(maxCellInd).m_e;

    for (const auto& ps : {nbhd.phi_center(),
			   nbhd.phi_low(),
			   nbhd.phi_high()})  {
      
      double s_peakE = secondaryPeakEnergy_forw(ps, maxCellInd);

      result = eratioCut(p_peakE, s_peakE);
      if (result) {break;}
      
      s_peakE = secondaryPeakEnergy_back(ps, maxCellInd);
   
      result = eratioCut(p_peakE, s_peakE);

      if (result) {break;} 
    }

    return StatusCode::SUCCESS;
  }


  double ERatio::secondaryPeakEnergy_forw(const StripDataVector& phi_str,
					  std::size_t maxCellInd) {

    /*
     * Look for a secondary peak in the direction of increasing eta
     * starting at the eta location of the maximum energy strip.
     */
    
    constexpr double maxStripE{std::numeric_limits<double>::max()};
    double peakE{0.};

    // eratio needs at least three elements
    auto sz = phi_str.size();
    if (sz < 3 or maxCellInd > sz) {return peakE;}

    bool valley =
      phi_str.at(maxCellInd+1).m_e < maxStripE - m_minDeltaE;
    
    for(auto iter = std::begin(phi_str)+maxCellInd+2;
	iter != std::end(phi_str);
	++iter){
      
      if(!valley) {
	valley = iter->m_e < ((iter-1)->m_e) - m_minDeltaE;
      }

      if(valley) {
	auto c_cellE = iter->m_e;
	auto p_cellE = (iter-1)->m_e;
	if(c_cellE > p_cellE + m_minDeltaE) {
	  peakE = std::max(peakE, c_cellE);
	}
      }
      
    }
    return peakE;
  }

  
  double ERatio::secondaryPeakEnergy_back(const StripDataVector& phi_str,
					  std::size_t maxCellInd) {

    
    /*
     * Look for a secondary peak in the direction of decreasing eta
     * starting at the eta location of the maximum energy strip.
     */

    
    constexpr double maxStripE{std::numeric_limits<double>::max()};
    double peakE{0.};

    // eratio needs at least three elements
    auto sz = maxCellInd;
    if (sz < 2) {return peakE;}

    bool valley =
      phi_str.at(maxCellInd-1).m_e < maxStripE - m_minDeltaE;


    // calculate offset to point to 2 less than phi_str[maxCellInd]
    std::size_t start = phi_str.size() - maxCellInd + 1;
    for(auto iter = std::rbegin(phi_str)+start;
	iter != std::rend(phi_str);
	++iter){
      
      if(!valley) {
	valley = iter->m_e < ((iter-1)->m_e) - m_minDeltaE;
	continue;
      }

      auto c_cellE = iter->m_e;
      auto p_cellE = (iter-1)->m_e;
      if(c_cellE > p_cellE + m_minDeltaE) {
	peakE = std::max(peakE, c_cellE);
      }
      
    }
    return peakE;
  }


  bool ERatio::eratioCut(double p_peakE, double s_peakE) {

    if (p_peakE <= 0 or s_peakE <= 0) {return false;}

    return std::max(p_peakE, s_peakE)/std::min(p_peakE, s_peakE) < m_maxERCut;

  }

  std::string ERatio::toString() const{
    std::stringstream ss;
    ss << "ERatio. "
       << " name: " << m_name
       << " minimum strip E difference to be significant " << m_minDeltaE
       << " upper ERatio limit for pi0 candidate " << m_maxERCut
       << '\n';

    return ss.str();
  }
}

std::ostream& operator<< (std::ostream& os, const GlobalSim::ERatio& er) {
  os << er.toString();
  return os;
}


