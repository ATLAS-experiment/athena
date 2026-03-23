/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FTagGhostLeptonAssociationAlg.h"
#include "xAODTracking/TrackParticleFwd.h"
#include <algorithm>
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
    void associate_leptons_to_jets_dR(
        const SG::ReadHandle<xAOD::JetContainer>& jets,
        const SG::ReadHandle<EC>& leptons, 
        SG::WriteDecorHandle<IPC, IPLV>& leptons_out
    ){
        const xAOD::JetContainer& jet_container = *jets.get();
        const EC& lepton_container = *leptons.get();

        std::unordered_map<const xAOD::Jet*, IPLV> leptons_in_jets;
        leptons_in_jets.reserve(jet_container.size());
        for (const xAOD::Jet* jet : jet_container) {
            leptons_in_jets.emplace(jet, IPLV{});
        }

        for (const T* lepton : lepton_container) {
            std::vector<std::pair<double, const xAOD::Jet*>> matched_jets;
            matched_jets.reserve(jet_container.size());
            for (const xAOD::Jet* jet : jet_container) {
                const double dR = jet->p4().DeltaR(lepton->p4());
                if (dR > 0.4) {
                    continue;
                }
                matched_jets.emplace_back(dR, jet);
            }

            std::sort(
                matched_jets.begin(),
                matched_jets.end(),
                [](const auto& lhs, const auto& rhs) {
                    return lhs.first < rhs.first;
                });

            if (matched_jets.empty()) {
                continue;
            }

            leptons_in_jets[matched_jets.front().second].push_back(
                ElementLink<IPC>(*leptons.get(), lepton->index()));
        }

        for (const xAOD::Jet* jet : jet_container) {
            leptons_out(*jet) = leptons_in_jets.at(jet);
        }
    }

    template<typename T, typename EC>
    void associate_leptons_to_jets_ghost(
        const SG::ReadHandle<xAOD::JetContainer>& jets,
        const SG::ReadHandle<EC>& leptons, 
        SG::WriteDecorHandle<IPC, IPLV>& leptons_out
    ){
        std::unordered_multimap<const xAOD::TrackParticle*, size_t> track_to_lepton_map;
        // Fill the unordered_multimap with track-lepton associations
        for (const T* lepton : *leptons.get()) {
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
        ATH_MSG_DEBUG( "    ** " << m_doConeMatching      );

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
        if (m_doConeMatching) {
            associate_leptons_to_jets_dR<xAOD::Muon, EC>(
                jets, muons, muons_out);
        } else {
            associate_leptons_to_jets_ghost<xAOD::Muon, EC>(
                jets, muons, muons_out);
        }
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
        associate_leptons_to_jets_ghost<xAOD::Electron, EC>(
            jets, electrons, electrons_out);
        ATH_MSG_DEBUG( "Associated electrons to jets..." );

        return StatusCode::SUCCESS;
    }
}
