/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FTagGhostLeptonAssociationAlg.h"
#include "xAODTracking/TrackParticleFwd.h"
#include <cstddef>
#include <vector>
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "StoreGate/WriteDecorHandle.h"
#include <unordered_map>

namespace {

    template<typename T>
    const xAOD::TrackParticle* getTrack(const T* lepton);

    template<>
    const xAOD::TrackParticle* getTrack<xAOD::Muon>(const xAOD::Muon* lepton) {
        return lepton->trackParticle(xAOD::Muon::InnerDetectorTrackParticle);
    }

    template<>
    const xAOD::TrackParticle* getTrack<xAOD::Electron>(const xAOD::Electron* lepton) {
        return xAOD::EgammaHelpers::getOriginalTrackParticle(lepton);
    }



    using IPC = xAOD::IParticleContainer;
    using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;
    template<typename T, typename EC>
    void associate_leptons_to_jets(
        const SG::ReadHandle<xAOD::JetContainer>& jets,
        const SG::ReadHandle<EC>& leptons, 
        SG::WriteDecorHandle<IPC, IPLV>& leptons_out
    ){
        std::unordered_multimap<const xAOD::TrackParticle*, size_t> track_to_lepton_map;
        // Fill the unordered_multimap with track-lepton associations
        for (const auto lepton : *leptons.get()) {
            const xAOD::TrackParticle* track = getTrack<T>(lepton);
            if (!track)
                continue;
            track_to_lepton_map.insert({track, lepton->index()});
        }
        for (auto jet : *jets.get()) {
            IPLV leptons_in_jet;
            std::vector<const xAOD::TrackParticle*> jet_tracks;    
            jet_tracks = jet->getAssociatedObjects<const xAOD::TrackParticle>("GhostTrack");
            for (const auto& track : jet_tracks) {
                // Find all leptons associated with this track
                auto range = track_to_lepton_map.equal_range(track);
                for (auto it = range.first; it != range.second; ++it) {
                    size_t lepton_index = it->second;
                    leptons_in_jet.push_back(ElementLink<IPC>(*leptons.get(), lepton_index));
                }
            }
            leptons_out(*jet) = leptons_in_jet;
        }
    }
}

namespace FlavorTagDiscriminants {

    FTagGhostMuonAssociationAlg::FTagGhostMuonAssociationAlg(const std::string& name,
                        ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode FTagGhostMuonAssociationAlg::initialize() {
        ATH_MSG_DEBUG( "Initializing " << name() << "... " );

        // Initialize Container keys
        ATH_MSG_DEBUG( "Initializing containers:"            );
        ATH_MSG_DEBUG( "    ** " << m_JetContainerKey      );
        ATH_MSG_DEBUG( "    ** " << m_MuonContainerKey      );
        ATH_MSG_DEBUG( "    ** " << m_MuonsOutKey      );

        ATH_CHECK( m_JetContainerKey.initialize() );
        ATH_CHECK( m_MuonContainerKey.initialize() );
        ATH_CHECK( m_MuonsOutKey.initialize() );

        return StatusCode::SUCCESS;
    }

    StatusCode FTagGhostMuonAssociationAlg::execute(const EventContext& ctx) const {
        ATH_MSG_DEBUG( "Executing " << name() << "... " );

        using EC = xAOD::MuonContainer;
        using IPC = xAOD::IParticleContainer;
        using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;

        // read collections
        SG::ReadHandle<EC> muons(m_MuonContainerKey, ctx);
        ATH_CHECK(muons.isValid());
        ATH_MSG_DEBUG( "Retrieved " << muons->size() << " muons..." );

        SG::ReadHandle<xAOD::JetContainer> jets(m_JetContainerKey, ctx);
        ATH_CHECK(jets.isValid());
        ATH_MSG_DEBUG( "Retrieved " << jets->size() << " jets..." );

        SG::WriteDecorHandle<IPC, IPLV> muons_out(m_MuonsOutKey, ctx);
        ATH_CHECK(muons_out.isValid());
        associate_leptons_to_jets<xAOD::Muon, xAOD::MuonContainer>(jets, muons, muons_out);
        ATH_MSG_DEBUG( "Associated muons to jets..." );

        return StatusCode::SUCCESS;
    }

    FTagGhostElectronAssociationAlg::FTagGhostElectronAssociationAlg(const std::string& name,
                          ISvcLocator* pSvcLocator ) : AthReentrantAlgorithm(name, pSvcLocator) {}

    StatusCode FTagGhostElectronAssociationAlg::initialize() {
        ATH_MSG_DEBUG( "Initializing " << name() << "... " );

        // Initialize Container keys
        ATH_MSG_DEBUG( "Initializing containers:"            );
        ATH_MSG_DEBUG( "    ** " << m_JetContainerKey      );
        ATH_MSG_DEBUG( "    ** " << m_ElectronContainerKey      );
        ATH_MSG_DEBUG( "    ** " << m_ElectronsOutKey      );

        ATH_CHECK( m_JetContainerKey.initialize() );
        ATH_CHECK( m_ElectronContainerKey.initialize() );
        ATH_CHECK( m_ElectronsOutKey.initialize() );

        return StatusCode::SUCCESS;
    }

    StatusCode FTagGhostElectronAssociationAlg::execute(const EventContext& ctx) const {
        ATH_MSG_DEBUG( "Executing " << name() << "... " );

        using EC = xAOD::ElectronContainer;
        using IPC = xAOD::IParticleContainer;
        using IPLV = std::vector<ElementLink<xAOD::IParticleContainer>>;

        // read collections
        SG::ReadHandle<EC> electrons(m_ElectronContainerKey, ctx);
        ATH_CHECK(electrons.isValid());
        ATH_MSG_DEBUG( "Retrieved " << electrons->size() << " electrons..." );

        SG::ReadHandle<xAOD::JetContainer> jets(m_JetContainerKey, ctx);
        ATH_CHECK(jets.isValid());
        ATH_MSG_DEBUG( "Retrieved " << jets->size() << " jets..." );

        SG::WriteDecorHandle<IPC, IPLV> electrons_out(m_ElectronsOutKey, ctx);
        ATH_CHECK(electrons_out.isValid());
        associate_leptons_to_jets<xAOD::Electron, EC>(jets, electrons, electrons_out);
        ATH_MSG_DEBUG( "Associated electrons to jets..." );

        return StatusCode::SUCCESS;
    }
}
