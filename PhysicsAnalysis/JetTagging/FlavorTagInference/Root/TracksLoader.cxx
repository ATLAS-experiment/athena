/*
Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/FlipTagEnums.h"
#include "FlavorTagInference/AssociationEnums.h"
#include "FlavorTagInference/TracksLoader.h"
#include "FlavorTagInference/StringUtils.h"

namespace FlavorTagInference {
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
              const xAOD::ParametersCovMatrix_t cov = tp->definingParametersCovMatrix();
              if (std::sqrt(cov(2, 2)) > 65504) return false;
              if (std::sqrt(cov(3, 3)) > 65504) return false;
              if (std::sqrt(cov(4, 4)) > 65504) return false;
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
    Tracks negativeIpOnly(BTagTrackIpAccessor& aug, const Tracks& tracks, const xAOD::Jet& j) 
    {
        Tracks filtered;
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
    std::pair<TracksLoader::TrackSequenceFilter,std::set<std::string>> TracksLoader::trackFlipper(
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
        const ConstituentsInputConfig& cfg,
        const FTagOptions& options
    ):
        IConstituentsLoader(cfg),
        m_trackSortVar(TracksLoader::trackSortVar(cfg.order, options)),
        m_trackFilter(TracksLoader::trackFilter(cfg.selection, options).first),
        m_trackFlipper(TracksLoader::trackFlipper(options).first),
        m_seqGetter(getter_utils::SeqGetter<xAOD::TrackParticle>(
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
            m_associator = [acc](const SG::AuxElement& btag) -> Tracks {
            Tracks tracks;
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
            m_associator = [acc](const SG::AuxElement& btag) -> Tracks {
            Tracks tracks;
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
        track_data_deps.merge(trackFlipper(options).second);
        track_data_deps.insert(m_seqGetter.getDependencies().begin(),
                               m_seqGetter.getDependencies().end());
        m_deps.trackInputs.merge(track_data_deps);
        m_deps.bTagInputs.insert(options.track_link_name);
        m_used_remap = m_seqGetter.getUsedRemap();
        m_name = cfg.name;
    }

    Tracks TracksLoader::getTracksFromJet(
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

    std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>>
    TracksLoader::getData(const xAOD::Jet& jet, [[maybe_unused]] const SG::AuxElement& btag) const
    {
        Tracks sorted_tracks = getTracksFromJet(jet, btag);
        Tracks flipped_tracks = m_trackFlipper(sorted_tracks, jet);
        
        // cast to IParticle for aux task decoration
        // this could probably be templated since we cast back again later
        std::vector<const xAOD::IParticle*> flipped_iparticles;
        for (const auto& trk: flipped_tracks) {
          flipped_iparticles.push_back(trk);
        }

        Inputs features = m_seqGetter.getFeats(jet, flipped_tracks);
        return std::make_tuple(m_config.output_name, features, flipped_iparticles);
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
      flipped_tracks = m_trackFlipper(sorted_tracks, jet);
      
      auto feats = m_seqGetter.getDL2Feats(jet, flipped_tracks);
      return std::make_tuple(invalid, feats);
    };

    const FTagDataDependencyNames& TracksLoader::getDependencies() const {
        return m_deps;
    }
    const std::set<std::string>& TracksLoader::getUsedRemap() const {
        return m_used_remap;
    }
    const std::string& TracksLoader::getName() const {
        return m_name;
    }
    const ConstituentsType& TracksLoader::getType() const {
        return m_config.type;
    }
}
