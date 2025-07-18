///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// METMuonAssociator.cxx 
// Implementation file for class METMuonAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METMuonAssociator.h"
#include "StoreGate/ReadDecorHandle.h"

// Muon EDM
#include "xAODMuon/MuonContainer.h"

// Tracking EDM
#include "xAODTracking/Vertex.h"

// DeltaR calculation
#include "FourMomUtils/xAODP4Helpers.h"


using MuonLink_t = ElementLink<xAOD::MuonContainer>;
using FELink_t = ElementLink<xAOD::FlowElementContainer>;

namespace met {

  using namespace xAOD;

  //accessor for PV
  const static SG::ConstAccessor<char> PVMatchedAcc("matchedToPV");

  // Constructors
  ////////////////
  METMuonAssociator::METMuonAssociator(const std::string& name) : 
    AsgTool(name),
    METAssociator(name)
  {
  }

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METMuonAssociator::initialize()
  {
    ATH_CHECK( METAssociator::initialize() );
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");
    ATH_CHECK( m_muContKey.initialize());
    if (m_useFELinks) {
      if (m_neutralFEReadDecorKey.empty()) {ATH_CHECK( m_neutralFEReadDecorKey.assign(m_muContKey.key()+"."+m_neutralFELinksKey));}
      if (m_chargedFEReadDecorKey.empty()) {ATH_CHECK( m_chargedFEReadDecorKey.assign(m_muContKey.key()+"."+m_chargedFELinksKey));}
      ATH_CHECK( m_neutralFEReadDecorKey.initialize());
      ATH_CHECK( m_chargedFEReadDecorKey.initialize());
    }

    ATH_CHECK(m_elementLinkName.initialize(m_doMuonClusterMatch));

    return StatusCode::SUCCESS;
  }

  // executeTool
  ////////////////
  StatusCode METMuonAssociator::executeTool(xAOD::MissingETContainer* /*metCont*/, xAOD::MissingETAssociationMap* metMap) const
  {
    ATH_MSG_VERBOSE ("In execute: " << name() << "...");

    SG::ReadHandle<xAOD::MuonContainer> muonCont(m_muContKey);
    if (!muonCont.isValid()) {
      ATH_MSG_WARNING("Unable to retrieve input muon container " << m_muContKey.key());
      return StatusCode::FAILURE;
    }

    ATH_MSG_DEBUG("Successfully retrieved muon collection");
    if (fillAssocMap(metMap,muonCont.cptr()).isFailure()) {
      ATH_MSG_WARNING("Unable to fill map with muon container " << m_muContKey.key());
      return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
  }

  // *********************************************************************************************************
  // Get constituents
  StatusCode METMuonAssociator::extractTopoClusters(const xAOD::IParticle* obj,
                                                    std::vector<const xAOD::IParticle*>& tclist,
                                                    const met::METAssociator::ConstitHolder& /*constits*/) const
  {
    const xAOD::Muon *mu = static_cast<const xAOD::Muon*>(obj);
    const CaloCluster* muclus = mu->cluster();
    if(muclus && m_doMuonClusterMatch) {
      ATH_MSG_VERBOSE("Muon " << mu->index() << " with pt " << mu->pt()
                   << ", eta "   << mu->eta()
                   << ", phi " << mu->phi()
                   << " has cluster with "
                   << "eta "   << muclus->calEta()
                   << ", phi " << muclus->calPhi()
                   << ", E "   << muclus->calE()
                   << " formed of " << muclus->size() << " cells.");
      ATH_MSG_VERBOSE("Muon Eloss type: " << mu->energyLossType()
                   << " Eloss: " << mu->floatParameter(xAOD::Muon::EnergyLoss)
                   << " MeasuredEloss: " << mu->floatParameter(xAOD::Muon::MeasEnergyLoss)
                   << " FSR E: " << mu->floatParameter(xAOD::Muon::FSR_CandidateEnergy) );
      
      SG::ReadDecorHandle<CaloClusterContainer, std::vector<ElementLink<CaloClusterContainer> > > tcLinkAcc(m_elementLinkName); 
      for(const auto& matchel : tcLinkAcc(*muclus)) {
        if(!matchel.isValid()) {continue;} // In case of thinned cluster collection
        ATH_MSG_VERBOSE("Tool found cluster " << (*matchel)->index() << " with pt " << (*matchel)->pt() );
        if((*matchel)->e()>1e-9) { // +ve E
          tclist.push_back(*matchel);
        }
      }
    } // muon has linked cluster
    
    return StatusCode::SUCCESS;
  }

  StatusCode METMuonAssociator::extractTracks(const xAOD::IParticle *obj,
                                              std::vector<const xAOD::IParticle*>& constlist,
                                              const met::METAssociator::ConstitHolder& constits) const
  {
    const xAOD::Muon *mu = static_cast<const xAOD::Muon*>(obj);
    const TrackParticle* idtrack = mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
    if(idtrack && acceptTrack(idtrack,constits.pv) && isGoodEoverP(idtrack)) {
    // if(idtrack && acceptTrack(idtrack,pv)) {
      ATH_MSG_VERBOSE("Accept muon track " << idtrack << " px, py = " << idtrack->p4().Px() << ", " << idtrack->p4().Py());
      ATH_MSG_VERBOSE("Muon ID track ptr: " << idtrack);
      constlist.push_back(idtrack);
      // if(mu->pt()>10e3 && (mu->muonType()==xAOD::Muon::Combined || mu->muonType()==xAOD::Muon::SegmentTagged)) {
      //   mutracks.push_back(idtrack);
      // }
    }
    return StatusCode::SUCCESS;
  }

  // *********************************************************************************************************
  // Get constituents
  StatusCode METMuonAssociator::extractPFO(const xAOD::IParticle* obj,
                                           std::vector<const xAOD::IParticle*>& pfolist,
                                           const met::METAssociator::ConstitHolder& constits,
                                           std::map<const IParticle*,MissingETBase::Types::constvec_t>& /*momenta*/) const
  {
    const xAOD::Muon *mu = static_cast<const xAOD::Muon*>(obj);
    const TrackParticle* idtrack = mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
    const CaloCluster* muclus = mu->cluster();

    ATH_MSG_VERBOSE("Muon " << mu->index() << " with pt " << mu->pt()
                    << ", eta "   << mu->eta()
                    << ", phi " << mu->phi());
    if(muclus) {
      ATH_MSG_VERBOSE(" has cluster with "
                      << "eta "   << muclus->calEta()
                      << ", phi " << muclus->calPhi()
                      << ", E "   << muclus->calE()
                      << " formed of " << muclus->size() << " cells.");
    }
    ATH_MSG_VERBOSE("Muon Eloss type: " << mu->energyLossType()
                    << " Eloss: " << mu->floatParameter(xAOD::Muon::EnergyLoss)
                    << " MeasuredEloss: " << mu->floatParameter(xAOD::Muon::MeasEnergyLoss)
                    << " FSR E: " << mu->floatParameter(xAOD::Muon::FSR_CandidateEnergy) );

    // One loop over PFOs
    for(const auto *const pfo : *constits.pfoCont) {
      if(pfo->isCharged()) {
        // get charged PFOs by matching the muon ID track
        // We set a small -ve pt for cPFOs that were rejected
        // by the ChargedHadronSubtractionTool
        const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
        if(idtrack && pfo->track(0) == idtrack && PVMatchedAcc(*pfo) &&
           ( !m_cleanChargedPFO || isGoodEoverP(pfo->track(0)) )
           ) {
          ATH_MSG_VERBOSE("Accept muon PFO " << pfo << " px, py = " << pfo->p4().Px() << ", " << pfo->p4().Py());
          ATH_MSG_VERBOSE("Muon PFO index: " << pfo->index() << ", pt: " << pfo->pt() << ", eta: " << pfo->eta() << ", phi: " << pfo->phi() );
          ATH_MSG_VERBOSE("Muon ID Track index: " << idtrack->index() << ", pt: " << idtrack->pt() << ", eta: " << idtrack->eta() << ", phi: " << idtrack->phi() );
          pfolist.push_back(pfo);
          break;
        } // track match
      } else {
        // get neutral PFOs by matching the muon cluster
        if(muclus && m_doMuonClusterMatch) {

          SG::ReadDecorHandle<CaloClusterContainer, std::vector<ElementLink<CaloClusterContainer> > > tcLinkAcc(m_elementLinkName);
                for(const auto& matchel : tcLinkAcc(*muclus)) {
            if(!matchel.isValid()) {
              ATH_MSG_DEBUG("Invalid muon-cluster elementLink");
            } else {
              if((*matchel)->e()>FLT_MIN && pfo->cluster(0) == *matchel) { // +ve E && matches cluster
                ATH_MSG_VERBOSE("Tool found cluster " << (*matchel)->index() << " with pt " << (*matchel)->pt() );
                pfolist.push_back(pfo);
              }
            }
          }
        } // muon has linked cluster
      } 
    } // end of cluster loop

    return StatusCode::SUCCESS;
  }


  StatusCode METMuonAssociator::extractFE(const xAOD::IParticle* obj,
                                            std::vector<const xAOD::IParticle*>& felist,
                                            const met::METAssociator::ConstitHolder& constits,
                                            std::map<const IParticle*,MissingETBase::Types::constvec_t> &/*momenta*/) const
  {
    const xAOD::Muon *mu = static_cast<const xAOD::Muon*>(obj);
    if (m_useFELinks)
      ATH_CHECK( extractFEsFromLinks(mu, felist,constits) );
    else
      ATH_CHECK( extractFEs(mu, felist, constits) );

    return StatusCode::SUCCESS;
  }

  StatusCode METMuonAssociator::extractFEsFromLinks(const xAOD::Muon* mu, //TODO to be tested
						       std::vector<const xAOD::IParticle*>& felist,
						       const met::METAssociator::ConstitHolder& constits) const
  {
    ATH_MSG_DEBUG("Extract FEs From Links for " << mu->type()  << " with pT " << mu->pt());

    std::vector<FELink_t> nFELinks;
    std::vector<FELink_t> cFELinks;

    SG::ReadDecorHandle<xAOD::MuonContainer, std::vector<FELink_t> > neutralFEReadDecorHandle (m_neutralFEReadDecorKey);
    SG::ReadDecorHandle<xAOD::MuonContainer, std::vector<FELink_t> > chargedFEReadDecorHandle (m_chargedFEReadDecorKey);
    nFELinks=neutralFEReadDecorHandle(*mu);
    cFELinks=chargedFEReadDecorHandle(*mu);

    // Charged FEs
    for (const FELink_t& feLink : cFELinks) {
      if (!feLink.isValid()) continue;
      const xAOD::FlowElement* fe_init = *feLink;
      for (const auto *const fe : *constits.feCont){
        if (fe->index() == fe_init->index() && fe->isCharged()){ //index-based match between JetETmiss and CHSFlowElements collections
          const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
          if(  fe->isCharged() && PVMatchedAcc(*fe)&& ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(fe->chargedObject(0))) ) ) {
            ATH_MSG_DEBUG("Accept cFE with pt " << fe->pt() << ", e " << fe->e() << ", eta " << fe->eta() << ", phi " << fe->phi() );
            felist.push_back(fe);
          }
        }
      }
    } // end cFE loop

    // Neutral FEs
    for (const FELink_t& feLink : nFELinks) {
      if (!feLink.isValid()) continue;
      const xAOD::FlowElement* fe_init = *feLink;
      for (const auto *const fe : *constits.feCont){
        if (fe->index() == fe_init->index() && !fe->isCharged()){ //index-based match between JetETmiss and CHSFlowElements collections
          if( ( !fe->isCharged()&& fe->e() > FLT_MIN ) ){
            ATH_MSG_DEBUG("Accept nFE with pt " << fe->pt() << ", e " << fe->e() << ", eta " << fe->eta() << ", phi " << fe->phi() << " in sum.");
            felist.push_back(fe);
          }
        }
      }
    } // end nFE links loop


    return StatusCode::SUCCESS;
  }

  StatusCode METMuonAssociator::extractFEs(const xAOD::Muon* mu,
				 std::vector<const xAOD::IParticle*>& felist,
				 const met::METAssociator::ConstitHolder& constits) const
  {
    const TrackParticle* idtrack = mu->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
    const CaloCluster* muclus = mu->cluster();
    ATH_MSG_VERBOSE("Muon " << mu->index() << " with pt " << mu->pt()
                    << ", eta "   << mu->eta()
                    << ", phi " << mu->phi());
    if(muclus) {
      ATH_MSG_VERBOSE(" has cluster with "
                      << "eta "   << muclus->calEta()
                      << ", phi " << muclus->calPhi()
                      << ", E "   << muclus->calE()
                      << " formed of " << muclus->size() << " cells.");
    }
    ATH_MSG_VERBOSE("Muon Eloss type: " << mu->energyLossType()
                    << " Eloss: " << mu->floatParameter(xAOD::Muon::EnergyLoss)
                    << " MeasuredEloss: " << mu->floatParameter(xAOD::Muon::MeasEnergyLoss)
                    << " FSR E: " << mu->floatParameter(xAOD::Muon::FSR_CandidateEnergy) );

    // One loop over PFOs
    for(const xAOD::FlowElement* fe : *constits.feCont) {
      if(fe->isCharged()) {
        // get charged FEs by matching the muon ID track
        // We set a small -ve pt for cPFOs that were rejected
        // by the ChargedHadronSubtractionTool
        const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
        if(idtrack && fe->chargedObject(0) == idtrack && PVMatchedAcc(*fe) &&
           ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(fe->chargedObject(0))) )
           ) {
          ATH_MSG_VERBOSE("Accept muon PFO (FE) " << fe << " px, py = " << fe->p4().Px() << ", " << fe->p4().Py());
          ATH_MSG_VERBOSE("Muon PFO index: " << fe->index() << ", pt: " << fe->pt() << ", eta: " << fe->eta() << ", phi: " << fe->phi() );
          ATH_MSG_VERBOSE("Muon ID Track index: " << idtrack->index() << ", pt: " << idtrack->pt() << ", eta: " << idtrack->eta() << ", phi: " << idtrack->phi() );
          felist.push_back(fe);
          break;
        } // track match
      } else {
        // get neutral PFOs by matching the muon cluster
        if(muclus && m_doMuonClusterMatch) {

          SG::ReadDecorHandle<CaloClusterContainer, std::vector<ElementLink<CaloClusterContainer> > > tcLinkAcc(m_elementLinkName); 
          for(const auto& matchel : tcLinkAcc(*muclus)) {
            if(!matchel.isValid()) {
              ATH_MSG_DEBUG("Invalid muon-cluster elementLink");
            } else {
              if((*matchel)->e()>FLT_MIN && fe->otherObject(0) == *matchel) { // +ve E && matches cluster
                ATH_MSG_VERBOSE("Tool found cluster " << (*matchel)->index() << " with pt " << (*matchel)->pt() );
                felist.push_back(fe);
              }
            }
          }
        } // muon has linked cluster
      }
    } // end of cluster loop

    return StatusCode::SUCCESS;
  }

  // add HR implementation from release 21.2
  // extractFE for W precision-type measurements
  StatusCode METMuonAssociator::extractFEHR(const xAOD::IParticle* obj,
                                            std::vector<const xAOD::IParticle*> hardObjs,
                                            std::vector<const xAOD::IParticle*>& felist,
                                            const met::METAssociator::ConstitHolder& constits,
                                            std::map<const IParticle*,MissingETBase::Types::constvec_t> & /*momenta*/,
                                            float& UEcorr) const
  {
    if(obj->type() != xAOD::Type::ObjectType::Muon){
      UEcorr=0.0;
      felist={};
      return StatusCode::SUCCESS;
    }
    const xAOD::Muon* mu = static_cast<const xAOD::Muon*>(obj);

    // Get PFOs associated to muons
    for(const auto& fe : *constits.feCont) {
      if( fe->isCharged()) { // Fill list with charged PFOs (using muon tracks)
        if( mu && P4Helpers::isInDeltaR(*fe, *mu, m_Drcone, m_useRapidity) && PVMatchedAcc(*fe) &&
          ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(fe->chargedObject(0))) ) ){
          felist.push_back(fe);
        }
      }
      else{ // Fill list with neutral PFOs (using muon clusters)
        if( mu && P4Helpers::isInDeltaR(*fe, *mu, m_Drcone, m_useRapidity) ){
          felist.push_back(fe);
        }
      } // neutral PFO condition
    } // loop over all PFOs

    // Calculating UE energy correction for a given lepton (using mu)
    if(mu){
      // Vectoral sum of all FE
      TLorentzVector HR;  // uncorrected HR (initialized with 0,0,0,0 automatically)
      for(const auto& fe_itr : *constits.feCont) {
        if( fe_itr->pt() < 0 || fe_itr->e() < 0 ) { // sanity check
          continue;
        }

        //remove charged FE that are not matched to the PV
        if(fe_itr->isCharged() && !PVMatchedAcc(*fe_itr)){
          continue;
        }
        HR += fe_itr->p4();
      }

      // Create vectors of muons
      std::vector<const xAOD::Muon*> v_mu;
      for(const auto& obj_i : hardObjs) {
        if(obj_i->pt()<5e3 && obj_i->type() != xAOD::Type::Muon) { // sanity check
          continue;
        }
        const xAOD::Muon* mu_curr = static_cast<const xAOD::Muon*>(obj_i); // current muon
        v_mu.push_back(mu_curr);
      }


      // Subtracting PFOs matched to muons from HR
      for(const auto& fe_i : *constits.feCont) { // charged and neutral PFOs
        if( fe_i->pt() < 0 || fe_i->e() < 0 ) { // sanity check
          continue;
        }
        for(const auto& mu_i : v_mu) { // loop over muons
          double dR = P4Helpers::deltaR( fe_i->eta(), fe_i->phi(), mu_i->eta(), mu_i->phi() );
          if( dR < m_Drcone ) { // if PFO is in a cone around muon
            HR -= fe_i->p4();
            break;
          } // cone requirement
        } // over v_mu
      } // over PFOs

      // Save v_mu as a vector TLV (as commonn type for electrons and muons)
      std::vector<TLorentzVector> v_muTLV;
      v_muTLV.reserve(v_mu.size());
      for(const auto& mu_i : v_mu) { // loop over v_mu
        v_muTLV.push_back( mu_i->p4() );
      }

      // Save current mu as TLV
      TLorentzVector muTLV = mu->p4();

      // Get UE correction
      ATH_CHECK( GetUEcorr(constits, v_muTLV, muTLV, HR, m_Drcone, m_MinDistCone, UEcorr) );
    } // available mu requirement


    return StatusCode::SUCCESS;
  }
}
