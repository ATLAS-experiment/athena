/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "GlobalJet_TagAlgTool.h"

#include "../IO/CommonTOB.h"

#include "StoreGate/ReadHandle.h"

#include <algorithm>
#include <bitset>

namespace GlobalSim {

  // Main constructor
  GlobalJet_TagAlgTool::GlobalJet_TagAlgTool(const std::string& type, const std::string& name, const IInterface* parent) :
    base_class(type, name, parent) {
  }


  // Initialize function running before first event
  StatusCode GlobalJet_TagAlgTool::initialize() {

    CHECK(m_gblJet1JetsContainerKey.initialize());
    CHECK(m_gblJet_TagJetsContainerKey.initialize());

    // v2 (basic) algorithm configuration
    // Mirroring the BasicV2 preset in TrigGepPerf's GepJetAlgConfig.py.
    // Kept in step with Jet_TagAlg::initialize(), which configures the same
    // algorithm for the BitSpec path.
    Gep::JetTaggerLRJConfig cfg;

    cfg.algoVersion          = 2;
    cfg.nSeedsInput          = m_nSeedsInput;
    cfg.nSeedsOutput         = m_nSeedsOutput;
    cfg.maxObjectsConsidered = m_maxObjectsConsidered;
    cfg.r2Cut                = static_cast<double>(m_jetR) * static_cast<double>(m_jetR);

    // Midpoint seeding and overlap removal are only in v3 algorithm --> disabled
    cfg.midpointSearchDistance   = 0.001;
    cfg.enableOverlapRemoval     = false;
    cfg.minEtSeedPosOptimization = false;
    cfg.enableEtWeightedMidpoint = false;

    // Field widths: the standard TOB format, matching CommonTOB's s_*_width
    cfg.et_bit_length  = static_cast<unsigned int>(IOBitwise::CommonTOB::s_et_width);
    cfg.eta_bit_length = static_cast<unsigned int>(IOBitwise::CommonTOB::s_eta_width);
    cfg.phi_bit_length = static_cast<unsigned int>(IOBitwise::CommonTOB::s_phi_width);

    // v2 computes no jet substructure, so every substructure field is zero-width
    cfg.num_subjets_length       = 0;
    cfg.N_subjetiness_bit_length = 0;
    cfg.mass_approx_bit_length   = 0;
    cfg.psi_R_bit_length         = 0;
    cfg.deltaR_lut_length        = 8;

    // Digitization ranges. et_max 2048 GeV over 13 bits gives the 0.25 GeV
    // LSB. NOTE: unlike the BitSpec path, nothing on this path actually
    // defines an Et LSB - CommonTOB stores raw bits, and GlobalCellTowerAlgTool
    // packs cell energies in MeV without scaling. The Et counts arriving here
    // are therefore not on the 0.25 GeV scale assumed below. Only eta/phi are
    // used geometrically, so the clustering is unaffected, but the output Et
    // inherits the input scale rather than this one.
    cfg.et_min  = 0.0;
    cfg.et_max  = 2048.0;
    cfg.eta_min = -4.85;
    cfg.eta_max = 4.95;
    cfg.phi_min = -3.15;
    cfg.phi_max = 3.25;

    cfg.computeDerived();
    m_maker.m_cfg = cfg;
    // Jet inputs: the seed's own E_T is primed into the output sum, and a
    // merged jet above threshold is itself a subjet candidate.
    m_maker.SetConstSource(Gep::JetTaggerConstSource::WTACone);

    ATH_MSG_DEBUG("Jet_Tag configured: v2, with R=" << cfg.rCut
                  << ", nSeedsInput=" << cfg.nSeedsInput
                  << ", nSeedsOutput=" << cfg.nSeedsOutput
                  << ", maxObjects=" << cfg.maxObjectsConsidered
                  << ", digitized_delta_R2Cut=" << cfg.digitized_delta_R2Cut
                  << ", pi_digitized_in_phi=" << cfg.pi_digitized_in_phi);

    return StatusCode::SUCCESS;
  }


  // Main functional block running for each event
  StatusCode GlobalJet_TagAlgTool::run(const std::unique_ptr<IDataCollector>& dc,
				       const EventContext& ctx) const {

    ATH_MSG_DEBUG("Building large-R tagged jets");
    if (dc){dc->collect(*this, "start");}

    // Read the Jet1 (WTA cone) small-R jets
    SG::ReadHandle<IOBitwise::Jet1TOBContainer> h_Jet1TOBs =
      SG::makeHandle(m_gblJet1JetsContainerKey, ctx);
    CHECK(h_Jet1TOBs.isValid());
    const IOBitwise::Jet1TOBContainer & inJets = *h_Jet1TOBs;
    const unsigned int nJets = inJets.size();
    ATH_MSG_DEBUG("Reading " << nJets << " Jet1Jets as Jet1TOBs");

    // Reading in Jet1 input (in same manner that Jet1 reads in tower input).
    // Only the CommonTOB kinematics are used
    std::vector<Gep::JetTaggerLRJMaker::DigiObj> objects;
    objects.reserve(nJets);
    for(unsigned int i = 0; i < nJets; i++){
      const IOBitwise::Jet1TOB* inJet = inJets[i];
      Gep::JetTaggerLRJMaker::DigiObj obj;
      obj.et  = inJet->et_bits().to_ulong();
      obj.eta = inJet->eta_bits().to_ulong();
      obj.phi = inJet->phi_bits().to_ulong();
      objects.push_back(obj);
    }

    // Seeds and constituents are read as two disjoint slices of the same
    // jet collection: the leading nSeedsOutput jets become the large-R jet
    // seeds, and the next maxObjectsConsidered are the constituents merged
    // into them.
    const size_t nSeeds = std::min<size_t>(m_nSeedsOutput.value(), objects.size());
    const size_t nConst = std::min<size_t>(m_maxObjectsConsidered.value(),
                                           objects.size() - nSeeds);

    const std::vector<Gep::JetTaggerLRJMaker::DigiObj>
        seeds(objects.begin(), objects.begin() + nSeeds);
    const std::vector<Gep::JetTaggerLRJMaker::DigiObj>
        constituents(objects.begin() + nSeeds, objects.begin() + nSeeds + nConst);

    const auto lrjs = m_maker.makeLargeRJetsDigitized(seeds, constituents);

    auto h_Jet_TagTOBs = SG::makeHandle(m_gblJet_TagJetsContainerKey, ctx);
    auto jets = std::make_unique<IOBitwise::CommonTOBContainer>();

    // The tagger already masks its outputs to the field widths configured
    // above, which are CommonTOB's, so the counts drop straight into the bits.
    for(const auto& lrj : lrjs){
      jets->emplace_back(new IOBitwise::CommonTOB(std::bitset<IOBitwise::CommonTOB::s_et_width>(lrj.et),
						  std::bitset<IOBitwise::CommonTOB::s_eta_width>(lrj.eta),
						  std::bitset<IOBitwise::CommonTOB::s_phi_width>(lrj.phi)));
      // Print jet TOB
      ATH_MSG_DEBUG("Returning large-R Jet: " << jets->back()->to_string());
    }

    ATH_MSG_DEBUG("Built " << jets->size() << " large-R tagged jets and stored them as GenericTobs");

    CHECK(h_Jet_TagTOBs.record(std::move(jets)));

    if (dc){dc->collect(*this, "end");}

    return StatusCode::SUCCESS;
  }

  // Overrides toString() function from base class, unused here
  std::string GlobalJet_TagAlgTool::toString() const {
      return {};
  }

} //namespace GlobalSim
