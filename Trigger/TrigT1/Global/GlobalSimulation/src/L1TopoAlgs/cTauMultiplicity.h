/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#ifndef GLOBALSIM_CTAUMULTIPLICITY_H
#define GLOBALSIM_CTAUMULTIPLICITY_H


#include <vector>
#include <map>
#include <string>

namespace TrigConf {
  class L1Threshold_cTAU;
}

namespace TCS {
  class cTauTOB;
}

class StatusCode;

namespace GlobalSim {

  class cTauTOBArray;
  class Count;
  
   class cTauMultiplicity{
   public:

     // Unlike other Algorithms converted from L1TopoSimulation,
     // cTauMultiplicity still uses the C++ L1Menu object, as this
     // contains information dereived from the menu, but which has not
     // been made available in python.
     cTauMultiplicity(const std::string& name,
		      unsigned int nbits,
		      const TrigConf::L1Threshold_cTAU&,
		      const std::map<std::string, int>& isoFW_CTAU,
		      const std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale
		      ); 
    
     virtual ~cTauMultiplicity() = default;

     StatusCode run( const cTauTOBArray& input,
		     Count & count );


     // Converts the isolation score to bit to be used for the
     // working point assignement 
     static unsigned int
     convertIsoToBit(const std::map<std::string, int>& isoFW_CTAU,
		     const std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale,
		     const float jTauCoreEt,
		     const float jTauIso,
		     const float eTauEt);


     std::string toString() const;

     // expose data to be monitored

     const std::vector<double>& TOB_et() const;
     const std::vector<double>& TOB_eta() const;
     const std::vector<double>& TOB_phi() const;
     const std::vector<double>& TOB_isolation_partial_loose() const;
     const std::vector<double>& TOB_isolation_partial_medium() const;
     const std::vector<double>& TOB_isolation_partial_tight() const;
     const std::vector<double>& TOB_isoScore() const;
     const std::vector<double>& accept_eta() const;
     const std::vector<double>& accept_et() const;
     const std::vector<double>& counts() const;

   private:

     std::string m_name{};
     unsigned int m_nbits{0};
     
     const TrigConf::L1Threshold_cTAU& m_threshold;

     // data to be monitored
     std::vector<double> m_TOB_et{};
     std::vector<double> m_TOB_eta{};
     std::vector<double> m_TOB_phi{};
     std::vector<double> m_TOB_isolation_partial_loose{};
     std::vector<double> m_TOB_isolation_partial_medium{};
     std::vector<double> m_TOB_isolation_partial_tight{};
     std::vector<double> m_TOB_isoScore{};
     std::vector<double> m_accept_eta{};
     std::vector<double> m_accept_et{};
     std::vector<double> m_counts{};

     std::map<std::string, int> m_isoFW_CTAU;
     std::map<std::string, int> m_isoFW_CTAU_jTAUCoreScale;

     // This function is used to map the ctau isolation working points
     // into a common format with eFEX EM and taus.
     // This allows us to use same functionalities from ConfigurableAlg
     // (L1TopoInterfaces) to apply isolation cuts in multiplicity algorithms
     // for all flavour of TOBS 

     unsigned int convertIsoToBit(const TCS::cTauTOB * etauCand,
				  const TCS::cTauTOB * jtauCand) const; 

     // Matching function for L1Topo
     bool cTauMatching(const TCS::cTauTOB * etauCand,
		       const TCS::cTauTOB * jtauCand) const;
 
   };

} 

#endif 
