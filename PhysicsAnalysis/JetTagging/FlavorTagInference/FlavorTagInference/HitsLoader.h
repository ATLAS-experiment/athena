/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general IParticles from the jet 
  and extract their features for the NN evaluation. For now it supports only neutral flow objects.
  Charged flow objects have experimental support and are not recommended for use.
*/

#ifndef HITS_LOADER_H
#define HITS_LOADER_H

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODJet/JetFwd.h"
#include "xAODBase/IParticle.h"
#include "xAODTracking/TrackMeasurementValidationContainer.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace FlavorTagInference {

  using namespace FlavorTagInference;

    ConstituentsInputConfig createHitsLoaderConfig(
      std::pair<std::string, std::vector<std::string>> hits_names
    );
    // Subclass for Hits loader inherited from abstract IConstituentsLoader class
    class HitsLoader : public IConstituentsLoader {
      public:
        HitsLoader(const ConstituentsInputConfig&, const FTagOptions& options);
        std::tuple<Inputs, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::IParticle& jet ) const override ;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      protected:
        // typedefs
        typedef xAOD::IParticle Jet;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // hit typedefs
        typedef std::vector<const xAOD::TrackMeasurementValidation*> Hits;

        // getter function
        typedef std::function<NamedSeq(const Jet&, const Hits&)> SeqFromHits;

        // usings for Hits getter
        using AE = SG::AuxElement;
        using TMC = xAOD::TrackMeasurementValidationContainer;
        using HitLinks = std::vector<ElementLink<TMC>>;
        using TMVV = std::vector<const xAOD::TrackMeasurementValidation*>;

        
        std::vector<const xAOD::TrackMeasurementValidation*> getHitsFromJet(const xAOD::IParticle& jet) const;

        getter_utils::SeqGetter<xAOD::TrackMeasurementValidation> m_seqGetter;        
        std::function<TMVV(const Jet&)> m_associator;
    };
}

#endif
