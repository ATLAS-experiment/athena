/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/TracksLoader.h"

#include "xAODTracking/TrackParticle.h"

namespace InDetGNNHardScatterSelection {


    // factory for functions which return the sort variable we
    // use to order iparticles
    TracksLoader::TrackParticleSortVar TracksLoader::iparticleSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::TrackParticle Ip;
      typedef xAOD::Vertex Vertex;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* tp, const Vertex&) {return tp->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    TracksLoader::TracksLoader(
        ConstituentsInputConfig cfg
    ):
        IConstituentsLoader(cfg),
        m_iparticleSortVar(TracksLoader::iparticleSortVar(cfg.order)),
        m_customSequenceGetter(getter_utils::CustomSequenceGetter<xAOD::TrackParticle>(
          cfg.inputs))
    {
        const SG::AuxElement::ConstAccessor<PartLinks> acc("trackParticleLinks");
        m_associator = [acc](const xAOD::Vertex& vertex) -> IPV {
          IPV particles;
          for (const ElementLink<IPC>& link : acc(vertex)){
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            particles.push_back(*link);
          }
          return particles;
        };
        m_name = cfg.name;
    }

    std::vector<const xAOD::TrackParticle*> TracksLoader::getTrackParticlesFromVertex(
        const xAOD::Vertex& vertex
    ) const
    {
        std::vector<std::pair<double, const xAOD::TrackParticle*>> particles;
        for (const xAOD::TrackParticle *tp : m_associator(vertex)) {
          particles.push_back({m_iparticleSortVar(tp, vertex), tp});
        }
        std::sort(particles.begin(), particles.end(), std::greater<>());
        std::vector<const xAOD::TrackParticle*> only_particles;
        for (const auto& particle: particles) {
          only_particles.push_back(particle.second);
        }
        return only_particles;
    }

    std::tuple<std::string, FlavorTagInference::Inputs, std::vector<const xAOD::IParticle*>> TracksLoader::getData(
      const xAOD::Vertex& vertex) const {
        TrackParticles sorted_particles = getTrackParticlesFromVertex(vertex);
        std::vector<const xAOD::IParticle*> sorted_particles_ip;
        for (const auto& p: sorted_particles) {
            sorted_particles_ip.push_back(p);
        }
        return std::make_tuple(m_config.output_name, m_customSequenceGetter.getFeats(vertex, sorted_particles), sorted_particles_ip);
    }

    std::string TracksLoader::getName() const {
        return m_name;
    }
    ConstituentsType TracksLoader::getType() const {
        return m_config.type;
    }

}
