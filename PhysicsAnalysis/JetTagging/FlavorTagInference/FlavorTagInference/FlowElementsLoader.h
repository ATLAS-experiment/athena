/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the FlowElements from the jet 
  and extract their features for the NN evaluation.
*/

#ifndef FLOW_ELEMENTS_LOADER_H
#define FLOW_ELEMENTS_LOADER_H

// local includes
#include "FlavorTagInference/ConstituentsLoader.h"
#include "FlavorTagInference/CustomGetterUtils.h"

// EDM includes
#include "xAODJet/JetFwd.h"
#include "xAODPFlow/FlowElement.h"

// STL includes
#include <string>
#include <vector>
#include <functional>

namespace FlavorTagInference {

    // Subclass for IParticles loader inherited from abstract IConstituentsLoader class
    class FlowElementsLoader : public IConstituentsLoader {
      public:
        FlowElementsLoader(const ConstituentsInputConfig& cfg, const FTagOptions& options);
        std::tuple<std::string, Inputs, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::Jet& jet ) const override ;
        const FTagDataDependencyNames& getDependencies() const override;
        const std::set<std::string>& getUsedRemap() const override;
        const std::string& getName() const override;
        const ConstituentsType& getType() const override;
      protected:
        // typedefs
        typedef xAOD::Jet Jet;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // FlowElement typedefs
        typedef std::vector<const xAOD::FlowElement*> FlowElements;
        typedef std::function<double(const xAOD::FlowElement*,
                                    const Jet&)> FlowElementSortVar;

        // getter function
        typedef std::function<NamedSeq(const Jet&, const FlowElements&)> SeqFromFlowElements;

        // usings for IParticle getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using FEV = std::vector<const xAOD::FlowElement*>;

        FlowElementSortVar flowElementSortVar(ConstituentsSortOrder);
        
        std::vector<const xAOD::FlowElement*> getFlowElementsFromJet(const xAOD::Jet& jet) const;

        FlowElementSortVar m_flowElementSortVar;
        getter_utils::SeqGetter<xAOD::FlowElement> m_seqGetter;        
        std::function<FEV(const Jet&)> m_associator;
    };
}

#endif
