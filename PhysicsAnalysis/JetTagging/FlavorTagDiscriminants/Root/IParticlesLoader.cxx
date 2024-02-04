/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/IParticlesLoader.h"
// #include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "xAODPFlow/FlowElement.h"

#include <iostream>

namespace {

  // define a regex literal operator
  std::regex operator "" _r(const char* c, size_t /* length */) {
    return std::regex(c);
  }

  using FlavorTagDiscriminants::ConstituentsEDMType;
  using FlavorTagDiscriminants::ConstituentsSortOrder;
  using FlavorTagDiscriminants::ConstituentsSelection;
  using FlavorTagDiscriminants::FTagConstituentsSequenceConfig;
  using FlavorTagDiscriminants::FTagConstituentsInputConfig;
  using FlavorTagDiscriminants::FlipTagConfig;
  // ____________________________________________________________________
  // High level adapter stuff
  //
  // We define a few structures to map variable names to type, default
  // value, etc. These are only used by the high level interface.
  //
  typedef std::vector<std::pair<std::regex, ConstituentsEDMType> > TypeRegexes;
  typedef std::vector<std::pair<std::regex, ConstituentsSortOrder> > SortRegexes;
  typedef std::vector<std::pair<std::regex, ConstituentsSelection> > SelRegexes;

  // Function to map the regex + list of inputs to variable config,
  // this time for sequence inputs.
  std::vector<FTagConstituentsSequenceConfig> get_iparticle_input_config(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& names,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const SelRegexes& select_regexes,
    const std::regex& re);


  //_______________________________________________________________________
  // Implementation of the above functions
  //

  template <typename T>
  T match_first(const std::vector<std::pair<std::regex, T> >& regexes,
                const std::string& var_name,
                const std::string& context) {
    for (const auto& pair: regexes) {
      if (std::regex_match(var_name, pair.first)) {
        return pair.second;
      }
    }
    throw std::logic_error(
      "no regex match found for input variable '" + var_name + "' in "
      + context);
  }

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
      // input.name = varname;
      input.type = match_first(type_regexes, input.name,
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
        // build the track inputs
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
          // throw std::logic_error("Unknown sort function");
          return [](const Ip* tp, const Jet&) {return tp->pt();};
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
        std::cout << "TEST: IParticlesLoader loaded " << std::endl;
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
            std::cout << "TEST: obj is nullptr" << std::endl;
            continue;
          }
          only_particles.push_back(obj);
        }
        return only_particles;
    }

    std::pair<std::string, input_pair> IParticlesLoader::getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const {
        IParticles sorted_particles = getIParticlesFromJet(jet);

        return std::make_pair("flow_features", m_customSequenceGetter.getFeats(jet, sorted_particles));
    }

    FTagDataDependencyNames IParticlesLoader::getDependencies() const {
        return deps;
    }
    std::set<std::string> IParticlesLoader::getUsedRemap() const {
        return used_remap;
    }

}