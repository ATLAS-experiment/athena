/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration

  This is a subclass of ConstituentsLoader. It is used to load the general IParticles from the jet 
  and extract their features for the NN evaluation. For now it supports only neutral flow objects.
  Charged flow objects have experimental support and are not recommended for use.
*/

#ifndef IPARTICLES_LOADER_H
#define IPARTICLES_LOADER_H

// local includes
#include "FlavorTagDiscriminants/ConstituentsLoader.h"
#include "FlavorTagDiscriminants/CustomGetterUtils.h"

// EDM includes
#include "xAODJet/JetFwd.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace FlavorTagDiscriminants {

    ConstituentsInputConfig createIParticlesLoaderConfig(
      std::pair<std::string, std::vector<std::string>> iparticle_names
    );
    // Subclass for IParticles loader inherited from abstract ConstituentsLoader class
    class IParticlesLoader : public ConstituentsLoader {
      public:
        IParticlesLoader(ConstituentsInputConfig, const FTagOptions& options);
        std::tuple<std::string, input_pair, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::Jet& jet, 
          [[maybe_unused]] const SG::AuxElement& btag) const override ;
        FTagDataDependencyNames getDependencies() const override;
        std::set<std::string> getUsedRemap() const override;
        std::string getName() const override;
        ConstituentsType getType() const override;
      protected:
        // typedefs
        typedef xAOD::Jet Jet;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // iparticle typedefs
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

        IParticleSortVar iparticleSortVar(ConstituentsSortOrder);
        
        std::vector<const xAOD::IParticle*> getIParticlesFromJet(const xAOD::Jet& jet) const;

        IParticleSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter m_customSequenceGetter;        
        std::function<IPV(const Jet&)> m_associator;
        bool m_isCharged;
    };
}

#endif