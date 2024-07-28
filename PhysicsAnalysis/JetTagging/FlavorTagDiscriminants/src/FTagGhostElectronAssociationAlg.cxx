/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FTagGhostElectronAssociationAlg.h"
#include "xAODEgamma/ElectronFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include <vector>
#include "FourMomUtils/xAODP4Helpers.h"
#include "xAODEgamma/ElectronxAODHelpers.h"
#include "StoreGate/WriteDecorHandle.h"

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
        std::set<size_t> used_electrons_idx;

        for (auto jet : *jets) {
            IPLV electrons_in_jet;
            std::vector<const xAOD::TrackParticle*> jet_tracks;    
            jet_tracks = jet->getAssociatedObjects<const xAOD::TrackParticle>("GhostTrack");
            for (const auto electron : *electrons){
              if (used_electrons_idx.count(electron->index())) continue;
              auto track = xAOD::EgammaHelpers::getOriginalTrackParticle(electron);
              if(!track) continue;
              if(xAOD::P4Helpers::deltaR(*jet, *electron) > 0.5) continue;
              for (unsigned int i = 0; i < jet_tracks.size(); i++){
                if (track == jet_tracks[i]){
                  electrons_in_jet.push_back(ElementLink<IPC>(*electrons, electron->index()));
                  used_electrons_idx.insert(electron->index());
                  break;
                }
              }
            }
            electrons_out(*jet) = electrons_in_jet;
        }
        return StatusCode::SUCCESS;
    }
}