/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
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
      virtual ~cTauMultiplicity() = default;

      virtual StatusCode initialize() override;

      virtual StatusCode processBitCorrect(const TCS::InputTOBArray& input, Count& count) override final;

      virtual StatusCode process(const TCS::InputTOBArray& input, Count& count ) override final;

      #ifndef TRIGCONF_STANDALONE
      // Functions for HLT seeding
      // Returns index of jtau matched to etau
      static size_t cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoIContainer& jTauRoIs);
      // Returns true when a matching is found
      static bool cTauMatching(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoI& jTau);
      // Check cTAU isolation
      static bool checkIsolationWP(const xAOD::eFexTauRoI& eTau, const xAOD::jFexTauRoI& jTau, const TrigConf::L1Threshold_cTAU& thr);
      // Check eTAU rCore/BDT and rHad WPs
      static bool checkeTAUWP(const xAOD::eFexTauRoI& eTau, const TrigConf::L1Threshold_cTAU& thr);
      #endif

   private:
      const TrigConf::L1Threshold_cTAU* m_threshold{nullptr};
      std::shared_ptr<TrigConf::L1ThrExtraInfo_cTAU> m_extraInfo;

      bool checkIsolationWP(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand) const; 
      bool checkeTAUWP(const TCS::cTauTOB* etauCand) const;

      // Matching function for L1Topo
      bool cTauMatching(const TCS::cTauTOB* etauCand, const TCS::cTauTOB* jtauCand) const; 

     // cTau monitoring histograms
     std::vector<std::string> m_histcTauEt;
     std::vector<std::string> m_histcTauPhiEta;
     std::vector<std::string> m_histcTauEtEta;
     std::vector<std::string> m_histcTauIsoFraction;
     std::vector<std::string> m_histcTauIsoMatchedPass;

     using WP = TrigConf::Selection::WP;
   };

} 

#endif 
