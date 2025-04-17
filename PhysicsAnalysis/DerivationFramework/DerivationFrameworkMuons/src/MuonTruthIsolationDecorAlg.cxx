/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////////
// MuonTruthIsolationDecorAlg.cxx, (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Runs on muons without a truth particle link.
// Finds the nearest stable truth particle and adds its info to the muon.
#include "MuonTruthIsolationDecorAlg.h"

#include "FourMomUtils/xAODP4Helpers.h"
#include "MuonDetDescrUtils/MuonSectorMapping.h"
#include "TruthUtils/HepMCHelpers.h"
namespace {
    static const SG::AuxElement::ConstAccessor<ElementLink<xAOD::TruthParticleContainer>> acc_tpl("truthParticleLink");

    static const SG::AuxElement::Decorator<float> decorator_topoetcone20("topoetcone20_truth");

    static const SG::AuxElement::Decorator<float> decorator_ptcone20("ptcone20_truth");
    static const SG::AuxElement::Decorator<float> decorator_ptvarcone20("ptvarcone20_truth");
    static const SG::AuxElement::Decorator<float> decorator_ptvarcone30("ptvarcone30_truth");

    static const SG::AuxElement::Decorator<float> decorator_ptcone20_pt500("ptcone20_pt500_truth");
    static const SG::AuxElement::Decorator<float> decorator_ptvarcone20_pt500("ptvarcone20_pt500_truth");
    static const SG::AuxElement::Decorator<float> decorator_ptvarcone30_pt500("ptvarcone30_pt500_truth");

    static const Muon::MuonSectorMapping sector_mapping;

}  // namespace
// Constructor
namespace DerivationFramework{
MuonTruthIsolationDecorAlg::MuonTruthIsolationDecorAlg(const std::string& name, ISvcLocator* pSvcLocator) :
    AthReentrantAlgorithm(name, pSvcLocator) {}

// Athena initialize and finalize
StatusCode MuonTruthIsolationDecorAlg::initialize() {
    ATH_MSG_VERBOSE("initialize() ...");
    ATH_CHECK(m_partSGKey.initialize());
    ATH_CHECK(m_truthSGKey.initialize());
    
    SG::AuxTypeRegistry& registry{SG::AuxTypeRegistry::instance()};
    m_topoetcone20_Key = registry.getName(decorator_topoetcone20.auxid());
    
    m_ptcone20_pt500_Key = registry.getName(decorator_ptcone20_pt500.auxid());
    m_ptvarcone20_pt500_Key = registry.getName(decorator_ptvarcone20_pt500.auxid());
    m_ptvarcone30_pt500_Key = registry.getName(decorator_ptvarcone30_pt500.auxid());

    m_ptcone20_Key = registry.getName(decorator_ptcone20.auxid());
    m_ptvarcone20_Key = registry.getName(decorator_ptvarcone20.auxid());
    m_ptvarcone30_Key = registry.getName(decorator_ptvarcone30.auxid());

    ATH_CHECK(m_topoetcone20_Key.initialize());
    ATH_CHECK(m_ptcone20_pt500_Key.initialize());
    ATH_CHECK(m_ptcone20_Key.initialize());
    ATH_CHECK(m_ptvarcone20_pt500_Key.initialize());
    ATH_CHECK(m_ptvarcone20_Key.initialize());
    ATH_CHECK(m_ptvarcone30_pt500_Key.initialize());
    ATH_CHECK(m_ptvarcone30_Key.initialize());

    return StatusCode::SUCCESS;
}

StatusCode MuonTruthIsolationDecorAlg::execute(const EventContext& ctx) const {
    // Retrieve main particle collection
    SG::ReadHandle<xAOD::IParticleContainer> parts{m_partSGKey, ctx};
    if (!parts.isValid()) {
        ATH_MSG_ERROR("No Muon collection with name " << m_partSGKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    SG::ReadHandle<xAOD::TruthEventContainer> tec{m_truthSGKey, ctx};
    if (!tec.isValid()) {
        ATH_MSG_ERROR("No truth collection with name " << m_truthSGKey.fullKey() << " found in StoreGate!");
        return StatusCode::FAILURE;
    }
    std::map<int, std::vector<const xAOD::TruthParticle*>> truth_map_calo;
    std::map<int, std::vector<const xAOD::TruthParticle*>> truth_map_track;

    // sector_mapping
    for (const xAOD::TruthEvent* event : *tec) {
        for (size_t parti = 0; parti < event->nTruthParticles(); ++parti) {
            const xAOD::TruthParticle* tpart = event->truthParticle(parti);
            if (!tpart || tpart->pt() < 1  || !MC::isStable(tpart) || HepMC::is_simulation_particle(tpart) || tpart->isNeutrino()) continue;
            const int sector = sector_mapping.getSector(tpart->phi());
            truth_map_calo[sector].push_back(tpart);
            if (tpart->pt() < 500 || !tpart->isCharged() || tpart->abseta() > 2.5) continue;
            truth_map_track[sector].push_back(tpart);
        }
    }

    for (const xAOD::IParticle* part : *parts) {
        const float varradius20 = std::min(10e3 / part->pt(), 0.2);
        const float varradius30 = std::min(10e3 / part->pt(), 0.3);
        const xAOD::TruthParticle* const truthLink = (acc_tpl(*part).isValid() ? *acc_tpl(*part) : nullptr);

        std::vector<int> sectors;
        sector_mapping.getSectors(part->phi(), sectors);

        float new_topoetcone20{0}, new_ptcone20{0}, new_ptvarcone20{0}, new_ptvarcone30{0}, 
              new_ptcone20_pt500{0}, new_ptvarcone20_pt500{0}, new_ptvarcone30_pt500{0};
        /// Loop over the sectors
        for (const int sector : sectors) {
            const std::vector<const xAOD::TruthParticle*>& calo_container = truth_map_calo[sector];
            /// Update the topo et cones
            for (const xAOD::TruthParticle* calo_part : calo_container) {
              if (calo_part == truthLink || (truthLink && HepMC::is_same_particle(truthLink,calo_part))) continue;
                const float dR = xAOD::P4Helpers::deltaR(calo_part, part, false);
                if (dR < 0.05 || dR > 0.2) continue;
                new_topoetcone20 += calo_part->pt();
            }

            const std::vector<const xAOD::TruthParticle*>& truth_container = truth_map_track[sector];
            for (const xAOD::TruthParticle* trk_part : truth_container) {
                if (trk_part == truthLink || (truthLink && HepMC::is_same_particle(truthLink,trk_part))) continue;
                const float dR = xAOD::P4Helpers::deltaR(trk_part, part, false);
                const float pt = trk_part->pt();
                if (dR > 0.3) continue;
                new_ptcone20_pt500 += (dR < 0.2) * pt;
                new_ptvarcone20_pt500 += (dR < varradius20) * pt;
                new_ptvarcone30_pt500 += (dR < varradius30) * pt;
                if (pt < 1000) continue;
                new_ptcone20 += (dR < 0.2) * pt;
                new_ptvarcone20 += (dR < varradius20) * pt;
                new_ptvarcone30 += (dR < varradius30) * pt;
            }
        }

        decorator_topoetcone20(*part) = new_topoetcone20;
        decorator_ptcone20(*part) = new_ptcone20;
        decorator_ptvarcone20(*part) = new_ptvarcone20;
        decorator_ptvarcone30(*part) = new_ptvarcone30;
        decorator_ptcone20_pt500(*part) = new_ptcone20_pt500;
        decorator_ptvarcone20_pt500(*part) = new_ptvarcone20_pt500;
        decorator_ptvarcone30_pt500(*part) = new_ptvarcone30_pt500;
    }
    return StatusCode::SUCCESS;
}
}
