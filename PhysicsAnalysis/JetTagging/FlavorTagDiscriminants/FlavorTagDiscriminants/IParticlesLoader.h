/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef SUBJETS_LOADER_H
#define SUBJETS_LOADER_H

// local includes
#include "FlavorTagDiscriminants/FlipTagEnums.h"
#include "FlavorTagDiscriminants/AssociationEnums.h"
#include "FlavorTagDiscriminants/FTagDataDependencyNames.h"

#include "FlavorTagDiscriminants/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/BTagTrackIpAccessor.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBase/IParticle.h"
#include "xAODBTagging/BTagging.h"
#include "xAODCaloEvent/CaloCluster.h"
#include "xAODPFlow/FlowElement.h"

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
    class IParticlesLoader : public ConstituentsLoader {
      public:
        // TracksLoader();
        IParticlesLoader(FTagConstituentsSequenceConfig, const FTagOptions& options);
        std::pair<std::string, input_pair> getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const override ;
      private:
        // typedefs
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        typedef xAOD::Jet Jet;
        // tracks typedefs
        typedef std::vector<const xAOD::IParticle*> IParticles;
        typedef std::function<double(const xAOD::IParticle*,
                                    const Jet&)> IParticleSortVar;

        // getter function
        typedef std::function<NamedSeq(const Jet&, const IParticles&)> SeqFromIParticles;

        // usings for track getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::IParticle*>;

        IParticleSortVar iparticleSortVar(ConstituentsSortOrder, const FTagOptions&);
        std::pair<SeqFromIParticles,std::set<std::string>> seqFromIParticles(
          const FTagConstituentsInputConfig&, const FTagOptions&);
        
        IParticles getIParticlesFromJet(const Jet& jet, const AE& btag) const;

        IParticleSortVar m_iparticleSortVar;
        std::vector<SeqFromIParticles> m_sequencesFromIParticles;
        std::function<IPV(const Jet&)> m_associator;
    };
}

#endif