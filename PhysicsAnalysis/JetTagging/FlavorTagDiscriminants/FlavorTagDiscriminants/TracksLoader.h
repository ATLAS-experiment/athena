/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the tracks from the jet 
  and extract their features for the NN evaluation.
*/

#ifndef TRACKS_LOADER_H
#define TRACKS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"

#include "FlavorTagDiscriminants/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"
#include "FlavorTagDiscriminants/CustomGetterUtils.h"

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

    // tracksConfig 
    ConstituentsInputConfig createTracksLoaderConfig(
      std::pair<std::string, std::vector<std::string>> trk_names,
      FlipTagConfig flip_config
    );


    // Subclass for Tracks loader inherited from abstract IConstituentsLoader class
    class TracksLoader : public IConstituentsLoader {
      public:
        typedef std::vector<const xAOD::TrackParticle*> Tracks;

        TracksLoader(ConstituentsInputConfig, const FTagOptions& options);
        std::tuple<std::string, input_pair, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::Jet& jet, 
          [[maybe_unused]] const SG::AuxElement& btag) const override;
        std::tuple<char, std::map<std::string, std::vector<double>>>  getDL2Data(
          const xAOD::Jet& jet, 
          const SG::AuxElement& btag, 
          std::function<char(const Tracks&)> ip_checker) const;
        FTagDataDependencyNames getDependencies() const override;
        std::set<std::string> getUsedRemap() const override;
        std::string getName() const override;
        ConstituentsType getType() const override;
      private:
        // typedefs
        typedef xAOD::Jet Jet;
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
        using TPV = std::vector<const xAOD::TrackParticle*>;

        TrackSortVar trackSortVar(ConstituentsSortOrder, const FTagOptions&);
        std::pair<TrackFilter,std::set<std::string>> trackFilter(
          ConstituentsSelection, const FTagOptions&);
        std::pair<TrackSequenceFilter,std::set<std::string>> flipFilter(
          const FTagOptions&);
        
        Tracks getTracksFromJet(const Jet& jet, const AE& btag) const;

        TrackSortVar m_trackSortVar;
        TrackFilter m_trackFilter;
        TrackSequenceFilter m_flipFilter;
        std::function<TPV(const SG::AuxElement&)> m_associator;
        getter_utils::CustomSequenceGetter<xAOD::TrackParticle> m_customSequenceGetter;
    };
}

#endif