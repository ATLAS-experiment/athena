/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPARTICLES_LOADER_H
#define IPARTICLES_LOADER_H

// local includes
#include "FlavorTagDiscriminants/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"

// EDM includes
#include "xAODJet/Jet.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace FlavorTagDiscriminants {
    // Subclass for Tracks loader inherited from abstract ConstituentsLoader class
    class IParticlesLoader : public ConstituentsLoader {
      public:
        // TracksLoader();
        IParticlesLoader(FTagConstituentsSequenceConfig, const FTagOptions& options);
        std::pair<std::string, input_pair> getData(const xAOD::Jet& jet, const SG::AuxElement& btag) const override ;
      protected:
        // typedefs
        typedef xAOD::Jet Jet;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // tracks typedefs
        typedef std::vector<const xAOD::IParticle*> IParticles;
        typedef std::function<double(const xAOD::IParticle*,
                                    const Jet&)> IParticleSortVar;

        // getter function
        typedef std::function<NamedSeq(const Jet&, const IParticles&)> SeqFromIParticles;

        // usings for IParticle getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::IParticle*>;

        IParticleSortVar iparticleSortVar(ConstituentsSortOrder, const FTagOptions&);
        
        std::vector<const xAOD::IParticle*> getIParticlesFromJet(const xAOD::Jet& jet) const;
        std::pair<SeqFromIParticles,std::set<std::string>> seqFromIParticles(
          const FTagConstituentsInputConfig&, const FTagOptions&);

        std::vector<SeqFromIParticles> m_sequencesFromIParticles;
        IParticleSortVar m_iparticleSortVar;
        std::function<IPV(const Jet&)> m_associator;
        bool m_isCharged;
    };
}

#endif