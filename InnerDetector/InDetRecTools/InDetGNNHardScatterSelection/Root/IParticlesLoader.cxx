/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/IParticlesLoader.h"

#include "xAODBase/IParticle.h"

namespace InDetGNNHardScatterSelection {

    // factory for functions which return the sort variable we
    // use to order iparticles
    IParticlesLoader::IParticleSortVar IParticlesLoader::iparticleSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::IParticle Ip;
      typedef xAOD::Vertex Vertex;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* tp, const Vertex&) {return tp->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    IParticlesLoader::IParticlesLoader(
        const ConstituentsInputConfig & cfg
    ):
        IConstituentsLoader(cfg),
        m_iparticleSortVar(IParticlesLoader::iparticleSortVar(cfg.order)),
        m_customSequenceGetter(getter_utils::CustomSequenceGetter<xAOD::IParticle>(
          cfg.inputs))
    {
        const SG::AuxElement::ConstAccessor<PartLinks> acc("particleLinks");
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

    std::vector<const xAOD::IParticle*> IParticlesLoader::getIParticlesFromVertex(
        const xAOD::Vertex& vertex
    ) const
    {
        std::vector<std::pair<double, const xAOD::IParticle*>> particles;
        for (const xAOD::IParticle *tp : m_associator(vertex)) {
          particles.push_back({m_iparticleSortVar(tp, vertex), tp});
        }
        std::sort(particles.begin(), particles.end(), std::greater<>());
        std::vector<const xAOD::IParticle*> only_particles;
        for (const auto& particle: particles) {
          only_particles.push_back(particle.second);
        }
        return only_particles;
    }

    std::tuple<std::string, FlavorTagInference::Inputs, std::vector<const xAOD::IParticle*>> IParticlesLoader::getData(
      const xAOD::Vertex& vertex) const {
        IParticles sorted_particles = getIParticlesFromVertex(vertex);
        return std::make_tuple(m_config.output_name, m_customSequenceGetter.getFeats(vertex, sorted_particles), sorted_particles);
    }

    std::string IParticlesLoader::getName() const {
        return m_name;
    }
    ConstituentsType IParticlesLoader::getType() const {
        return m_config.type;
    }

}
