/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/IParticlesLoader.h"
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "xAODBase/ObjectType.h"
#include "xAODPFlow/FlowElement.h"

#include "FlavorTagDiscriminants/customGetter.h"
#include <iostream>
#include <cxxabi.h>

namespace {

  std::string demangled(std::string const& sym) {
    std::unique_ptr<char, void(*)(void*)>
        name{abi::__cxa_demangle(sym.c_str(), nullptr, nullptr, nullptr), std::free};
    return {name.get()};
}


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
      input.name = varname;
      input.type = match_first(type_regexes, varname,
                                "track type matching");
      config.inputs.push_back(input);
    }
    return config;
  }
}

namespace FlavorTagDiscriminants {
    
    FTagConstituentsSequenceConfig createIParticleLoaderConfig(
      std::pair<std::string, std::vector<std::string>> iparticle_names
    ){
        // build the track inputs
        TypeRegexes trk_type_regexes {
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
          {"(deta|dphi)"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"phi|theta|qOverP"_r, ConstituentsEDMType::FLOAT},
          {"(phi|theta|qOverP)Uncertainty"_r, ConstituentsEDMType::CUSTOM_GETTER},
          {"leptonID"_r, ConstituentsEDMType::CHAR}
        };

        auto trk_config = get_iparticle_input_config(
          iparticle_names, trk_type_regexes);

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

    // factory for functions that build std::vector objects from
    // iparticle sequences
    std::pair<IParticlesLoader::SeqFromIParticles,std::set<std::string>> IParticlesLoader::seqFromIParticles(
        const FTagConstituentsInputConfig& cfg, 
        const FTagOptions& options)
    {
        const std::string prefix = options.track_prefix;
        switch (cfg.type) {
          case ConstituentsEDMType::INT: return {
              SequenceGetter<int, xAOD::IParticle>(cfg.name), {cfg.name}
            };
          case ConstituentsEDMType::FLOAT: return {
              SequenceGetter<float, xAOD::IParticle>(cfg.name), {cfg.name}
            };
          case ConstituentsEDMType::CHAR: return {
              SequenceGetter<char, xAOD::IParticle>(cfg.name), {cfg.name}
            };
          case ConstituentsEDMType::UCHAR: return {
              SequenceGetter<unsigned char, xAOD::IParticle>(cfg.name), {cfg.name}
            };
        //   case ConstituentsEDMType::CUSTOM_GETTER: {
        //     return internal::customNamedSeqGetterWithDeps(
        //       cfg.name, options.track_prefix);
        //   }
        //   default: {
        //     throw std::logic_error("Unknown EDM type for tracks");
        //   }
          default: return {
              SequenceGetter<float, xAOD::IParticle>("phi"), {"phi"}
            };
        }
    }

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

        if (cfg.name.find("charged") != std::string::npos){
            m_isCharged = true;
        } else {
            m_isCharged = false;
        }
        m_isCharged = true;

        std::map<std::string, std::string> remap = options.remap_scalar;
        std::set<std::string> used_remap;
        std::set<std::string> iparticle_data_deps;

        for (const FTagConstituentsInputConfig& input_cfg: cfg.inputs) {
            std::cout << input_cfg.name << std::endl;
            auto [seqGetter, deps] = seqFromIParticles(
            input_cfg, options);

            m_sequencesFromIParticles.push_back(seqGetter);
            iparticle_data_deps.merge(deps);
            if (auto h = remap.extract(input_cfg.name)){
              used_remap.insert(h.key());
            }
        }
        std::cout << "TEST IPARTICLE 3 " << std::endl;
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
            xAOD::Type::ObjectType objType = trk.second->type();
            if (objType != xAOD::Type::ObjectType::FlowElement) {
              std::cout << "objType: " << (int)objType << std::endl;
              std::cout << "not FlowElement" << std::endl;
              continue;
            }
            else{
              auto* flow = dynamic_cast<const xAOD::FlowElement*>(trk.second);
              if (!flow) continue;
              if ((flow->isCharged() != m_isCharged)) continue;
              else {
                std::cout << "TEST isCharged: " << flow->isCharged() << std::endl;
                if (m_isCharged){
                  only_particles.push_back(flow->chargedObject(0));
                }
                else{
                  size_t n_clusters = flow->nOtherObjects();
                  if (n_clusters == 0) continue;
                  if (n_clusters == 1)
                    only_particles.push_back(flow->otherObject(0));
                  else {
                    // if we have a few clusters take the one with the highest weight
                    auto ops = flow->otherObjectsAndWeights();
                    auto obj = std::max_element(
                      ops.begin(), ops.end(),
                      [](const auto& x, const auto& y) { return x.second < y.second; }
                      )->first;
                    only_particles.push_back(obj);
                  }
                }
              }
            }
        }
        return only_particles;
    }

    std::pair<std::string, input_pair> IParticlesLoader::getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const {
        // std::vector<float> particle_feat(20); // (#tracks, #feats).flatten
        std::vector<float> particle_feat;

        int num_iparticle_vars = static_cast<int>(m_sequencesFromIParticles.size());
        int num_iparticles = 0;
        SG::AuxElement::Accessor<float> m_getter("phi");

        IParticles sorted_particles = getIParticlesFromJet(jet);
        for (auto el : sorted_particles){
            xAOD::Type::ObjectType objType = el->type();
            std::cout << "objType: " << (int)objType << " ";
            std::cout << el->pt() / 1000 << " ";
            std::cout << m_getter.isAvailable(*el) << " ";
        }
        int iparticle_var_idx=0;
        for (const auto& seq_builder: m_sequencesFromIParticles) {
            auto double_vec = seq_builder(jet, sorted_particles).second;
            std::cout << "double_vec.size(): " << double_vec.size() << std::endl;
            if (iparticle_var_idx == 0){
              num_iparticles = static_cast<int>(double_vec.size());
              particle_feat.resize(num_iparticles * num_iparticle_vars);
            }

            for (unsigned int particle_idx=0; particle_idx < double_vec.size(); particle_idx++){
                particle_feat[iparticle_var_idx * num_iparticles + particle_idx] = double_vec[particle_idx];
            }
        }
        // std::cout << std::endl;
        std::vector<int64_t> particle_feat_dim = {num_iparticles, num_iparticle_vars};

        return std::make_pair("particle_features", std::make_pair(particle_feat, particle_feat_dim));
    }
}