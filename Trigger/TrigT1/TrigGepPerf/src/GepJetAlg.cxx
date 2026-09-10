
/*
 *   Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
 */

#include "./GepJetAlg.h"

// Interface to jet reconstruction objects
#include "./IJetMaker.h"

// concrete jet reconstruction classes.
#include "./ModAntikTJetMaker.h"
#include "./ConeJetMaker.h"
#include "./WTAConeJetMaker.h"
#include "./JetTaggerLRJJetMaker.h"

// input and output types
#include "./Cluster.h"
#include "./Jet.h"
#include "./JetTaggerLargeRJet.h"

#include "xAODEventInfo/EventInfo.h"
#include "xAODJet/JetAuxContainer.h"

#include <tuple>
#include <vector>

GepJetAlg::GepJetAlg( const std::string& name, ISvcLocator* pSvcLocator ) :
    AthReentrantAlgorithm( name, pSvcLocator ){

}


StatusCode GepJetAlg::initialize() {
  ATH_MSG_INFO ("Initializing " << name() << "...");
  ATH_MSG_INFO ("Jet alg " << m_jetAlgName);

  // Initialize data access keys
  CHECK(m_caloClustersKey.initialize());
  CHECK(m_jFexSRJetsKey.initialize());
  CHECK(m_outputGepJetsKey.initialize());

  CHECK(m_lrjWTAConeSeedsKey.initialize(!m_lrjWTAConeSeedsKey.empty()));
  CHECK(m_lrjGFexSRJetsKey.initialize(!m_lrjGFexSRJetsKey.empty()));

  // Build the JetTaggerLRJ configuration (and its deltaR LUT) once, so it is
  // not recomputed per event. Gated on an explicit property (set by
  // GepJetTaggerLRJAlgCfg) rather than the mode string; guard that the two
  // agree so a misconfiguration fails loudly instead of silently skipping.
  if (m_configureLRJMaker != (m_jetAlgName == "JetTaggerLRJ")) {
    ATH_MSG_ERROR("EnableLRJMaker (" << m_configureLRJMaker.value()
                  << ") is inconsistent with jetAlgName '" << m_jetAlgName.value()
                  << "'; both must select JetTaggerLRJ together.");
    return StatusCode::FAILURE;
  }
  if (m_configureLRJMaker) {
    ATH_CHECK(configureLRJMaker());
  }

  return StatusCode::SUCCESS;
}


StatusCode GepJetAlg::configureLRJMaker() {

  // All JetTaggerLRJ settings are resolved in Python (GepJetTaggerLRJAlgCfg
  // layers the BasicV2 / AdvancedV3 preset under any user overrides), so the
  // properties are already concrete: copy them straight into the config.
  Gep::JetTaggerLRJConfig cfg;
  cfg.algoVersion = m_LRJAlgoVersion;

  // Geometry.
  cfg.r2Cut     = static_cast<double>(m_LRJJetR) * static_cast<double>(m_LRJJetR);
  cfg.midpointSearchDistance = m_LRJDSearch;

  // Multiplicities.
  cfg.nSeedsInput          = m_LRJNSeedsInput;
  cfg.nProtoSeeds          = m_LRJNProtoSeeds;
  cfg.nSeedsOutput         = m_LRJNSeedsOutput;
  cfg.maxObjectsConsidered = m_LRJMaxObjectsConsidered;

  // Digitization bit lengths.
  cfg.et_bit_length            = m_LRJEtBitLength;
  cfg.eta_bit_length           = m_LRJEtaBitLength;
  cfg.phi_bit_length           = m_LRJPhiBitLength;
  cfg.num_subjets_length       = m_LRJNumSubjetsLength;
  cfg.N_subjetiness_bit_length = m_LRJNSubjetinessBitLength;
  cfg.mass_approx_bit_length   = m_LRJMassApproxBitLength;
  cfg.psi_R_bit_length         = m_LRJPsiRBitLength;
  cfg.deltaR_lut_length        = m_LRJDeltaRLutLength;

  // Physical ranges.
  cfg.phi_min        = m_LRJPhiMin;
  cfg.phi_max        = m_LRJPhiMax;
  cfg.eta_min        = m_LRJEtaMin;
  cfg.eta_max        = m_LRJEtaMax;
  cfg.et_min         = m_LRJEtMin;
  cfg.et_max         = m_LRJEtMax;
  cfg.massApprox_max = m_LRJMassApproxMax;
  cfg.inputEtToGeV   = m_LRJInputEtToGeV;

  // Thresholds.
  cfg.subjetEtThresholdGeV  = m_LRJSubjetEtThresholdGeV;
  cfg.minEtSeedPosOptCutGeV = m_LRJMinEtSeedPosOptCutGeV;
  cfg.constEtCutGeV         = m_LRJConstEtCutGeV;

  // Flow / output toggles.
  cfg.enableOverlapRemoval     = m_LRJEnableOverlapRemoval;
  cfg.enableEtWeightedMidpoint = m_LRJEnableEtWeightedMidpoint;
  cfg.minEtSeedPosOptimization = m_LRJMinEtSeedPosOptimization;
  cfg.writeSubstructure        = m_LRJWriteSubstructure;
  cfg.writeSubjetKinematics    = m_LRJWriteSubjetKinematics;
  cfg.writeConstituentIndices  = m_LRJWriteConstituentIndices;

  // Digitization must be fully specified (Python supplies the preset); a zero
  // in these fields means the algorithm was built without GepJetTaggerLRJAlgCfg,
  // which would make computeDerived() produce nonsense granularities
  if (cfg.et_bit_length == 0 || cfg.eta_bit_length == 0 ||
      cfg.phi_bit_length == 0 || cfg.deltaR_lut_length == 0) {
    ATH_MSG_ERROR("JetTaggerLRJ digitization not configured (et/eta/phi/deltaR "
                  "bit length is 0); configure it via GepJetTaggerLRJAlgCfg.");
    return StatusCode::FAILURE;
  }

  // Fill derived constants and build the deltaR LUT once.
  cfg.computeDerived();
  m_lrjMaker.m_cfg = cfg;

  // Seed source: resolve the string to an enum once (no silent fallback) and
  // require the matching input collection to be configured. Doing this here, in
  // initialize(), means a bad seed/key combination fails at configure time
  // rather than mid-event.
  if (m_LRJSeedSource == "WTACone") {
    m_lrjMaker.SetSeedSource(Gep::JetTaggerSeedSource::WTACone);
    if (m_lrjWTAConeSeedsKey.key().empty()) {
      ATH_MSG_ERROR("LRJSeedSource=WTACone requires LRJWTAConeSeedsKey to be set");
      return StatusCode::FAILURE;
    }
  } else if (m_LRJSeedSource == "jFexSRJ") {
    m_lrjMaker.SetSeedSource(Gep::JetTaggerSeedSource::jFexSRJ);
    if (m_jFexSRJetsKey.key().empty()) {
      ATH_MSG_ERROR("LRJSeedSource=jFexSRJ requires jFexSRJetRoIs to be set");
      return StatusCode::FAILURE;
    }
  } else if (m_LRJSeedSource == "gFexSRJ") {
    m_lrjMaker.SetSeedSource(Gep::JetTaggerSeedSource::gFexSRJ);
    if (m_lrjGFexSRJetsKey.key().empty()) {
      ATH_MSG_ERROR("LRJSeedSource=gFexSRJ requires LRJgFexSRJetRoIs to be set");
      return StatusCode::FAILURE;
    }
  } else {
    ATH_MSG_ERROR("Unknown LRJSeedSource '" << m_LRJSeedSource.value() << "'");
    return StatusCode::FAILURE;
  }

  // Constituent source: same explicit resolution (no silent fallback).
  if (m_LRJConstSource == "WTACone") {
    m_lrjMaker.SetConstSource(Gep::JetTaggerConstSource::WTACone);
  } else if (m_LRJConstSource == "Towers") {
    m_lrjMaker.SetConstSource(Gep::JetTaggerConstSource::Towers);
  } else {
    ATH_MSG_ERROR("Unknown LRJConstSource '" << m_LRJConstSource.value() << "'");
    return StatusCode::FAILURE;
  }

  ATH_MSG_INFO("Configured JetTaggerLRJ (v" << cfg.algoVersion
               << ", seed=" << m_LRJSeedSource.value()
               << ", const=" << m_LRJConstSource.value()
               << "): r2Cut=" << cfg.r2Cut << ", midpointSearchDistance=" << cfg.midpointSearchDistance
               << ", deltaR LUT size=" << cfg.lutR_8b.size());

  return StatusCode::SUCCESS;
}

// Unified function for loading jet tagger LRJ seeds (WTA-cone, gFEX, or jFEX)
template <typename Container, typename KinFn>
StatusCode GepJetAlg::loadLRJSeeds(const SG::ReadHandleKey<Container>& key,
                                   const EventContext& ctx,
                                   KinFn&& kin,
                                   std::vector<Gep::Jet>& seeds) const {
  auto h = SG::makeHandle(key, ctx);
  CHECK(h.isValid());
  seeds.reserve(seeds.size() + h->size());
  for (const auto* j : *h) {
    const auto [pt, eta, phi, m] = kin(j);
    Gep::Jet s;
    s.vec.SetPtEtaPhiM(pt, eta, phi, m);
    s.etaInput = eta;  // raw, pre-round-trip (see Jet.h / JetTaggerLRJ)
    s.phiInput = phi;
    seeds.push_back(std::move(s));
  }
  return StatusCode::SUCCESS;
}


StatusCode GepJetAlg::execute(const EventContext& context) const {
  ATH_MSG_DEBUG ("Executing " << name() << "...");
  
  
  SG::WriteHandle<xAOD::JetContainer>
    h_outputJets = SG::makeHandle(m_outputGepJetsKey, context);


  CHECK(h_outputJets.record(std::make_unique<xAOD::JetContainer>(),
			    std::make_unique<xAOD::JetAuxContainer>()));
	
  // read in clusters
  auto h_caloClusters = SG::makeHandle(m_caloClustersKey, context);
  CHECK(h_caloClusters.isValid());
  ATH_MSG_DEBUG("Read in " << h_caloClusters->size() << " clusters");

  const auto& clusters = *h_caloClusters; 



  std::vector<Gep::Cluster> gepClusters;
  std::transform(clusters.cbegin(),
		 clusters.cend(),
		 std::back_inserter(gepClusters),
		 [](const auto& cluster){
		   Gep::Cluster c(cluster->p4());
		   // Keep the raw eta/phi (pre-TLorentzVector round-trip) so
		   // JetTaggerLRJ can digitize the same values the emulation reads.
		   c.etaInput = cluster->eta();
		   c.phiInput = cluster->phi();
		   return c;});
		 


  // create a  jet maker
  std::unique_ptr<Gep::IJetMaker> jetMaker{};
  

  if ( m_jetAlgName=="ModAntikT" ) {
    jetMaker.reset(new Gep::ModAntikTJetMaker());
  }
  
  else if ( m_jetAlgName=="Cone" ) {

    // Use jJFexSR RoIs as seeds
    auto h_seeds = SG::makeHandle(m_jFexSRJetsKey, context);
    CHECK(h_seeds.isValid());
    ATH_MSG_DEBUG("No of seeds "<< h_seeds->size());
    jetMaker.reset(new Gep::ConeJetMaker(0.4, *h_seeds));
    
  } else if(m_jetAlgName=="WTACone"){ // Large block for the WTACone

    auto WTAConeJetMaker = std::make_unique<Gep::WTAConeJetMaker>(); // Default parameters for now

    #ifdef FLOATING_POINT_SIMULATION
      WTAConeJetMaker->m_GEPWTAParameters.SetConstEtCut(m_WTAConstEtCut * Athena::Units::GeV); // Set ConstEtCut to 2GeV
      WTAConeJetMaker->m_GEPWTAParameters.SetSeedEtCut(m_WTASeedEtCut * Athena::Units::GeV); // Set SeedEtCut to 5GeV by default
      WTAConeJetMaker->m_GEPWTAParameters.SetJet_dR(m_WTAJet_dR);
      WTAConeJetMaker->m_GEPWTAParameters.SetIso_dR(m_WTAJet_dR); // Default is Jet_dR = Iso_dR
    #else
      // float to int/bitwise conversion
      WTAConeJetMaker->m_GEPWTAParameters.SetConstEtCut(static_cast<unsigned int>(m_WTAConstEtCut * Athena::Units::GeV / LSB)); // Set ConstEtCut to 8 bits, LSB = 250 MeV
      WTAConeJetMaker->m_GEPWTAParameters.SetSeedEtCut(static_cast<unsigned int>(m_WTASeedEtCut * Athena::Units::GeV / LSB)); // Set SeedEtCut to 20 bits by default
      WTAConeJetMaker->m_GEPWTAParameters.SetJet_dR(static_cast<unsigned int>(m_WTAJet_dR * 10));
      WTAConeJetMaker->m_GEPWTAParameters.SetIso_dR(static_cast<unsigned int>(m_WTAJet_dR * 10));
    #endif

    WTAConeJetMaker->m_GEPWTAParameters.SetMaxConstN(m_WTAMaxConstN);
    WTAConeJetMaker->m_GEPWTAParameters.SetMaxSeedSortingN(m_WTAMaxSeedSortingN);
    WTAConeJetMaker->SetBlockN(m_WTABlockN);

    WTAConeJetMaker->SetSeedCleaningAlgo(0); // 0 = Baseline
    if(m_WTASeedCleaningName=="TwoPass")WTAConeJetMaker->SetSeedCleaningAlgo(1); // 1 = TwoPass

    jetMaker = std::move(WTAConeJetMaker);
  } // WTACone loop, will be updated as the WTAConeJets
  else if ( m_jetAlgName=="JetTaggerLRJ" ) {

    // ------------------------------------------------------------------
    // JetTaggerLRJ: modified seeded-cone large-R jet maker.
    // ------------------------------------------------------------------

    // ---- build seeds vector (Gep::Jet) ----
    // Seed source is validated in initialize(); dispatch on the resolved enum.
    // The sources differ only in the input collection and the (pt/et, mass)
    // accessor, which each per-case lambda supplies to loadLRJSeeds().
    std::vector<Gep::Jet> lrjSeeds;
    switch (m_lrjMaker.GetSeedSource()) {
      case Gep::JetTaggerSeedSource::WTACone:
        ATH_CHECK(loadLRJSeeds(m_lrjWTAConeSeedsKey, context,
            [](const xAOD::Jet* j) {
              return std::tuple{j->pt(), j->eta(), j->phi(), j->m()};
            }, lrjSeeds));
        break;
      case Gep::JetTaggerSeedSource::jFexSRJ:
        ATH_CHECK(loadLRJSeeds(m_jFexSRJetsKey, context,
            [](const xAOD::jFexSRJetRoI* j) {
              return std::tuple{static_cast<double>(j->et()), static_cast<double>(j->eta()), static_cast<double>(j->phi()), 0.0};
            }, lrjSeeds));
        break;
      case Gep::JetTaggerSeedSource::gFexSRJ:
        ATH_CHECK(loadLRJSeeds(m_lrjGFexSRJetsKey, context,
            [](const xAOD::gFexJetRoI* j) {
              return std::tuple{static_cast<double>(j->et()), static_cast<double>(j->eta()), static_cast<double>(j->phi()), 0.0};
            }, lrjSeeds));
        break;
    }
    ATH_MSG_DEBUG("JetTaggerLRJ loaded " << lrjSeeds.size() << " seeds ("
                  << m_LRJSeedSource.value() << ")");

    // ---- build constituents vector (Gep::Cluster) ----
    // Both "Towers" and "WTACone" constituent sources currently reuse the
    // input m_caloClustersKey container; runConfig.py points it at the
    // appropriate collection ("CellTower" or the WTACone output). When the
    // two need to differ at run time the maker takes std::vector<Gep::Cluster>
    // either way; adjust the wiring here without touching the maker.
    const std::vector<Gep::Cluster>& lrjConstituents = gepClusters;

    // Maker (and its deltaR LUT) is configured once in initialize().
    std::vector<Gep::LargeRJet> lrjs = m_lrjMaker.makeLargeRJets(lrjSeeds, lrjConstituents);
    ATH_MSG_DEBUG("JetTaggerLRJ produced " << lrjs.size() << " large-R jets");

    if (!lrjs.empty()) {
      for (const auto& lrj : lrjs) {
        std::unique_ptr<xAOD::Jet> xAODJet{new xAOD::Jet()};
        xAOD::Jet* p = xAODJet.get();
        h_outputJets->push_back(std::move(xAODJet));

        xAOD::JetFourMom_t p4;
        p4.SetPt (lrj.vec.Pt());
        p4.SetEta(lrj.vec.Eta());
        p4.SetPhi(lrj.vec.Phi());
        p4.SetM  (lrj.vec.M());
        p->setJetP4(p4);

        p->setAttribute("RCut",        lrj.radius);
        p->setAttribute("SeedEt",      lrj.seedEt);
        p->setAttribute("SeedEta",     lrj.seedEta);
        p->setAttribute("SeedPhi",     lrj.seedPhi);
        p->setAttribute("NSubjets",    lrj.nSubjets);
        p->setAttribute("Psi_R",       lrj.psi_R);
        p->setAttribute("Tau_1",       lrj.tau_1);
        p->setAttribute("Tau_2",       lrj.tau_2);
        p->setAttribute("Tau_21",      lrj.tau_21);
        p->setAttribute("MassApprox",  lrj.massApprox);
        // Per-subjet kinematics as indexed SCALAR attributes (SubjetEt0,
        // SubjetEt1, ...). std::vector<float> jet attributes do not round-trip
        // to the reader here (neither setAttribute nor auxdata), whereas
        // scalar attributes do; the reader reads back NSubjets of them.
        for (size_t is = 0; is < lrj.subjet_et.size(); ++is) {
          const std::string s = std::to_string(is);
          p->setAttribute(std::string("SubjetEt")  + s, lrj.subjet_et[is]);
          p->setAttribute(std::string("SubjetEta") + s, lrj.subjet_eta[is]);
          p->setAttribute(std::string("SubjetPhi") + s, lrj.subjet_phi[is]);
        }

        // Attach constituents (indices into the input cluster container) so the
        // reader's numConstituents()/getConstituents() populate, mirroring the
        // generic maker path below.
        for (const int i : lrj.constituentsIndices) {
          p->addConstituent(clusters.at(i));
        }
      }
    }
    return StatusCode::SUCCESS;
  }
  else {
    ATH_MSG_ERROR( "Unknown JetMaker " <<  m_jetAlgName);
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG( "jet maker: " << jetMaker->toString());
  
  std::vector<Gep::Jet> gepJets = jetMaker->makeJets( gepClusters );
  
  ATH_MSG_DEBUG("Number of jets found for " <<
		 m_jetAlgName << " " <<gepJets.size());

  // if no jets were found, skip event
  if( gepJets.empty() ){
    return StatusCode::SUCCESS;
  }
  
  // store gep jets in athena format
  for(const auto& gjet: gepJets){
    
    std::unique_ptr<xAOD::Jet> xAODJet{new xAOD::Jet()};
    xAOD::Jet* p_xAODJet = xAODJet.get();

    // store the xAOD::Jet in the output container to prepare the Aux container
    // The move invalids the unique_ptr, but we still have the bare pointer
    // which allows the updating of the xAODJet from the gep jet data.
    h_outputJets->push_back(std::move(xAODJet));
    
    xAOD::JetFourMom_t p4;
    p4.SetPt(gjet.vec.Pt());
    p4.SetEta(gjet.vec.Eta());
    p4.SetPhi(gjet.vec.Phi());
    p4.SetM(gjet.vec.M());

    p_xAODJet->setJetP4(p4);
    
    p_xAODJet->setAttribute("RCut", gjet.radius);
    p_xAODJet->setAttribute("SeedEta", gjet.seedEta); // < gep attributes
    p_xAODJet->setAttribute("SeedPhi", gjet.seedPhi); //
    p_xAODJet->setAttribute("SeedEt", gjet.seedEt); //
    p_xAODJet->setAttribute("Ring0_Et", gjet.ring0_Et);
    p_xAODJet->setAttribute("Ring1_Et", gjet.ring1_Et);
    p_xAODJet->setAttribute("Ring2_Et", gjet.ring2_Et);
    p_xAODJet->setAttribute("Ring3_Et", gjet.ring3_Et);
    p_xAODJet->setAttribute("Ring4_Et", gjet.ring4_Et);
    p_xAODJet->setAttribute("Total_TobN", gjet.total_TobN);
    p_xAODJet->setAttribute("Ring0_TobN", gjet.ring0_TobN);
    p_xAODJet->setAttribute("Ring1_TobN", gjet.ring1_TobN);
    p_xAODJet->setAttribute("Ring2_TobN", gjet.ring2_TobN);
    p_xAODJet->setAttribute("Ring3_TobN", gjet.ring3_TobN);
    p_xAODJet->setAttribute("Ring4_TobN", gjet.ring4_TobN);

    for (const auto& i: gjet.constituentsIndices) {
      p_xAODJet->addConstituent(clusters.at(i));
    }
    
  }
	
  return StatusCode::SUCCESS;
}
