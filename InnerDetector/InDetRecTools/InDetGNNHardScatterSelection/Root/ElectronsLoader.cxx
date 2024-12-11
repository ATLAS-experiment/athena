/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetGNNHardScatterSelection/ElectronsLoader.h"

#include "xAODEgamma/Electron.h"

namespace InDetGNNHardScatterSelection {

    // factory for functions which return the sort variable we
    // use to order iparticles
    ElectronsLoader::ElectronSortVar ElectronsLoader::iparticleSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::Electron Ip;
      typedef xAOD::Vertex Vertex;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* tp, const Vertex&) {return tp->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    ElectronsLoader::ElectronsLoader(
        const ConstituentsInputConfig & cfg
    ):
        IConstituentsLoader(cfg),
        m_iparticleSortVar(ElectronsLoader::iparticleSortVar(cfg.order)),
        m_customSequenceGetter(getter_utils::CustomSequenceGetter<xAOD::Electron>(
          cfg.inputs))
    {
        const SG::AuxElement::ConstAccessor<PartLinks> acc("electronLinks");
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

    std::vector<const xAOD::Electron*> ElectronsLoader::getElectronsFromVertex(
        const xAOD::Vertex& vertex
    ) const
    {
        std::vector<std::pair<double, const xAOD::Electron*>> particles;
        for (const xAOD::Electron *tp : m_associator(vertex)) {
          particles.push_back({m_iparticleSortVar(tp, vertex), tp});
        }
        std::sort(particles.begin(), particles.end(), std::greater<>());
        std::vector<const xAOD::Electron*> only_particles;
        for (const auto& particle: particles) {
          only_particles.push_back(particle.second);
        }
        return only_particles;
    }

    std::tuple<std::string, FlavorTagDiscriminants::Inputs, std::vector<const xAOD::IParticle*>> ElectronsLoader::getData(
      const xAOD::Vertex& vertex) const {
        Electrons sorted_particles = getElectronsFromVertex(vertex);
        std::vector<const xAOD::IParticle*> sorted_particles_ip;
        for (const auto& p: sorted_particles) {
            sorted_particles_ip.push_back(p);
        }
        return std::make_tuple(m_config.output_name, m_customSequenceGetter.getFeats(vertex, sorted_particles), sorted_particles_ip);
    }

    std::string ElectronsLoader::getName() const {
        return m_name;
    }
    ConstituentsType ElectronsLoader::getType() const {
        return m_config.type;
    }

}
