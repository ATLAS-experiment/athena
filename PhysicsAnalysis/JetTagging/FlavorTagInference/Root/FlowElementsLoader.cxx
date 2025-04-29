/*
Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/FlowElementsLoader.h"
#include "xAODBase/IParticle.h"
#include "xAODPFlow/FlowElement.h"
#include <vector>

namespace FlavorTagInference {
    
    // factory for functions which return the sort variable we
    // use to order flow elements
    FlowElementsLoader::FlowElementSortVar FlowElementsLoader::flowElementSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::FlowElement FE;
      typedef xAOD::Jet Jet;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const FE* p, const Jet&) {return p->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of FlowElements sort getter

    FlowElementsLoader::FlowElementsLoader(
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_flowElementSortVar(FlowElementsLoader::flowElementSortVar(cfg.order)),
        m_seqGetter(getter_utils::SeqGetter<xAOD::FlowElement>(
          cfg.inputs, options))
    {
        static const SG::AuxElement::ConstAccessor<PartLinks> acc("constituentLinks");
        m_associator = [](const xAOD::Jet& jet) -> FEV {
          FEV particles;
          for (const ElementLink<IPC>& link : acc(jet)){
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            const auto* flow = dynamic_cast<const xAOD::FlowElement*>(*link);
            if (!flow){
              throw std::runtime_error("FlowsLoader: Dynamic cast to FlowElement failed");
            }
            particles.push_back(flow);
          }
          return particles;
        };

        m_used_remap = m_seqGetter.getUsedRemap();
        m_name = cfg.name;
    }

    std::vector<const xAOD::FlowElement*> FlowElementsLoader::getFlowElementsFromJet(
        const xAOD::Jet& jet
    ) const
    {
        std::vector<std::pair<double, const xAOD::FlowElement*>> particles;
        for (const xAOD::FlowElement *p : m_associator(jet)) {
          particles.push_back({m_flowElementSortVar(p, jet), p});
        }
        std::sort(particles.begin(), particles.end(), std::greater<>());
        std::vector<const xAOD::FlowElement*> only_particles;
        for (const auto& particle: particles) {
          only_particles.push_back(particle.second);
        }
        return only_particles;
    }

    std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>> FlowElementsLoader::getData(
      const xAOD::Jet& jet, 
      [[maybe_unused]] const SG::AuxElement& btag) const {
        FlowElements sorted_flows = getFlowElementsFromJet(jet);

        // We return a dummy vector of IParticles as we don't decorate flow elements
        return std::make_tuple(m_config.output_name, m_seqGetter.getFeats(jet, sorted_flows), std::vector<const xAOD::IParticle *>{});
    }

    const FTagDataDependencyNames& FlowElementsLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& FlowElementsLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& FlowElementsLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& FlowElementsLoader::getType() const {
        return m_config.type;
    }

}
