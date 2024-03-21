/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/IParticlesLoader.h"
#include "xAODPFlow/FlowElement.h"
#include "FlavorTagDiscriminants/StringUtils.h"

namespace FlavorTagDiscriminants {
    
    // factory for functions which return the sort variable we
    // use to order iparticles
    IParticlesLoader::IParticleSortVar IParticlesLoader::iparticleSortVar(
        ConstituentsSortOrder config) 
    {
      typedef xAOD::IParticle Ip;
      typedef xAOD::Jet Jet;
      switch(config) {
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Ip* tp, const Jet&) {return tp->pt();};
        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of iparticle sort getter

    IParticlesLoader::IParticlesLoader(
        ConstituentsInputConfig cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_iparticleSortVar(IParticlesLoader::iparticleSortVar(cfg.order)),
        m_customSequenceGetter(getter_utils::CustomSequenceGetter<xAOD::IParticle>(
          cfg.inputs, options))
    {
        SG::AuxElement::ConstAccessor<PartLinks> acc("constituentLinks");
        m_associator = [acc](const xAOD::Jet& jet) -> IPV {
          IPV particles;
          for (const ElementLink<IPC>& link : acc(jet)){
            if (!link.isValid()) {
              throw std::logic_error("invalid particle link");
            }
            particles.push_back(*link);
          }
          return particles;
        };

        if (cfg.name.find("charged") != std::string::npos){
            m_isCharged = true;
        } else {
            m_isCharged = false;
        }
        m_used_remap = m_customSequenceGetter.getUsedRemap();
        m_name = cfg.name;
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
        for (const auto& particle: particles) {
          auto* flow = dynamic_cast<const xAOD::FlowElement*>(particle.second);
          const xAOD::IParticle* obj = nullptr;
          if (!flow){
            throw std::runtime_error("IParticlesLoader: Dynamic cast to FlowElement failed");
          }
          if ((flow->isCharged() != m_isCharged)) continue;
          else {
            if (m_isCharged){
              obj = flow->chargedObject(0);
            }
            else{
              obj = flow;
            }
          }
          if (!obj){
            continue;
          }
          only_particles.push_back(obj);
        }
        return only_particles;
    }

    std::tuple<std::string, input_pair, std::vector<const xAOD::IParticle*>> IParticlesLoader::getData(
      const xAOD::Jet& jet, 
      [[maybe_unused]] const SG::AuxElement& btag) const {
        IParticles sorted_particles = getIParticlesFromJet(jet);

        return std::make_tuple(m_config.output_name, m_customSequenceGetter.getFeats(jet, sorted_particles), sorted_particles);
    }

    FTagDataDependencyNames IParticlesLoader::getDependencies() const {
        return m_deps;
    }
    std::set<std::string> IParticlesLoader::getUsedRemap() const {
        return m_used_remap;
    }
    std::string IParticlesLoader::getName() const {
        return m_name;
    }
    ConstituentsType IParticlesLoader::getType() const {
        return m_config.type;
    }

}