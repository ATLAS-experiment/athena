/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/TracksLoader.h"

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

  FTagConstituentsSequenceConfig get_track_input_config(
    const std::pair<std::string, std::vector<std::string>> name_node,
    const TypeRegexes& type_regexes,
    const SortRegexes& sort_regexes,
    const TrkSelRegexes& select_regexes,
    const std::regex& re,
    const FlipTagConfig& flip_config) {
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

      input.flip_sign=false;
      if ((flip_config != FlipTagConfig::STANDARD) && std::regex_match(varname, re)){
        input.flip_sign=true;
      }
      config.inputs.push_back(input);
    }
    return config;
  }
}

namespace FlavorTagDiscriminants {

    FTagConstituentsSequenceConfig createTracksLoaderConfig(
      std::pair<std::string, std::vector<std::string>> trk_names,
      FlipTagConfig flip_config
    ){
        // some sequences also need to be sign-flipped. We apply this by
        // changing the input scaling and normalizations
        std::regex flip_sequences;
        if (flip_config == FlipTagConfig::FLIP_SIGN || flip_config == FlipTagConfig::NEGATIVE_IP_ONLY){
          flip_sequences=std::regex(".*signed_[dz]0.*");
        }
        if (flip_config == FlipTagConfig::SIMPLE_FLIP){
          flip_sequences=std::regex("(.*signed_[dz]0.*)|d0|z0SinTheta");
        }

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

        auto trk_config = get_track_input_config(
          trk_names, trk_type_regexes, trk_sort_regexes, trk_select_regexes,flip_sequences,flip_config);

        return trk_config;
    }


    // factory for functions which return the sort variable we
    // use to order tracks
    TracksLoader::TrackSortVar TracksLoader::trackSortVar(
        ConstituentsSortOrder config, 
        const FTagOptions& options) 
    {
      typedef xAOD::TrackParticle Tp;
      typedef xAOD::Jet Jet;
      BTagTrackIpAccessor aug(options.track_prefix);
      switch(config) {
        case ConstituentsSortOrder::ABS_D0_SIGNIFICANCE_DESCENDING:
          return [aug](const Tp* tp, const Jet&) {
            return std::abs(aug.d0(*tp) / aug.d0Uncertainty(*tp));
          };
        case ConstituentsSortOrder::D0_SIGNIFICANCE_DESCENDING:
          return [aug](const Tp* tp, const Jet& j) {
            return aug.getSignedIp(*tp, j).ip3d_signed_d0_significance;
          };
        case ConstituentsSortOrder::PT_DESCENDING:
          return [](const Tp* tp, const Jet&) {return tp->pt();};
        case ConstituentsSortOrder::ABS_D0_DESCENDING:
          return [aug](const Tp* tp, const Jet&) {
            return std::abs(aug.d0(*tp));
          };

        default: {
          throw std::logic_error("Unknown sort function");
        }
      }
    } // end of track sort getter

    // factory for functions that return true for tracks we want to
    // use, false for those we don't want
    std::pair<TracksLoader::TrackFilter,std::set<std::string>> TracksLoader::trackFilter(
        ConstituentsSelection config, 
        const FTagOptions& options) 
    {

        typedef xAOD::TrackParticle Tp;
        typedef SG::AuxElement AE;
        BTagTrackIpAccessor aug(options.track_prefix);
        auto data_deps = aug.getTrackIpDataDependencyNames();

        // make sure we record accessors as data dependencies
        std::set<std::string> track_deps;
        auto addAccessor = [&track_deps](const std::string& n) {
                             AE::ConstAccessor<unsigned char> a(n);
                             track_deps.insert(n);
                             return a;
                           };
        auto pix_hits = addAccessor("numberOfPixelHits");
        auto pix_holes = addAccessor("numberOfPixelHoles");
        auto pix_shared = addAccessor("numberOfPixelSharedHits");
        auto pix_dead = addAccessor("numberOfPixelDeadSensors");
        auto sct_hits = addAccessor("numberOfSCTHits");
        auto sct_holes = addAccessor("numberOfSCTHoles");
        auto sct_shared = addAccessor("numberOfSCTSharedHits");
        auto sct_dead = addAccessor("numberOfSCTDeadSensors");

        // data deps is all possible dependencies. We insert here to
        // avoid removing them from track_deps (as merge would).
        data_deps.insert(track_deps.begin(), track_deps.end());

        switch (config) {
        case ConstituentsSelection::ALL: return {[](const Tp*) {return true;}, {} };
          // the following numbers come from Nicole, Dec 2018:
          // pt > 1 GeV
          // abs(d0) < 1 mm
          // abs(z0 sin(theta)) < 1.5 mm
          // >= 7 si hits
          // <= 2 si holes
          // <= 1 pix holes
        case ConstituentsSelection::IP3D_2018:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 1e3) return false;
              if (std::abs(aug.d0(*tp)) >= 1.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 1.5) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
          // Tight track selection for DIPS upgrade config
          // abs(eta) < 4 
          // pt > 1 GeV
          // abs(d0) < 1 mm
          // abs(z0 sin(theta)) < 1.5 mm
          // No cuts for si hits, si holes and pix holes - only reconstruction selection is applied
        case ConstituentsSelection::DIPS_TIGHT_UPGRADE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 4.0) return false;
              if (tp->pt() <= 1e3) return false;
              if (std::abs(aug.d0(*tp)) >= 1.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 1.5) return false;
              return true;
            }, data_deps
          };
          // Loose track selection for DIPS upgrade config
          // abs(eta) < 4
          // pt > 0.5 GeV
          // abs(d0) < 3.5 mm
          // abs(z0 sin(theta)) < 5.0 mm
          // No cuts for si hits, si holes and pix holes - only reconstruction selection is applied
        case ConstituentsSelection::DIPS_LOOSE_UPGRADE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 4.0) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              return true;
            }, data_deps
          };
          // Loose track selection for DIPS
          // pt > 0.5 GeV
          // abs(d0) < 3.5 mm
          // abs(z0 sin(theta)) < 5.0 mm
          // >= 7 si hits
          // <= 2 si holes
          // <= 1 pix holes
        case ConstituentsSelection::DIPS_LOOSE_202102:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        case ConstituentsSelection::LOOSE_202102_NOIP:
          return {
            [=](const Tp* tp) {
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 7) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, track_deps
          };
        // R22_DEFAULT is similar to DIPS_LOOSE_202102, but modifies the min Si hit cut to 8,
        // which is the default tracking CP recommendation for r22, see
        // https://twiki.cern.ch/twiki/bin/view/AtlasProtected/TrackingCPRecsRun2R22#Selection_Criteria
        case ConstituentsSelection::R22_DEFAULT:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              double n_module_shared = (pix_shared(*tp) + sct_shared(*tp) / 2);
              if (n_module_shared > 1) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 3.5) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 8) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        // R22_LOOSE is similar to R22_DEFAULT, but removes the shared module cut 
        // and loosens the d0 cut
        case ConstituentsSelection::R22_LOOSE:
          return {
            [=](const Tp* tp) {
              // from the track selector tool
              if (std::abs(tp->eta()) > 2.5) return false;
              if (tp->pt() <= 0.5e3) return false;
              if (std::abs(aug.d0(*tp)) >= 5.0) return false;
              if (std::abs(aug.z0SinTheta(*tp)) >= 5.0) return false;
              if (pix_hits(*tp) + pix_dead(*tp) + sct_hits(*tp) + sct_dead(*tp) < 8) return false;
              if ((pix_holes(*tp) + sct_holes(*tp)) > 2) return false;
              if (pix_holes(*tp) > 1) return false;
              return true;
            }, data_deps
          };
        default:
          throw std::logic_error("unknown track selection function");
        }
    }

    // here we define filters for the "flip" taggers
    //
    // start by defining the raw functions, there's a factory
    // function below to convert the configuration enums to a
    // std::function
    std::vector<const xAOD::TrackParticle*> negativeIpOnly(
        BTagTrackIpAccessor& aug,
        const std::vector<const xAOD::TrackParticle*>& tracks,
        const xAOD::Jet& j) 
    {
        std::vector<const xAOD::TrackParticle*> filtered;
        // we want to reverse the order of the tracks as part of the
        // flipping
        for (auto ti = tracks.crbegin(); ti != tracks.crend(); ti++) {
          const xAOD::TrackParticle* tp = *ti;
          double sip = aug.getSignedIp(*tp, j).ip3d_signed_d0_significance;
          if (sip < 0) filtered.push_back(tp);
        }
        return filtered;
      }

    // factory function
    std::pair<TracksLoader::TrackSequenceFilter,std::set<std::string>> TracksLoader::flipFilter(
        const FTagOptions& options)
    {
        namespace ph = std::placeholders;  // for _1, _2, _3
        BTagTrackIpAccessor aug(options.track_prefix);
        switch(options.flip) {
        case FlipTagConfig::NEGATIVE_IP_ONLY:
          // flips order and removes tracks with negative IP
          return {
            std::bind(&negativeIpOnly, aug, ph::_1, ph::_2),
            aug.getTrackIpDataDependencyNames()
          };
        case FlipTagConfig::FLIP_SIGN:
          // Just flips the order
          return {
            [](const Tracks& tr, const xAOD::Jet& ) {
              return Tracks(tr.crbegin(), tr.crend());},
            {}
          };
        case FlipTagConfig::SIMPLE_FLIP:
          // Just flips the order
          return {
            [](const Tracks& tr, const xAOD::Jet& ) {
              return Tracks(tr.crbegin(), tr.crend());},
            {}
          };

        case FlipTagConfig::STANDARD:
          return {[](const Tracks& tr, const xAOD::Jet& ) { return tr; }, {}};
        default: {
          throw std::logic_error("Unknown flip config");
        }
        }
    }

    TracksLoader::TracksLoader(
        FTagConstituentsSequenceConfig cfg,
        const FTagOptions& options
    ):
        ConstituentsLoader(cfg),
        m_trackSortVar(TracksLoader::trackSortVar(cfg.order, options)),
        m_trackFilter(TracksLoader::trackFilter(cfg.selection, options).first),
        m_flipFilter(TracksLoader::flipFilter(options).first),
        m_customSequenceGetter(sequence_getter::CustomSequenceGetter(
          cfg.inputs, options))
    {
        // We have several ways to get tracks: either we retrieve an
        // IParticleContainer and cast the pointers to TrackParticle, or
        // we retrieve a TrackParticleContainer directly. Unfortunately
        // the way tracks are stored isn't consistent across the EDM, so
        // we allow configuration for both setups.
        //
        if (options.track_link_type == TrackLinkType::IPARTICLE) {
            SG::AuxElement::ConstAccessor<PartLinks> acc(options.track_link_name);
            m_associator = [acc](const SG::AuxElement& btag) -> TPV {
            TPV tracks;
            for (const ElementLink<IPC>& link: acc(btag)) {
                if (!link.isValid()) {
                throw std::logic_error("invalid particle link");
                }
                const auto* trk = dynamic_cast<const xAOD::TrackParticle*>(*link);
                if (!trk) {
                throw std::logic_error("iparticle does not cast to Track");
                }
                tracks.push_back(trk);
            }
            return tracks;
            };
        } else if (options.track_link_type == TrackLinkType::TRACK_PARTICLE){
            SG::AuxElement::ConstAccessor<TrackLinks> acc(options.track_link_name);
            m_associator = [acc](const SG::AuxElement& btag) -> TPV {
            TPV tracks;
            for (const ElementLink<TPC>& link: acc(btag)) {
                if (!link.isValid()) {
                throw std::logic_error("invalid track link");
                }
                tracks.push_back(*link);
            }
            return tracks;
            };
        } else {
            throw std::logic_error("Unknown TrackLinkType");
        }
        auto track_data_deps = trackFilter(cfg.selection, options).second;
        track_data_deps.merge(flipFilter(options).second);
        track_data_deps.merge(m_customSequenceGetter.getDependencies());
        deps.trackInputs.merge(track_data_deps);
        deps.bTagInputs.insert(options.track_link_name);
        used_remap = m_customSequenceGetter.getUsedRemap();
        name = cfg.name;
    }

    std::vector<const xAOD::TrackParticle*> TracksLoader::getTracksFromJet(
        const xAOD::Jet& jet,
        const SG::AuxElement& btag) const
    {
        std::vector<std::pair<double, const Track*>> tracks;
        for (const Track *tp : m_associator(btag)) {
            if (m_trackFilter(tp)) {
                tracks.push_back({m_trackSortVar(tp, jet), tp});
            };
        }
        std::sort(tracks.begin(), tracks.end(), std::greater<>());
        std::vector<const Track*> only_tracks;
        only_tracks.reserve(tracks.size());
        for (const auto& trk: tracks) {
            only_tracks.push_back(trk.second);
        }
        return only_tracks;
    }

    std::tuple<std::string, input_pair, std::vector<const xAOD::IParticle*>> TracksLoader::getData(
      const xAOD::Jet& jet, 
      const SG::AuxElement& btag) const {
        Tracks flipped_tracks;
        Tracks sorted_tracks = getTracksFromJet(jet, btag);
        std::vector<const xAOD::IParticle*> flipped_tracks_ip;

        flipped_tracks = m_flipFilter(sorted_tracks, jet);
        
        for (const auto& trk: flipped_tracks) {
            flipped_tracks_ip.push_back(dynamic_cast<const xAOD::IParticle*>(trk));
        }

        return std::make_tuple("track_features", m_customSequenceGetter.getFeats(jet, flipped_tracks_ip), flipped_tracks_ip);
    }

    std::tuple<char, std::map<std::string, std::vector<double>>> TracksLoader::getDL2Data(
      const xAOD::Jet& jet, 
      const SG::AuxElement& btag, 
      std::function<char(const Tracks&)> ip_checker) const{
      char invalid = 0;
      Tracks flipped_tracks;
      std::vector<const xAOD::IParticle*> flipped_tracks_ip;

      Tracks sorted_tracks = getTracksFromJet(jet, btag);
      if (ip_checker(sorted_tracks)) invalid = 1;
      flipped_tracks = m_flipFilter(sorted_tracks, jet);
      
      for (const auto& trk: flipped_tracks) {
          flipped_tracks_ip.push_back(dynamic_cast<const xAOD::IParticle*>(trk));
      }
      auto feats = m_customSequenceGetter.getDL2Feats(jet, flipped_tracks_ip);
      return std::make_tuple(invalid, feats);
    };

    FTagDataDependencyNames TracksLoader::getDependencies() const {
        return deps;
    }
    std::set<std::string> TracksLoader::getUsedRemap() const {
        return used_remap;
    }

    std::string TracksLoader::getName() const {
        return name;
    }
}