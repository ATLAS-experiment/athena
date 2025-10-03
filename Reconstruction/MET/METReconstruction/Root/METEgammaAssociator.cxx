///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

// METEgammaAssociator.cxx 
// Implementation file for class METEgammaAssociator
//
//  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
//
// Author: P Loch, S Resconi, TJ Khoo, AS Mete
/////////////////////////////////////////////////////////////////// 

// METReconstruction includes
#include "METReconstruction/METEgammaAssociator.h"

// Egamma EDM
#include "xAODEgamma/ElectronContainer.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODEgamma/EgammaxAODHelpers.h"

// Tracking EDM
#include "xAODTracking/Vertex.h"

// DeltaR calculation
#include "FourMomUtils/xAODP4Helpers.h"

using ElectronLink_t = ElementLink<xAOD::ElectronContainer>;
using PhotonLink_t = ElementLink<xAOD::PhotonContainer>;
using PFOLink_t = ElementLink<xAOD::PFOContainer>;
using FELink_t = ElementLink<xAOD::FlowElementContainer>;

namespace met {

  using namespace xAOD;

  //accessor for PV
  const static SG::ConstAccessor<char> PVMatchedAcc("matchedToPV");

  // Constructors
  ////////////////
  METEgammaAssociator::METEgammaAssociator(const std::string& name) : 
    AsgTool(name),
    METAssociator(name)
  {
    // 0 is delta-R geometrical matching
    // 1 is using TopoClusterLink decoration on clusters
    declareProperty( "TCMatchMethod",     m_tcMatch_method = DeltaR );
    declareProperty( "TCMatchMaxRat",     m_tcMatch_maxRat = 1.5    );
    declareProperty( "TCMatchDeltaR",     m_tcMatch_dR     = 0.1    );
    declareProperty( "ExtraTrackMatchDeltaR", m_extraTrkMatch_dR = 0.05 );
    declareProperty( "CheckUnmatched", m_checkUnmatched = false); 
  }

  // Destructor
  ///////////////
  METEgammaAssociator::~METEgammaAssociator()
  = default;

  // Athena algtool's Hooks
  ////////////////////////////
  StatusCode METEgammaAssociator::initialize()
  {
    ATH_CHECK( METAssociator::initialize() );
    ATH_MSG_VERBOSE ("Initializing " << name() << "...");
    switch(m_tcMatch_method) {
    case DeltaR: ATH_MSG_INFO("Egamma-topocluster matching uses DeltaR check."); break;
    case ClusterLink: ATH_MSG_INFO("Egamma-topocluster matching uses topocluster links."); break;
    default:
      ATH_MSG_WARNING( "Invalid topocluster match method configured!" );
      return StatusCode::FAILURE;
    } 

    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::finalize()
  {
    ATH_MSG_VERBOSE ("Finalizing " << name() << "...");
    return StatusCode::SUCCESS;
  }


  // **********************************************************************
  // Get Egamma constituents
  StatusCode METEgammaAssociator::extractTopoClusters(const xAOD::IParticle* obj,
                                                      std::vector<const xAOD::IParticle*>& tclist,
                                                      const met::METAssociator::ConstitHolder& constits) const
  {
    const Egamma *eg = static_cast<const Egamma*>(obj);
    // safe to assume a single SW cluster?
    // will do so for now...
    const CaloCluster* swclus = eg->caloCluster();

    // the matching strategy depends on how the cluster container is sorted
    // easier if it's sorted in descending pt order
    // we'll worry about optimisation later
    std::vector<const IParticle*> inputTC;
    inputTC.reserve(10);
    
    if(m_tcMatch_method==DeltaR) {
      for(const auto *const cl : *constits.tcCont) {
        // this can probably be done more elegantly
        if(P4Helpers::isInDeltaR(*swclus,*cl,m_tcMatch_dR,m_useRapidity) && cl->e()>FLT_MIN) {
          // could consider also requirements on the EM fraction or depth
          inputTC.push_back(cl);
        } // match TC in a cone around SW cluster
      }
      ATH_MSG_VERBOSE("Found " << inputTC.size() << " nearby topoclusters");
      std::sort(inputTC.begin(),inputTC.end(),greaterPt);
    } else if(m_tcMatch_method==ClusterLink) {
      static const SG::AuxElement::ConstAccessor<std::vector<ElementLink<CaloClusterContainer> > > tcLinkAcc("constituentClusterLinks");
      // Fill a vector of vectors
      for(const auto& el : tcLinkAcc(*swclus)) {
        if(el.isValid())
          inputTC.push_back(*el);
        else{
          ATH_MSG_ERROR("Invalid constituentClusterLinks on input electron/photon!");
          return StatusCode::FAILURE;
        }
      }
      ATH_MSG_VERBOSE("Found " << inputTC.size() << " linked topoclusters");
    } else {
      ATH_MSG_WARNING( "Invalid topocluster match method configured!" );
      return StatusCode::FAILURE;
    } 

    ATH_CHECK( selectEgammaClusters(swclus, inputTC, tclist) );

    return StatusCode::SUCCESS;
  }


  StatusCode METEgammaAssociator::extractTracks(const xAOD::IParticle* obj,
                                                std::vector<const xAOD::IParticle*>& constlist,
                                                const met::METAssociator::ConstitHolder& constits) const
  {
    const xAOD::Egamma *eg = static_cast<const xAOD::Egamma*>(obj);
    std::set<const xAOD::TrackParticle*> trackset; // use a set for duplicate-free retrieval
    ATH_CHECK( selectEgammaTracks(eg, constits.trkCont, trackset) );
    for(const auto& track : trackset) {
      if(acceptTrack(track,constits.pv) && isGoodEoverP(track)) {
        constlist.push_back(track);
      }
    }
    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::extractPFO(const xAOD::IParticle* obj,
                                             std::vector<const xAOD::IParticle*>& pfolist,
                                             const met::METAssociator::ConstitHolder& constits,
                                             std::map<const IParticle*,MissingETBase::Types::constvec_t> &/*momenta*/) const
  {
    const xAOD::Egamma *eg = static_cast<const xAOD::Egamma*>(obj);

    if (m_usePFOLinks)
      ATH_CHECK( extractPFOsFromLinks(eg, pfolist,constits) );
    else
      ATH_CHECK( extractPFOs(eg, pfolist, constits) );

    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::extractPFOsFromLinks(const xAOD::Egamma* eg,
						       std::vector<const xAOD::IParticle*>& pfolist,
						       const met::METAssociator::ConstitHolder& constits) const
  {

    ATH_MSG_DEBUG("Extract PFOs From Links for " << eg->type()  << " with pT " << eg->pt());

    std::vector<PFOLink_t> cPFOLinks;
    std::vector<PFOLink_t> nPFOLinks;

    if (eg->type() == xAOD::Type::Electron){
      SG::ReadDecorHandle<xAOD::ElectronContainer, std::vector<PFOLink_t> > neutralPFOReadDecorHandle (m_electronNeutralPFOReadDecorKey);
      SG::ReadDecorHandle<xAOD::ElectronContainer, std::vector<PFOLink_t> > chargedPFOReadDecorHandle (m_electronChargedPFOReadDecorKey);
      nPFOLinks=neutralPFOReadDecorHandle(*eg);
      cPFOLinks=chargedPFOReadDecorHandle(*eg);
    }
    if (eg->type() == xAOD::Type::Photon) {
      SG::ReadDecorHandle<xAOD::PhotonContainer, std::vector<PFOLink_t> > neutralPFOReadDecorHandle (m_photonNeutralPFOReadDecorKey);
      SG::ReadDecorHandle<xAOD::PhotonContainer, std::vector<PFOLink_t> > chargedPFOReadDecorHandle (m_photonChargedPFOReadDecorKey);
      nPFOLinks=neutralPFOReadDecorHandle(*eg);
      cPFOLinks=chargedPFOReadDecorHandle(*eg);
    }


    // Charged PFOs
    for (const PFOLink_t& pfoLink : cPFOLinks) {
      if (!pfoLink.isValid()) continue;
      const xAOD::PFO* pfo_init = *pfoLink;
      for (const auto *const pfo : *constits.pfoCont){
        if (pfo->index() == pfo_init->index() && pfo->isCharged()){ //index-based match between JetETmiss and CHSParticleFlow collections
          const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");
          if(  pfo->isCharged() && PVMatchedAcc(*pfo)&& ( !m_cleanChargedPFO || isGoodEoverP(pfo->track(0)) ) ) {
            ATH_MSG_DEBUG("Accept cPFO with pt " << pfo->pt() << ", e " << pfo->e() << ", eta " << pfo->eta() << ", phi " << pfo->phi() );
            if (!m_checkUnmatched || !hasUnmatchedClusters(eg,pfo_init)) pfolist.push_back(pfo); 
          }
        } 
      }
    } // end cPFO loop

    // Neutral PFOs
    double eg_cl_e = eg->caloCluster()->e();
    double sumE_pfo = 0.;

    for (const PFOLink_t& pfoLink : nPFOLinks) {
      if (!pfoLink.isValid()) continue;
      const xAOD::PFO* pfo_init = *pfoLink;
      for (const auto *const pfo : *constits.pfoCont){
        if (pfo->index() == pfo_init->index() && !pfo->isCharged()){ //index-based match between JetETmiss and CHSParticleFlow collections
          double pfo_e = pfo->eEM();
          if( ( !pfo->isCharged()&& pfo->e() > FLT_MIN ) ){   
            sumE_pfo += pfo_e;
            ATH_MSG_DEBUG("E match with new nPFO: " << fabs(sumE_pfo+pfo_e - eg_cl_e) / eg_cl_e);
            ATH_MSG_DEBUG("Accept nPFO with pt " << pfo->pt() << ", e " << pfo->e() << ", eta " << pfo->eta() << ", phi " << pfo->phi() << " in sum.");
            ATH_MSG_DEBUG("Energy ratio of nPFO to eg: " << pfo_e / eg_cl_e);
            pfolist.push_back(pfo);
          }   
        }
      }
    } // end nPFO links loop


    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::extractPFOs(const xAOD::Egamma* eg,
						    std::vector<const xAOD::IParticle*>& pfolist,
						    const met::METAssociator::ConstitHolder& constits) const

  {
    // safe to assume a single SW cluster?
    // will do so for now...
    const xAOD::IParticle* swclus = eg->caloCluster();
    ANA_MSG_VERBOSE("Extract PFOs with DeltaR for " << eg->type()  << " with pT " << eg->pt());

    // Preselect PFOs based on proximity: dR<0.4
    std::vector<const xAOD::PFO*> nearbyPFO;
    nearbyPFO.reserve(20);
    for(const auto *const pfo : *constits.pfoCont) {
      if(P4Helpers::isInDeltaR(*pfo, *swclus, 0.4, m_useRapidity)) {
        // We set a small -ve pt for cPFOs that were rejected
        // by the ChargedHadronSubtractionTool
        const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");        
        if( ( !pfo->isCharged() && pfo->e() > FLT_MIN ) ||
            ( pfo->isCharged() && PVMatchedAcc(*pfo)
              && ( !m_cleanChargedPFO || isGoodEoverP(pfo->track(0)) ) )
            ) {
          nearbyPFO.push_back(pfo);
        } // retain +ve E neutral PFOs and charged PFOs passing PV association
      } // DeltaR check
    } // PFO loop
    ATH_MSG_VERBOSE("Found " << nearbyPFO.size() << " nearby pfos");

    std::set<const xAOD::TrackParticle*> trackset; // use a set for duplicate-free retrieval
    ATH_CHECK( selectEgammaTracks(eg, constits.trkCont, trackset) );
    for(const auto& track : trackset) {
      for(const auto& pfo : nearbyPFO) {
        if(pfo->isCharged() && pfo->track(0) == track) {
          pfolist.push_back(pfo);
        } // PFO/track match
      } // PFO loop
    } // Track loop
    double eg_cl_e = swclus->e();

    // the matching strategy depends on how the cluster container is sorted
    // easier if it's sorted in descending pt order
    // ideally this should be done using cell matching, but we can't use the links from topoclusters reliably
    // because some PFOs don't correspond to the original TC
    bool doSum = true;
    double sumE_pfo = 0.;
    const IParticle* bestbadmatch = nullptr;
    std::sort(nearbyPFO.begin(),nearbyPFO.end(),greaterPtPFO);
    for(const auto& pfo : nearbyPFO) {
      // Skip charged PFOs, as we already matched them
      if(pfo->isCharged() || !P4Helpers::isInDeltaR(*pfo, *swclus, m_tcMatch_dR, m_useRapidity)) {continue;}
      // Handle neutral PFOs like topoclusters
      double pfo_e = pfo->eEM();
      // skip cluster if it's above our bad match threshold or outside the matching radius
      if(pfo_e>m_tcMatch_maxRat*eg_cl_e) {
        ATH_MSG_VERBOSE("Reject topocluster in sum. Ratio vs eg cluster: " << (pfo_e/eg_cl_e));
            if( !bestbadmatch || (fabs(pfo_e/eg_cl_e-1.) < fabs(bestbadmatch->e()/eg_cl_e-1.)) ) bestbadmatch = pfo;
        continue;
      }

      ATH_MSG_VERBOSE("E match with new nPFO: " << fabs(sumE_pfo+pfo_e - eg_cl_e) / eg_cl_e);
      if( (doSum = fabs(sumE_pfo+pfo_e-eg_cl_e) < fabs(sumE_pfo - eg_cl_e)) ) {
        pfolist.push_back(pfo);
        sumE_pfo += pfo_e;
            ATH_MSG_VERBOSE("Accept pfo with pt " << pfo->pt() << ", e " << pfo->e() << " in sum.");
            ATH_MSG_VERBOSE("Energy ratio of nPFO to eg: " << pfo_e / eg_cl_e);
            ATH_MSG_VERBOSE("E match with new PFO: " << fabs(sumE_pfo+pfo_e - eg_cl_e) / eg_cl_e);
      } // if we will retain the topocluster
      else {break;}
    } // loop over nearby clusters
    if(sumE_pfo<FLT_MIN && bestbadmatch) {
      ATH_MSG_VERBOSE("No better matches found -- add bad match topocluster with pt "
                          << bestbadmatch->pt() << ", e " << bestbadmatch->e() << ".");
      pfolist.push_back(bestbadmatch);
    }

    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::extractFE(const xAOD::IParticle* obj, 
                                            std::vector<const xAOD::IParticle*>& felist,
                                            const met::METAssociator::ConstitHolder& constits,
                                            std::map<const IParticle*,MissingETBase::Types::constvec_t> &/*momenta*/) const
  {
    const xAOD::Egamma *eg = static_cast<const xAOD::Egamma*>(obj);

    if (m_useFELinks)
      ATH_CHECK( extractFEsFromLinks(eg, felist,constits) );
    else
      ATH_CHECK( extractFEs(eg, felist, constits) );

    return StatusCode::SUCCESS;
  }


  StatusCode METEgammaAssociator::extractFEsFromLinks(const xAOD::Egamma* eg, // TODO: to be tested
						       std::vector<const xAOD::IParticle*>& felist,
						       const met::METAssociator::ConstitHolder& constits) const
  {

    ATH_MSG_DEBUG("Extract FEs From Links for " << eg->type()  << " with pT " << eg->pt());

    std::vector<FELink_t> nFELinks;
    std::vector<FELink_t> cFELinks;

    if (eg->type() == xAOD::Type::Electron){
      SG::ReadDecorHandle<xAOD::ElectronContainer, std::vector<FELink_t> > neutralFEReadDecorHandle (m_electronNeutralFEReadDecorKey);
      SG::ReadDecorHandle<xAOD::ElectronContainer, std::vector<FELink_t> > chargedFEReadDecorHandle (m_electronChargedFEReadDecorKey);
      nFELinks=neutralFEReadDecorHandle(*eg);
      cFELinks=chargedFEReadDecorHandle(*eg);
    }
    if (eg->type() == xAOD::Type::Photon) {
      SG::ReadDecorHandle<xAOD::PhotonContainer, std::vector<FELink_t> > neutralFEReadDecorHandle (m_photonNeutralFEReadDecorKey);
      SG::ReadDecorHandle<xAOD::PhotonContainer, std::vector<FELink_t> > chargedFEReadDecorHandle (m_photonChargedFEReadDecorKey);
      nFELinks=neutralFEReadDecorHandle(*eg);
      cFELinks=chargedFEReadDecorHandle(*eg);
    }


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
    double eg_cl_e = eg->caloCluster()->e();
    double sumE_fe = 0.;

    for (const FELink_t& feLink : nFELinks) {
      if (!feLink.isValid()) continue;
      const xAOD::FlowElement* fe_init = *feLink;
      for (const auto *const fe : *constits.feCont){
        if (fe->index() == fe_init->index() && !fe->isCharged()){ //index-based match between JetETmiss and CHSFlowElements collections
          double fe_e = fe->e();
          if( ( !fe->isCharged()&& fe->e() > FLT_MIN ) ){   
            sumE_fe += fe_e;
            ATH_MSG_DEBUG("E match with new nFE: " << fabs(sumE_fe+fe_e - eg_cl_e) / eg_cl_e);
            ATH_MSG_DEBUG("Accept nFE with pt " << fe->pt() << ", e " << fe->e() << ", eta " << fe->eta() << ", phi " << fe->phi() << " in sum.");
            ATH_MSG_DEBUG("Energy ratio of nFE to eg: " << fe_e / eg_cl_e);
            felist.push_back(fe);
          }   
        }
      }
    } // end nFE links loop


    return StatusCode::SUCCESS;
  }

 StatusCode METEgammaAssociator::extractFEs(const xAOD::Egamma* eg,
                                            std::vector<const xAOD::IParticle*>& felist,
                                            const met::METAssociator::ConstitHolder& constits) const
  {
    ATH_MSG_VERBOSE("Extract FEs From DeltaR for " << eg->type()  << " with pT " << eg->pt());

    // safe to assume a single SW cluster?
    // will do so for now...
    const xAOD::IParticle* swclus = eg->caloCluster();

    // Preselect PFOs based on proximity: dR<0.4
    std::vector<const xAOD::FlowElement*> nearbyFE;
    nearbyFE.reserve(20);
    for(const xAOD::FlowElement* fe : *constits.feCont) {
      if(!(fe->signalType() & xAOD::FlowElement::PFlow)){
        ATH_MSG_ERROR("Attempted to extract non-PFlow FlowElements. This is not supported!");
        return StatusCode::FAILURE;
      }
      if(P4Helpers::isInDeltaR(*fe, *swclus, 0.4, m_useRapidity)) {
        // We set a small -ve pt for cPFOs that were rejected
        // by the ChargedHadronSubtractionTool
        const static SG::AuxElement::ConstAccessor<char> PVMatchedAcc("matchedToPV");        
        if( ( !fe->isCharged() && fe->e() > FLT_MIN ) ||
            ( fe->isCharged() && PVMatchedAcc(*fe)
              && ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(fe->chargedObject(0))) ) )
            ) {
          nearbyFE.push_back(fe);
        } // retain +ve E neutral PFOs and charged PFOs passing PV association
      } // DeltaR check
    } // PFO loop
    ATH_MSG_VERBOSE("Found " << nearbyFE.size() << " nearby FlowElements (PFOs)");

    std::set<const xAOD::TrackParticle*> trackset; // use a set for duplicate-free retrieval
    ATH_CHECK( selectEgammaTracks(eg, constits.trkCont, trackset) );
    for(const xAOD::TrackParticle* track : trackset) {
      for(const xAOD::FlowElement* fe : nearbyFE) {
        if(fe->isCharged() && fe->chargedObject(0) == track) {
          felist.push_back(fe);
        } // PFO/track match
      } // PFO loop
    } // Track loop
    double eg_cl_e = swclus->e();

    // the matching strategy depends on how the cluster container is sorted
    // easier if it's sorted in descending pt order
    // ideally this should be done using cell matching, but we can't use the links from topoclusters reliably
    // because some PFOs don't correspond to the original TC
    bool doSum = true;
    double sumE_pfo = 0.;
    const IParticle* bestbadmatch = nullptr;
    std::sort(nearbyFE.begin(),nearbyFE.end(),greaterPtFE);
    for(const xAOD::FlowElement* fe : nearbyFE) {
      // Skip charged PFOs, as we already matched them
      if(fe->isCharged() || !P4Helpers::isInDeltaR(*fe, *swclus, m_tcMatch_dR, m_useRapidity)) continue;
      // Handle neutral PFOs like topoclusters
      // TODO: Use EM-scale energy here in the future? No way to access from FlowElement in general.
      double pfo_e = fe->e();
      // skip cluster if it's above our bad match threshold or outside the matching radius
      if(pfo_e > m_tcMatch_maxRat*eg_cl_e) {
        ATH_MSG_VERBOSE("Reject topocluster in sum. Ratio vs eg cluster: " << (pfo_e/eg_cl_e));
        if( !bestbadmatch || (fabs(pfo_e/eg_cl_e-1.) < fabs(bestbadmatch->e()/eg_cl_e-1.)) ) bestbadmatch = fe;
        continue;
      }

      ATH_MSG_VERBOSE("E match with new nPFO: " << fabs(sumE_pfo+pfo_e - eg_cl_e) / eg_cl_e);
      if( (doSum = fabs(sumE_pfo+pfo_e-eg_cl_e) < fabs(sumE_pfo - eg_cl_e)) ) {
        felist.push_back(fe);
        sumE_pfo += pfo_e;
        ATH_MSG_VERBOSE("Accept pfo with pt " << fe->pt() << ", e " << fe->e() << " in sum.");
        ATH_MSG_VERBOSE("Energy ratio of nPFO to eg: " << pfo_e / eg_cl_e);
        ATH_MSG_VERBOSE("E match with new PFO: " << fabs(sumE_pfo+pfo_e - eg_cl_e) / eg_cl_e);
      } // if we will retain the topocluster
      else break;
    } // loop over nearby clusters
    if(sumE_pfo<FLT_MIN && bestbadmatch) {
      ATH_MSG_VERBOSE("No better matches found -- add bad match topocluster with pt "
                          << bestbadmatch->pt() << ", e " << bestbadmatch->e() << ".");
      felist.push_back(bestbadmatch);
    }

    return StatusCode::SUCCESS;
  }


  // add HR implementation from release 21.2
  // extractFE for W precision-type measurements
  StatusCode METEgammaAssociator::extractFEHR(const xAOD::IParticle* obj,
                                              std::vector<const xAOD::IParticle*> hardObjs,
                                              std::vector<const xAOD::IParticle*>& felist,
                                              const met::METAssociator::ConstitHolder& constits,
                                              std::map<const IParticle*,MissingETBase::Types::constvec_t> & /*momenta*/,
                                              float& UEcorr) const
  {
    // Constructing association electron-FE map
    if(obj->type() != xAOD::Type::ObjectType::Electron){
      UEcorr=0.0;
      felist={};
      return StatusCode::SUCCESS;
    }
    const xAOD::Egamma *eg = static_cast<const xAOD::Egamma*>(obj);

    // Preselect charged and neutral FEs, based on proximity: dR < m_Drcone
    for(const auto fe : *constits.feCont) {
      if(eg && P4Helpers::isInDeltaR(*fe, *eg, m_Drcone, m_useRapidity)) {
        if( ( !fe->isCharged() && fe->e() > FLT_MIN ) ||
            ( fe->isCharged() && PVMatchedAcc(*fe)  && ( !m_cleanChargedPFO || isGoodEoverP(static_cast<const xAOD::TrackParticle*>(fe->chargedObject(0))) ) ) ) {
          felist.push_back(fe);
        } // quality cuts
      } // DeltaR check
    } // FE loop

    // Step 2. Calculating Uncorrected HR and UE energy correction
    if(eg){
      // Vectoral sum of all FEs
      TLorentzVector HR;  // uncorrected HR (initialized with 0,0,0,0 automatically)
      for(const auto fe_itr : *constits.feCont) {
        if( fe_itr->pt() < 0 || fe_itr->e() < 0 ) { // sanity check
          continue;
        }

        //remove charged FE that are not matched to the PV
        if(fe_itr->isCharged() && !PVMatchedAcc(*fe_itr)){
          continue;
        }
        HR += fe_itr->p4();
      }

      // Create a vector of egamma form hardObjs (all electrons)
      std::vector<const xAOD::Egamma*> v_eg;
      for(const auto& obj_i : hardObjs){
        const xAOD::Egamma* eg_curr = static_cast<const xAOD::Egamma*>(obj_i); // current egamma object
        v_eg.push_back( eg_curr );
      }

      // Subtruct FEs which are in the cone around egamma (gives uncorrected HR)
      for(const auto fe_i : *constits.feCont) {  // charged and neutral FEs
        if( fe_i->pt() < 0 || fe_i->e() < 0 ) { // sanity check
          continue;
        }
        //std::cout << "new eg candidate" << std::endl;
        for(const auto& eg_i : v_eg) { // loop over v_eg
          double dR = P4Helpers::deltaR( fe_i->eta(), fe_i->phi(), eg_i->eta(), eg_i->phi() );
          if( dR < m_Drcone ) {
            HR -= fe_i->p4();
            break;
          }
        } // over v_eg
      } // over FEs

      // Save v_eg as a vector TLV (as commonn type for electrons and muons)
      std::vector<TLorentzVector> v_egTLV;
      v_egTLV.reserve(v_eg.size());
      for(const auto& eg_i : v_eg) { // loop over v_eg
        v_egTLV.push_back( eg_i->p4() );
      }

      // Save current eg as TLV
      TLorentzVector egTLV = eg->p4();

      // Get UE correction
      ATH_CHECK( GetUEcorr(constits, v_egTLV, egTLV, HR, m_Drcone, m_MinDistCone, UEcorr) );
    } // eg existance requirement

    return StatusCode::SUCCESS;
  }

  //**********************************************************************
  // Select Egamma tracks & clusters

  StatusCode METEgammaAssociator::selectEgammaClusters(const xAOD::CaloCluster* swclus,
                                                       const std::vector<const IParticle*>& inputTC,
                                                       std::vector<const xAOD::IParticle*>& tclist) const
  {    
    double eg_cl_e = swclus->e();

    bool doSum = true;
    double sumE_tc = 0.;
    const IParticle* bestbadmatch = nullptr;
    for(const auto& cl : inputTC) {
      double tcl_e = cl->e();
      // skip cluster if it's above our bad match threshold
      // retain pointer of the closest matching cluster in case no better is found
      if(tcl_e>m_tcMatch_maxRat*eg_cl_e) {
            ATH_MSG_VERBOSE("Reject topocluster in sum. Ratio vs eg cluster: " << (tcl_e/eg_cl_e));
            if( !bestbadmatch || (fabs(tcl_e/eg_cl_e-1.) < fabs(bestbadmatch->e()/eg_cl_e-1.)) ) bestbadmatch = cl;
            continue;
      }

      ATH_MSG_VERBOSE("E match with new cluster: " << fabs(sumE_tc+tcl_e - eg_cl_e) / eg_cl_e);
      if( (doSum = (fabs(sumE_tc+tcl_e - eg_cl_e) < fabs(sumE_tc - eg_cl_e))) ) {
            ATH_MSG_VERBOSE("Accept topocluster with pt " << cl->pt() << ", e " << cl->e() << " in sum.");
            ATH_MSG_VERBOSE("Energy ratio of nPFO to eg: " << tcl_e / eg_cl_e);
            ATH_MSG_VERBOSE("E match with new cluster: " << fabs(sumE_tc+tcl_e - eg_cl_e) / eg_cl_e);
            tclist.push_back(cl);
            sumE_tc += tcl_e;
      } // if we will retain the topocluster
    } // loop over nearby clusters
    if(sumE_tc<FLT_MIN && bestbadmatch) {
      ATH_MSG_VERBOSE("No better matches found -- add bad match topocluster with pt "
                          << bestbadmatch->pt() << ", e " << bestbadmatch->e() << ".");
      tclist.push_back(bestbadmatch);
    }
    return StatusCode::SUCCESS;
  }

  StatusCode METEgammaAssociator::selectEgammaTracks(const xAOD::Egamma* eg,
                                                     const xAOD::TrackParticleContainer* trkCont,
                                                     std::set<const xAOD::TrackParticle*>& tracklist) const
  {
    // switch to using egamma helpers for track extraction
    // set ensures that there's no duplication
    const std::set<const xAOD::TrackParticle*> egtracks = EgammaHelpers::getTrackParticles(eg);
    for(const auto& track : egtracks) {
        ATH_MSG_VERBOSE("Accept " << eg->type() << " track " << track << " px, py = " << track->p4().Px() << ", " << track->p4().Py());
        tracklist.insert(track);
    } // end initial egamma track loop

    // for objects with ambiguous author, grab the tracks matched to the counterpart ambiguous object too
    // set ensures that there's no duplication
    if (eg->author() & xAOD::EgammaParameters::AuthorAmbiguous && eg->ambiguousObject()) {
      const std::set<const xAOD::TrackParticle*> ambitracks = EgammaHelpers::getTrackParticles(eg->ambiguousObject());
      for(const auto& track : egtracks) {
        ATH_MSG_VERBOSE("Accept ambiguous " << eg->type() << " track " << track << " px, py = " << track->p4().Px() << ", " << track->p4().Py());
        tracklist.insert(track);
      }
    } // end ambiguous track case

    // in a small dR window, also accept tracks without an IBL hit
    for(const auto *const track : *trkCont) {
      if(P4Helpers::isInDeltaR(*track, *eg, m_extraTrkMatch_dR, m_useRapidity)) {
        // dR check should be faster than track summary retrieval
        uint8_t expect_innermostHit(false);
        uint8_t N_innermostHit(false);
        uint8_t expect_nextToInnermostHit(false);
        uint8_t N_nextToInnermostHit(false);
        if( !track->summaryValue(expect_innermostHit, expectInnermostPixelLayerHit)
            || !track->summaryValue(expect_nextToInnermostHit, expectNextToInnermostPixelLayerHit)) {
          ATH_MSG_WARNING("Track summary retrieval failed for 'expect(NextTo)InnermostPixelLayerHit'");
          return StatusCode::FAILURE;
        }
        if(expect_innermostHit) {
          if( !track->summaryValue(N_innermostHit, numberOfInnermostPixelLayerHits) ) {
            ATH_MSG_WARNING("Track summary retrieval failed for 'numberOfInnermostPixelLayerHits'");
            return StatusCode::FAILURE;
            if(N_innermostHit==0 ) {
              ATH_MSG_VERBOSE("Accept nearby track w/o innermost hit");
              tracklist.insert(track);
            }
          }
        } else if(expect_nextToInnermostHit) {
          if( !track->summaryValue(N_nextToInnermostHit, numberOfNextToInnermostPixelLayerHits) ) {
            ATH_MSG_WARNING("Track summary retrieval failed for 'numberOfNextToInnermostPixelLayerHits'");
            return StatusCode::FAILURE;
            if(N_nextToInnermostHit==0 ) {
              ATH_MSG_VERBOSE("Accept nearby track w/o next-to-innermost hit");
              tracklist.insert(track);
            }
          }
        }
        
      } // end dR check
    } // end extra track loop
    return StatusCode::SUCCESS;
  }


  bool METEgammaAssociator::hasUnmatchedClusters(const xAOD::Egamma* eg, const xAOD::PFO* pfo) const{

          bool   has_unmatched=false;
	  float  totSumpt=0;
	  float  unmatchedSumpt=0;
	  float  unmatchedE=0;
	  float  unmatchedTotEMFrac=0;
	  double emfrac=0;

    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFrac("unmatchedFrac");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFracSumpt("unmatchedFracSumpt");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFracPt("unmatchedFracPt");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFracE("unmatchedFracE");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFracEClusterPFO("unmatchedFracEClusterPFO");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedFracPtClusterPFO("unmatchedFracPtClusterPFO");
    	  static const SG::AuxElement::Decorator<Float_t> dec_unmatchedTotEMFrac("unmatchedTotEMFrac");

	  TLorentzVector totVec(0.,0.,0.,0.), unmatchedVec(0.,0.,0.,0.);
	  const std::vector<const xAOD::CaloCluster*> egClusters = xAOD::EgammaHelpers::getAssociatedTopoClusters(eg->caloCluster());
	  std::set<const xAOD::CaloCluster*> cPFOClusters;
	  int nCluscPFO = pfo->nCaloCluster();

	  for (int cl = 0; cl < nCluscPFO; ++cl) {
	      if (pfo->cluster(cl)) cPFOClusters.insert( pfo->cluster(cl) );
	  }

	  std::vector<const xAOD::CaloCluster*> unmatchedClusters;
	  for (const xAOD::CaloCluster* pfoclus : cPFOClusters) {
	      TLorentzVector tmpVec;
	      tmpVec.SetPtEtaPhiE(pfoclus->pt(),pfoclus->eta(),pfoclus->phi(),pfoclus->e());
	      totSumpt+=pfoclus->pt();
	      totVec+=tmpVec;
	      bool inEgamma = false;
	      for (const xAOD::CaloCluster* phclus : egClusters) {
	        if (pfoclus == phclus) {
		   inEgamma = true;
	        }
	    }
	    if (!inEgamma) {
	        unmatchedClusters.push_back(pfoclus);
		unmatchedSumpt+=pfoclus->pt();
		unmatchedE+=pfoclus->e();
		unmatchedVec+=tmpVec;
		pfoclus->retrieveMoment(xAOD::CaloCluster::ENG_FRAC_EM ,emfrac);
		unmatchedTotEMFrac=unmatchedTotEMFrac+emfrac*pfoclus->e(); 
	    }

	  }

	  ATH_MSG_DEBUG("PFO associated to "<<nCluscPFO<< " cluster, of which " << unmatchedClusters.size() << "unmatched one and unmatched pt "<<unmatchedSumpt);
	  dec_unmatchedFrac(*pfo)=nCluscPFO>0 ?  float(unmatchedClusters.size())/float(nCluscPFO) : -1;
	  dec_unmatchedFracPt(*pfo)= totVec.Pt()>0 ? float(unmatchedVec.Pt()/totVec.Pt()): -1;
	  dec_unmatchedFracSumpt(*pfo)= totSumpt>0 ? float(unmatchedSumpt/totSumpt): -1;
	  dec_unmatchedFracE(*pfo)= totVec.E()>0 ? float(unmatchedE/totVec.E()): -1;
	  dec_unmatchedTotEMFrac(*pfo)= totVec.E()>0 ? float(unmatchedTotEMFrac/totVec.E()): -1; 
	  dec_unmatchedFracEClusterPFO(*pfo)= pfo->e()>0 ? float(unmatchedE/pfo->e()): -1;
	  dec_unmatchedFracPtClusterPFO(*pfo)= pfo->pt()>0 ? float(unmatchedE/pfo->pt()): -1;
                                                                                                                                           	  
   return has_unmatched;
  }



}

