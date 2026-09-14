/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGMUONEFFICIENCY_MUONEFFICIENCYTOOL_H
#define TRIGMUONEFFICIENCY_MUONEFFICIENCYTOOL_H


#include "TrigMuonMatching/ITrigMuonMatching.h"

#include "AsgTools/AsgMetadataTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/ToolHandle.h"

#include "TrigDecisionTool/TrigDecisionTool.h"

namespace Trig {

  class TrigMuonMatching : 
    public asg::AsgMetadataTool,
    public virtual ITrigMuonMatching
    {
      ASG_TOOL_INTERFACE(Trig::TrigMuonMatching)
      ASG_TOOL_CLASS2( TrigMuonMatching, Trig::ITrigMuonMatching, Trig::TrigMuonMatching )
      
   public:

      TrigMuonMatching( const std::string& name );

      virtual ~TrigMuonMatching();

      virtual StatusCode initialize(void) override;
      
      virtual Bool_t match(const xAOD::Muon* mu,
			   std::string_view chain,
			   const double mindelR = 0.1) const override;
      
      virtual Bool_t matchL1(const xAOD::Muon* mu,
			     std::string_view l1item,
			     const double DelR = 0.2) const override;

      virtual Bool_t matchL2SA(const xAOD::Muon* mu,
			       std::string_view l1item,
			       std::string_view chain,
			       const double DelR = 0.2) const override;

      virtual Bool_t matchL2CB(const xAOD::Muon* mu,
			       std::string_view chain,
			       const double DelR = 0.2) const override;
      
      virtual Double_t minDelR(const xAOD::Muon* mu,
			       std::string_view chain,
			       const double mindelR = 0.1) const override;
      
      virtual Double_t minDelRL1(const xAOD::Muon* mu,
				 std::string_view l1item,
				 const double DelR = 0.2) const override;
      
      virtual Bool_t matchDimuon(const xAOD::Muon* mu1,
				 const xAOD::Muon* mu2,
				 const std::string& chain,
				 std::pair<Bool_t, Bool_t>& result1,
				 std::pair<Bool_t, Bool_t>& result2,
				 const Double_t& mindelR = 0.1) override;
      
      virtual Bool_t match(const double eta,
			   const double phi,
			   std::string_view chain,
			   const double mindelR = 0.1) const override;
      
      virtual Bool_t matchL1(const double eta,
			     const double phi,
			     std::string_view l1item,
			     const double DelR = 0.2) const override;
      
      virtual Bool_t matchDimuon(const TLorentzVector& muon1,
				 const TLorentzVector& muon2,
				 const std::string& chain,
				 std::pair<Bool_t, Bool_t>& result1,
				 std::pair<Bool_t, Bool_t>& result2,
				 const Double_t& mindelR = 0.1) override;

      virtual Bool_t isPassedRerun(const std::string& trigger) const override;
      
      struct EFmuon {
	bool valid{};
	float pt{-1.e30};
	float eta{-1.e30};
	float phi{-1.e30};
	
      };
      
    private:

      ToolHandle<Trig::TrigDecisionTool> m_trigDecTool;

      std::map<std::string, DimuonChainInfo> m_DimuonChainMap;

      std::pair<bool,bool> matchDimuon(const TLorentzVector& muon1,
				       const TLorentzVector& muon2,
				       const DimuonChainInfo& chainInfo,
				       const double mindelR);

      double dR(const double eta1,
		const double phi1,
		const double eta2,
		const double phi2) const;

      int getL1pt(std::string_view l1item) const;
      
      
      
      Double_t matchedTrackDetail(EFmuon& efMuonId,
				  const EFmuon& usedEFMuonId,
				  const double eta,
				  const double phi,
				  const double mindelR,
				  std::string_view chainEventTrigger) const;
      
      bool decodeDimuonChain(DimuonChainInfo& chainInfo);

      bool isEqual(const double x,
		   const double y) const;
      
      
    }; 
} 

#endif 
