/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"
#include "FlavorTagDiscriminants/IParticlesLoader.h"

#include "FlavorTagDiscriminants/customGetter.h"

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
  typedef std::vector<std::pair<std::regex, ConstituentsSelection> > TrkSelRegexes;

  // Function to map the regex + list of inputs to variable config,
  // this time for sequence inputs.
  std::vector<FTagConstituentsSequenceConfig> get_iparticle_input_config(
    const std::vector<std::pair<std::string, std::vector<std::string>>>& names,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const TrkSelRegexes& select_regexes,
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
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const TrkSelRegexes& select_regexes) {
    FTagConstituentsSequenceConfig config;
    config.name = name_node.first;
    config.order = match_first(sort_regexes, name_node.first,
                              "track order matching");
    config.selection = match_first(select_regexes, name_node.first,
                                  "track selection matching");
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
    
    FTagConstituentsSequenceConfig createIParticlesLoaderConfig(
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
        // We have a number of special naming conventions to sort and
        // filter tracks. The track nodes should be named according to
        //
        // tracks_<selection>_<sort-order>
        //
        SortRegexes trk_sort_regexes {
          {".*absSd0sort"_r, ConstituentsSortOrder::ABS_D0_SIGNIFICANCE_DESCENDING},
          {".*sd0sort"_r, ConstituentsSortOrder::D0_SIGNIFICANCE_DESCENDING},
          {".*ptsort"_r, ConstituentsSortOrder::PT_DESCENDING},
          {".*absD0DescendingSort"_r, ConstituentsSortOrder::ABS_D0_DESCENDING},
        };
        TrkSelRegexes trk_select_regexes {
          {".*_ip3d_.*"_r, ConstituentsSelection::IP3D_2018},
          {".*_dipsTightUpgrade_.*"_r, ConstituentsSelection::DIPS_TIGHT_UPGRADE},
          {".*_dipsLooseUpgrade_.*"_r, ConstituentsSelection::DIPS_LOOSE_UPGRADE},
          {".*_all_.*"_r, ConstituentsSelection::ALL},
          {".*_dipsLoose202102_.*"_r, ConstituentsSelection::DIPS_LOOSE_202102},
          {".*_loose202102NoIpCuts_.*"_r, ConstituentsSelection::LOOSE_202102_NOIP},
          {".*_r22default_.*"_r, ConstituentsSelection::R22_DEFAULT},
          {".*_r22loose_.*"_r, ConstituentsSelection::R22_LOOSE},
        };

        auto trk_config = get_iparticle_input_config(
          iparticle_names, trk_type_regexes, trk_sort_regexes, trk_select_regexes);

        return trk_config;
    }


    // factory for functions which return the sort variable we
    // use to order tracks
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
    } // end of track sort getter

    // factory for functions that build std::vector objects from
    // track sequences
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
          // case ConstituentsEDMType::CUSTOM_GETTER: {
          //   return internal::customNamedSeqGetterWithDeps(
          //     cfg.name, options.track_prefix);
          // }
          default: {
            throw std::logic_error("Unknown EDM type for tracks");
          }
        }
    }

    template <typename T>
    struct AssociationConfig
    {
      T accessor;
      std::string name;
    };

    template <typename T>
    AssociationConfig<T> getAssociationConfig(
      T accessor,
      const std::string& name)
    {
      return AssociationConfig<T> {accessor, name};
    }


    auto getFlowAssociations() {

      using Type = xAOD::FlowElement;
      using PartLinks = std::vector<ElementLink<xAOD::IParticleContainer>>;

      // track getter
      auto track = [](const Type* el) -> const xAOD::TrackParticle* {
        if (!el) throw std::runtime_error("missing flow object");
        size_t n_charged = el->nChargedObjects();
        // see AFT-619: neutral UFO constituents have n_charged == 1
        if (not el->isCharged()) return nullptr;
        if (n_charged != 1) throw std::runtime_error(
          "n charged > 1, found " + std::to_string(n_charged));
        const auto* obj = el->chargedObject(0);
        if (!obj) throw std::runtime_error("charged object missing");
        const auto* track = dynamic_cast<const xAOD::TrackParticle*>(obj);
        if (!track) throw std::runtime_error("can't cast to track particle");
        return track;
      };

      // cluster getter
      auto cluster = [](const Type* el) -> const xAOD::CaloCluster* {
        if (!el) throw std::runtime_error("missing flow object");
        size_t n_clusters = el->nOtherObjects();
        if (n_clusters == 0) return nullptr;
        const xAOD::IParticle* obj = nullptr;
        if (n_clusters == 1) {
          obj = el->otherObject(0);
        } else {
          // if we have a few clusters take the one with the highest weight
          auto ops = el->otherObjectsAndWeights();
          obj = std::max_element(
            ops.begin(), ops.end(),
            [](const auto& x, const auto& y) { return x.second < y.second; }
            )->first;
        }
        if (!obj) { // very rare: handle slimmed negative energy clusters
          return nullptr;
        }
        const auto* cluster = dynamic_cast<const xAOD::CaloCluster*>(obj);
        if (!cluster) throw std::runtime_error("can't cast to calo cluster");
        return cluster;
      };

      // build the association tuple
      return std::tuple{
        getAssociationConfig(
          [track] (const Type* e) -> const xAOD::TrackParticle* {
            return track(e);
          },"track"),
        getAssociationConfig(
          [cluster](const Type* e) -> const xAOD::CaloCluster* {
            return cluster(e);
          }, "cluster")
      };
    }


    IParticlesLoader::IParticlesLoader(
        FTagConstituentsSequenceConfig cfg,
        const FTagOptions& options
    ):
        ConstituentsLoader(cfg),
        m_iparticleSortVar(IParticlesLoader::iparticleSortVar(cfg.order, options))
    {
        // We have several ways to get tracks: either we retrieve an
        // IParticleContainer and cast the pointers to TrackParticle, or
        // we retrieve a TrackParticleContainer directly. Unfortunately
        // the way tracks are stored isn't consistent across the EDM, so
        // we allow configuration for both setups.
        //
        std::cout << "TEST TRACK 1 " << std::endl;
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
        std::cout << "TEST TRACK 2 " << std::endl;
        FTagDataDependencyNames deps;
        std::map<std::string, std::string> remap = options.remap_scalar;
        std::set<std::string> used_remap;

        std::set<std::string> particle_data_deps;
        for (const FTagConstituentsInputConfig& input_cfg: cfg.inputs) {
            auto [seqGetter, seq_deps] = seqFromIParticles(
            input_cfg, options);

            m_sequencesFromIParticles.push_back(seqGetter);
            particle_data_deps.merge(seq_deps);
            if (auto h = remap.extract(input_cfg.name)){
              used_remap.insert(h.key());
            }
        }
        std::cout << "TEST TRACK 3 " << std::endl;
        deps.trackInputs.merge(particle_data_deps);
        deps.bTagInputs.insert(options.track_link_name);
    }

    std::vector<const xAOD::IParticle*> IParticlesLoader::getIParticlesFromJet(
        const xAOD::Jet& jet,
        const SG::AuxElement& btag) const
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
        std::vector<float> particle_feat; // (#tracks, #feats).flatten

        int num_particle_vars = static_cast<int>(m_sequencesFromIParticles.size());
        int num_tracks = 0;

        IParticles sorted_tracks = getIParticlesFromJet(jet, btag);

        int particle_var_idx=0;
        for (const auto& seq_builder: m_sequencesFromIParticles) {
            auto double_vec = seq_builder(jet, sorted_tracks).second;

            if (particle_var_idx==0){
                num_tracks = static_cast<int>(double_vec.size());
                particle_feat.resize(num_tracks * num_particle_vars);
            }

            // need to transpose + flatten
            for (unsigned int particle_idx=0; particle_idx<double_vec.size(); particle_idx++){
            particle_feat.at(particle_idx*num_particle_vars + particle_var_idx)
                = double_vec.at(particle_idx);
            }
            particle_var_idx++;
        }
        std::vector<int64_t> particle_feat_dim = {num_tracks, num_particle_vars};

        return std::make_pair("particle_features", std::make_pair(particle_feat, particle_feat_dim));
    }
}