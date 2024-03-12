/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
//  cTauMultiplicity.h
//  TopoCore

#ifndef __TopoCore__cTauMultiplicity__
#define __TopoCore__cTauMultiplicity__


#include "L1TopoInterfaces/CountingAlg.h"
#include "L1TopoEvent/TOBArray.h"
#include "TrigConfData/L1Threshold.h"

// Include xAOD headers here
#ifndef TRIGCONF_STANDALONE
  #include "xAODTrigger/eFexTauRoIContainer.h"
  #include "xAODTrigger/jFexTauRoIContainer.h"
#endif

#include <vector>
#include <map>

class TH2;

namespace TCS {
   
   class cTauMultiplicity : public CountingAlg {
   public:
      
      cTauMultiplicity(const std::string & name);
      virtual ~cTauMultiplicity();

      virtual StatusCode initialize() override;

      virtual StatusCode processBitCorrect(const TCS::InputTOBArray& input, Count& count) override final;

      virtual StatusCode process(const TCS::InputTOBArray& input, Count& count ) override final;

      #ifndef TRIGCONF_STANDALONE
      // Functions for HLT seeding
      // Returns index of jtau matched to etau
      static size_t cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoIContainer& jTauRoIs);
      // Returns true when a matching is found
      static bool cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoI& jTau);
      // Converts the isolation score to bit to be used for the working point assignement 
      static bool checkIsolationWP(const std::map<std::string, int>& isoFW_CTAU, const std::map<std::string, int>& isoFW_CTAU_jTAUCoreScale, const float jTauCoreEt, const float jTauIsoEt, const float eTauEt, const std::string& isolation_wp);
      #endif

   private:

      const TrigConf::L1Threshold_cTAU* m_threshold{nullptr};
      std::map<std::string, int> m_isoFW_CTAU;
      std::map<std::string, int> m_isoFW_CTAU_jTAUCoreScale;

      // This function is used to map the ctau isolation working points into a common format with eFEX EM and taus.
      // This allows us to use same functionalities from ConfigurableAlg (L1TopoInterfaces) to apply isolation cuts in multiplicity algorithms for all flavour of TOBS 
      bool checkIsolationWP(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand, const std::string& isolation_wp) const; 

      // Matching function for L1Topo
      bool cTauMatching(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand) const; 

     // cTau monitoring histograms
     std::vector<std::string> m_histcTauEt;
     std::vector<std::string> m_histcTauPhiEta;
     std::vector<std::string> m_histcTauEtEta;
     std::vector<std::string> m_histcTauPartialIsoLoose;
     std::vector<std::string> m_histcTauPartialIsoMedium;
     std::vector<std::string> m_histcTauPartialIsoMedium12;
     std::vector<std::string> m_histcTauPartialIsoMedium20;
     std::vector<std::string> m_histcTauPartialIsoMedium30;
     std::vector<std::string> m_histcTauPartialIsoMedium35;
     std::vector<std::string> m_histcTauPartialIsoTight;
     std::vector<std::string> m_histcTauIsoMatchedPass;

   };

} 

#endif 
