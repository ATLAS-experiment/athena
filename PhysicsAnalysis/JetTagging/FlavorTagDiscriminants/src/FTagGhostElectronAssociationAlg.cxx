/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FTagGhostElectronAssociationAlg.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include <cstddef>
#include <vector>
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "StoreGate/WriteDecorHandle.h"
#include <unordered_map>

namespace FlavorTagDiscriminants {

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
        std::unordered_multimap<const xAOD::TrackParticle*, size_t> track_to_electron_map;
        // Fill the unordered_multimap with track-electron associations
        for (const auto electron : *electrons) {
          auto track = xAOD::EgammaHelpers::getOriginalTrackParticle(electron);
          if (!track)
            continue;
          track_to_electron_map.insert({track, electron->index()});
        }
        for (auto jet : *jets) {
            IPLV electrons_in_jet;
            std::vector<const xAOD::TrackParticle*> jet_tracks;    
            jet_tracks = jet->getAssociatedObjects<const xAOD::TrackParticle>("GhostTrack");
            for (const auto& track : jet_tracks) {
                // Find all electrons associated with this track
                auto range = track_to_electron_map.equal_range(track);
                for (auto it = range.first; it != range.second; ++it) {
                    size_t electron_index = it->second;
                    electrons_in_jet.push_back(ElementLink<IPC>(*electrons, electron_index));
                }
            }
            electrons_out(*jet) = electrons_in_jet;
        }
        return StatusCode::SUCCESS;
    }
}
