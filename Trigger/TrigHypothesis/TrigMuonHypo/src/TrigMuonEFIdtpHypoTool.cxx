/*
# Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#include "GaudiKernel/SystemOfUnits.h"

#include "TrigCompositeUtils/Combinators.h"
#include "TrigMuonEFIdtpHypoTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

class ISvcLocator;

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

TrigMuonEFIdtpHypoTool::TrigMuonEFIdtpHypoTool(const std::string & type, const std::string & name, const IInterface* parent):
   AthAlgTool(type, name, parent),
   m_decisionId(HLT::Identifier::fromToolName(name)) {
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoTool::initialize()
{
   if( m_muonqualityCut ) {
      if(m_muonSelTool.retrieve().isFailure()) {
	 ATH_MSG_ERROR("Unable to retrieve " << m_muonSelTool);
	 return StatusCode::FAILURE;
      }
   } else m_muonSelTool.disable();
   
   if( m_acceptAll ) {
      ATH_MSG_DEBUG("Accepting all the events!");
   } else {
      if(m_ptBins.size()<=0) { 
	 ATH_MSG_ERROR("Trying to configure hypo with no pT bins. This is probably a configuration mistake.");
	 return StatusCode::FAILURE;
      }
      m_bins.resize(m_ptBins.size());
      for(size_t j=0; j<m_ptBins.size(); j++){
	 m_bins[j] = m_ptBins[j].size() - 1;
	 if (m_bins[j] != m_ptThresholds[j].size()) {
	    ATH_MSG_ERROR("bad thresholds setup .... exiting!");
	    return StatusCode::FAILURE;
	 }
	 if (msgLvl(MSG::DEBUG)) {
	    for (std::vector<float>::size_type i=0; i<m_bins[j];++i) {
	       ATH_MSG_DEBUG( "bin " << m_ptBins[j][i] << " - " <<  m_ptBins[j][i+1]<<" with Pt Threshold of " << (m_ptThresholds[j][i])/Gaudi::Units::GeV<< " GeV");
	    }
	 }
      }
   }

   if ( not m_monTool.name().empty() ) {
      ATH_CHECK( m_monTool.retrieve() );
      ATH_MSG_DEBUG("MonTool name: " << m_monTool);
   }

   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigMuonEFIdtpHypoTool::decideOnSingleObject(TrigMuonEFIdtpHypoTool::MuonEFIdperfInfo& input, size_t cutIndex) const
{
   // for pass through mode
   if(m_acceptAll) {
      ATH_MSG_DEBUG("Accept property is set: taking all the events");
      return true;
   }
   
   const xAOD::Muon* muon = input.muon;
   if( !muon ){
      ATH_MSG_ERROR("Retrieval of xAOD::MuonContainer failed");
      return false;
   }
   if(! muon->primaryTrackParticle()) return false;

   const xAOD::TrackParticle* tr = muon->trackParticle(xAOD::Muon::TrackParticleType::ExtrapolatedMuonSpectrometerTrackParticle);
   if ( !tr ) {
      ATH_MSG_DEBUG("No TrackParticle found.");
      return false;
   }

   // monitored Variables
   std::vector<float> fexPt, fexEta, fexPhi, selPt, selEta, selPhi;
   auto muonPtMon     = Monitored::Collection("SA_pt",      fexPt);
   auto muonEtaMon    = Monitored::Collection("SA_eta",     fexEta);
   auto muonPhiMon    = Monitored::Collection("SA_phi",     fexPhi);
   auto muonPtSelMon  = Monitored::Collection("SA_pt_sel",  selPt);
   auto muonEtaSelMon = Monitored::Collection("SA_eta_sel", selEta);
   auto muonPhiSelMon = Monitored::Collection("SA_phi_sel", selPhi);
   auto monitorIt     = Monitored::Group(m_monTool, muonPtMon, muonEtaMon, muonPhiMon, muonPtSelMon, muonEtaSelMon, muonPhiSelMon); 

   //
   bool result = false;

   ATH_MSG_VERBOSE("Retrieved track with abs pt "<< (*tr).pt()/Gaudi::Units::GeV << " GeV ");
   fexPt.push_back(tr->pt()/Gaudi::Units::GeV);
   fexEta.push_back(tr->eta());
   fexPhi.push_back(tr->phi());

   // apply hypo cuts
   float absEta = std::abs(tr->eta());
   float threshold = 0;
   for (std::vector<float>::size_type k=0; k<m_bins[0]; ++k) {
      if (absEta > m_ptBins[cutIndex][k] && absEta <= m_ptBins[cutIndex][k+1]) threshold = m_ptThresholds[cutIndex][k];
   }
   if ( (std::abs(tr->pt())/Gaudi::Units::GeV > (threshold/Gaudi::Units::GeV)) &&
	( (!m_muonqualityCut) || (m_muonqualityCut && passedQualityCuts(muon)) ) ) { // selection passed
      result = true;
      selPt.push_back(tr->pt()/Gaudi::Units::GeV);
      selEta.push_back(tr->eta());
      selPhi.push_back(tr->phi());
   }
   
   ATH_MSG_VERBOSE(" REGTEST muon pt is " << tr->pt()/Gaudi::Units::GeV << " GeV "
		   << " with Charge " << tr->charge()
		   << " and threshold cut is " << threshold/Gaudi::Units::GeV << " GeV"
		   << " so hypothesis is " << (result?"true":"false"));
   
   // hypo result
   return result;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

bool TrigMuonEFIdtpHypoTool::passedQualityCuts(const xAOD::Muon* muon) const
{
   bool passCut = false;

   const xAOD::TrackParticle* metrack = muon->trackParticle( xAOD::Muon::ExtrapolatedMuonSpectrometerTrackParticle );
   float reducedChi2 = -10;
   
   if( metrack ) {
      reducedChi2 = muon->primaryTrackParticle()->chiSquared()/muon->primaryTrackParticle()->numberDoF(); 
      // Selection criteria based on the requirements that are part of the muon quality working points (offline)
      if(std::abs(reducedChi2) < 8.0 && !m_muonSelTool->isBadMuon(*muon) && muon->author()==xAOD::Muon::MuidSA) passCut = true;
   }
   
   return passCut;
}
  
// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoTool::decide(std::vector<MuonEFIdperfInfo>& toolInput) const
{
   size_t numTrigger = m_ptBins.size();
   size_t numMuon=toolInput.size();
   if(numTrigger==1){
      ATH_MSG_DEBUG("Applying selection of single << " << m_decisionId);
      return inclusiveSelection(toolInput);
   }
   else{
      ATH_MSG_DEBUG("Applying selection of multiplicity "<< m_decisionId<<" with nMuons"<<numMuon);
      return multiplicitySelection(toolInput);
   }
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoTool::inclusiveSelection(std::vector<MuonEFIdperfInfo>& toolInput) const
{
   for(auto& tool : toolInput) {
      if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool.previousDecisionIDs)){
	 if(decideOnSingleObject(tool, 0)) {
	    ATH_MSG_DEBUG("Passes selection");
	    TrigCompositeUtils::addDecisionID(m_decisionId, tool.decision);
	 }
      }
      else ATH_MSG_DEBUG("Does not pass selection");
   }

   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------

StatusCode TrigMuonEFIdtpHypoTool::multiplicitySelection(std::vector<MuonEFIdperfInfo>& toolInput) const
{
   HLT::Index2DVec passingSelection(m_ptBins.size());
   for(size_t cutIndex=0; cutIndex < m_ptBins.size(); ++cutIndex) {
      size_t elementIndex{0};
      for(auto& tool : toolInput) {
	 if(TrigCompositeUtils::passed(m_decisionId.numeric(), tool.previousDecisionIDs)){
	    if(decideOnSingleObject(tool, cutIndex)){
	       ATH_MSG_DEBUG("Passing selection "<<m_decisionId << " , Index["<<elementIndex<<"]");
	       passingSelection[cutIndex].push_back(elementIndex);
	    }
	    else ATH_MSG_DEBUG("Not passing selection "<<m_decisionId << " , Index["<<elementIndex<<"]");
	 }
	 else{
	    ATH_MSG_DEBUG("No match for decisionId "<<m_decisionId);
	 }
	 elementIndex++;
      }
      //If nothing passes, then we should stop
      if(passingSelection[cutIndex].empty()){
	 ATH_MSG_DEBUG("No muons passed the selection "<<cutIndex<<" rejecting...");
	 return StatusCode::SUCCESS;
      }
   }
   std::set<size_t> passingIndices;
   HLT::elementsInUniqueCombinations(passingSelection, passingIndices);

   if(passingIndices.empty()) {
      ATH_MSG_DEBUG("No muons passed selection "<<m_decisionId);
      return StatusCode::SUCCESS;
   }
   for(auto i : passingIndices) {
      ATH_MSG_DEBUG("Muon["<<i<<"] passes "<<m_decisionId<<" with pT = "<<toolInput[i].muon->pt()/Gaudi::Units::GeV << "GeV");
      TrigCompositeUtils::addDecisionID(m_decisionId.numeric(), toolInput[i].decision);
   }

   //
   return StatusCode::SUCCESS;
}

// --------------------------------------------------------------------------------
// --------------------------------------------------------------------------------
