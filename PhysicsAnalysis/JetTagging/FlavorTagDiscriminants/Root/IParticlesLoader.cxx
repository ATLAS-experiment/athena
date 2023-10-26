/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/IParticlesLoader.h"
#include <iostream>


namespace FlavorTagDiscriminants {
    
    // factory for functions which return the sort variable we
    // use to order iparticles
    IParticlesLoader::IParticleSortVar IParticlesLoader::iparticleSortVar(
        ConstituentsSortOrder config, 
        const FTagOptions& options) 
    {
      typedef xAOD::IParticle Ip;
      typedef xAOD::Jet Jet;
      return [](const Ip* tp, const Jet&) {return tp->pt();};
      // switch(config) {
      //   case ConstituentsSortOrder::PT_DESCENDING:
      //     return [](const Ip* tp, const Jet&) {return tp->pt();};
      //   default: {
      //     throw std::logic_error("Unknown sort function");
      //   }
      // }
    } // end of track sort getter

    IParticlesLoader::IParticlesLoader(
        FTagConstituentsSequenceConfig cfg,
        const FTagOptions& options
    ):
        ConstituentsLoader(cfg),
        m_iparticleSortVar(IParticlesLoader::iparticleSortVar(cfg.order, options))
    {
        SG::AuxElement::ConstAccessor<PartLinks> acc("constituentLinks");
        m_associator = [acc](const xAOD::Jet& jet) -> IPV {
          IPV particles;
          for (const ElementLink<IPC>& link : acc(jet)){
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            const auto* particle = dynamic_cast<const xAOD::IParticle*>(*link);
            particles.push_back(particle);
          }
          return particles;
        };
    }

    std::vector<const xAOD::IParticle*> IParticlesLoader::getIParticlesFromJet(
        const xAOD::Jet& jet
    ) const
    {
        std::vector<std::pair<double, const xAOD::IParticle*>> particles;
        for (const xAOD::IParticle *tp : m_associator(jet)) {
          particles.push_back({m_iparticleSortVar(tp, jet), tp});
        }
        std::sort(particles.begin(), particles.end(), std::greater<>());
        std::vector<const xAOD::IParticle*> only_particles;
        only_particles.reserve(particles.size());
        for (const auto& trk: particles) {
            only_particles.push_back(trk.second);
        }
        return only_particles;
    }

    std::pair<std::string, input_pair> IParticlesLoader::getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const {
        std::vector<float> particle_feat(20); // (#tracks, #feats).flatten
        int num_tracks = 0;

        IParticles sorted_particles = getIParticlesFromJet(jet);
        for (auto el : sorted_particles){
            std::cout << el->pt() / 1000 << " ";
        }
        std::cout << std::endl;
        std::vector<int64_t> particle_feat_dim = {10, 2};

        return std::make_pair("particle_features", std::make_pair(particle_feat, particle_feat_dim));
    }
}