/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the tracks from the jet 
  and extract their features for the NN evaluation.
*/

#ifndef TRACKS_LOADER_H
#define TRACKS_LOADER_H

// local includes
#include "FlavorTagInference/FlipTagEnums.h"

#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/DataPrepUtilities.h"
#include "FlavorTagInference/BTagTrackIpAccessor.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODJet/Jet.h"

// external libraries
#include "lwtnn/lightweight_network_config.hh"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace FlavorTagInference {
    using Tracks = std::vector<const xAOD::TrackParticle*>;

    // tracksConfig 
    ConstituentsInputConfig createTracksLoaderConfig(
      std::pair<std::string, std::vector<std::string>> trk_names,
      FlipTagConfig flip_config
    );

    // Subclass for Tracks loader inherited from abstract IConstituentsLoader class
    class TracksLoader : public IConstituentsLoader {
      public:

        TracksLoader(const ConstituentsInputConfig&, const FTagOptions& options);
        std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::IParticle& jet ) const override;
        std::tuple<char, std::map<std::string, std::vector<double>>>  getDL2Data(
          const xAOD::IParticle& jet, 
          std::function<char(const Tracks&)> ip_checker) const;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      private:
        // typedefs
        typedef xAOD::IParticle Jet;
        typedef xAOD::TrackParticle Track;
        // tracks typedefs
        typedef std::function<double(const Track*,
                                    const Jet&)> TrackSortVar;
        typedef std::function<bool(const Track*)> TrackFilter;
        typedef std::function<Tracks(const Tracks&,
                                    const Jet&)> TrackSequenceFilter;

        // usings for track getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using TPC = xAOD::TrackParticleContainer;
        using TrackLinks = std::vector<ElementLink<TPC>>;
        using PartLinks = std::vector<ElementLink<IPC>>;

        TrackSortVar trackSortVar(ConstituentsSortOrder, const FTagOptions&);
        std::pair<TrackFilter,std::set<std::string>> trackFilter(
          ConstituentsSelection, const FTagOptions&);
        std::pair<TrackSequenceFilter,std::set<std::string>> trackFlipper(
          const FTagOptions&);
        
        Tracks getTracksFromJet(const Jet& jet) const;

        TrackSortVar m_trackSortVar;
        TrackFilter m_trackFilter;
        TrackSequenceFilter m_trackFlipper;
        std::function<Tracks(const SG::AuxElement&)> m_associator;
        getter_utils::SeqGetter<xAOD::TrackParticle> m_seqGetter;
    };
}

#endif
