/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRACKS_LOADER_H
#define TRACKS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"

#include "FlavorTagDiscriminants/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBTagging/BTagging.h"

// external libraries
#include "lwtnn/lightweight_network_config.hh"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace FlavorTagDiscriminants {

    // tracksConfig getter

    FTagConstituentsSequenceConfig convertTracksConfig(
      FTagTrackSequenceConfig config
    );
    std::vector<FTagTrackSequenceConfig> convertTracksConfigBack(
      FTagConstituentsSequenceConfig config
    );
    FTagConstituentsSequenceConfig createTracksLoaderConfig(
      std::pair<std::string, std::vector<std::string>> trk_names,
      FlipTagConfig flip_config
    );


    // Subclass for Tracks loader inherited from abstract ConstituentsLoader class
    class TracksLoader : public ConstituentsLoader {
      public:
        // TracksLoader();
        TracksLoader(FTagConstituentsSequenceConfig, const FTagOptions& options);
        std::pair<std::string, input_pair> getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const override ;
      private:
        // typedefs
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        typedef xAOD::Jet Jet;
        typedef xAOD::TrackParticle Track;
        // tracks typedefs
        typedef std::vector<const Track*> Tracks;
        typedef std::function<double(const Track*,
                                    const Jet&)> TrackSortVar;
        typedef std::function<bool(const Track*)> TrackFilter;
        typedef std::function<Tracks(const Tracks&,
                                    const Jet&)> TrackSequenceFilter;

        // getter function
        typedef std::function<NamedSeq(const Jet&, const Tracks&)> SeqFromTracks;

        // usings for track getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using TPC = xAOD::TrackParticleContainer;
        using TrackLinks = std::vector<ElementLink<TPC>>;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using TPV = std::vector<const xAOD::TrackParticle*>;

        TrackSortVar trackSortVar(ConstituentsSortOrder, const FTagOptions&);
        std::pair<TrackFilter,std::set<std::string>> trackFilter(
          ConstituentsSelection, const FTagOptions&);
        std::pair<SeqFromTracks,std::set<std::string>> seqFromTracks(
          const FTagConstituentsInputConfig&, const FTagOptions&);
        std::pair<TrackSequenceFilter,std::set<std::string>> flipFilter(
          const FTagOptions&);
        
        Tracks getTracksFromJet(const Jet& jet, const AE& btag) const;

        TrackSortVar m_trackSortVar;
        TrackFilter m_trackFilter;
        TrackSequenceFilter m_flipFilter;
        std::vector<SeqFromTracks> m_sequencesFromTracks;
        std::function<TPV(const SG::AuxElement&)> m_associator;
    };
}

#endif