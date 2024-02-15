/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/IParticlesLoader.h"
#include "xAODPFlow/FlowElement.h"
#include "FlavorTagDiscriminants/StringUtils.h"

namespace {
  using namespace FlavorTagDiscriminants;

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  // ____________________________________________________________________
  //
  // We define a few structures to map variable names to type, default
  // value, etc.
  //
  typedef std::vector<std::pair<std::regex, ConstituentsEDMType> > TypeRegexes;

  FTagConstituentsSequenceConfig get_iparticle_input_config(
    const std::pair<std::string, std::vector<std::string>> name_node,
    const TypeRegexes& type_regexes) {
    FTagConstituentsSequenceConfig config;
    config.name = name_node.first;
    config.order = ConstituentsSortOrder::PT_DESCENDING;
    for (const auto& varname: name_node.second) {
      FTagConstituentsInputConfig input;
      size_t pos = varname.find("flow_");
      if (pos != std::string::npos){
        input.name = varname.substr(pos+5);
      }
      else{
        input.name = varname;
      }
      input.type = str::match_first(type_regexes, input.name,
                                "track type matching");
      config.inputs.push_back(input);
    }
    return config;
  }
}

namespace FlavorTagDiscriminants {
    
    FTagConstituentsSequenceConfig createIParticlesLoaderConfig(
      std::pair<std::string, std::vector<std::string>> iparticle_names
    ){
        // build the iparticle inputs
        TypeRegexes var_type_regexes {
          // Some innermost / next-to-innermost hit variables had a different
          // definition in 21p9, recomputed here with customGetter to reuse
          // existing training
          // EDMType picked correspond to the first matching regex
          {"numberOf.*21p9"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"numberOf.*"_r, ConstituentsEDMType::UCHAR},
          {"btagIp_(d0|z0SinTheta)Uncertainty"_r, ConstituentsEDMType::FLOAT},
          {"(numberDoF|chiSquared|qOverP|theta)"_r, ConstituentsEDMType::FLOAT},
          {"(^.*[_])?(d|z)0.*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(log_)?(ptfrac|dr|pt).*"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"(deta|dphi|energy)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"phi|theta|qOverP"_r, ConstituentsEDMType::FLOAT},
          {"(phi|theta|qOverP)Uncertainty"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"leptonID"_r, ConstituentsEDMType::CHAR}
        };

        auto trk_config = get_iparticle_input_config(
          iparticle_names, var_type_regexes);

        return trk_config;
    }


    // factory for functions which return the sort variable we
    // use to order iparticles
    IParticlesLoader::IParticleSortVar IParticlesLoader::iparticleSortVar(
        ConstituentsSortOrder config, 
        const FTagOptions& options) 
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
        FTagConstituentsSequenceConfig cfg,
        const FTagOptions& options
    ):
        ConstituentsLoader(cfg),
        m_iparticleSortVar(IParticlesLoader::iparticleSortVar(cfg.order, options)),
        m_customSequenceGetter(sequence_getter::CustomSequenceGetter(
          cfg.inputs, options))
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

        if (cfg.name.find("charged") != std::string::npos){
            m_isCharged = true;
        } else {
            m_isCharged = false;
        }
        used_remap = m_customSequenceGetter.getUsedRemap();
        name = cfg.name;
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
          if (!flow) continue;
          if ((flow->isCharged() != m_isCharged)) continue;
          else {
            if (m_isCharged){
              obj = flow->chargedObject(0);
            }
            else{
              obj = dynamic_cast<const xAOD::IParticle*>(flow);
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
      const SG::AuxElement& btag) const {
        IParticles sorted_particles = getIParticlesFromJet(jet);

        return std::make_tuple("flow_features", m_customSequenceGetter.getFeats(jet, sorted_particles), sorted_particles);
    }

    FTagDataDependencyNames IParticlesLoader::getDependencies() const {
        return deps;
    }
    std::set<std::string> IParticlesLoader::getUsedRemap() const {
        return used_remap;
    }
    std::string IParticlesLoader::getName() const {
        return name;
    }

}