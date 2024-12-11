/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general Electrons from the vertex
  and extract their features for the NN evaluation.
*/

#ifndef INDET_ELECTRONS_LOADER_H
#define INDET_ELECTRONS_LOADER_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "xAODEgamma/Electron.h"
#include "xAODEgamma/ElectronContainer.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace InDetGNNHardScatterSelection {

    // Subclass for Electrons loader inherited from abstract IConstituentsLoader class
    class ElectronsLoader : public IConstituentsLoader {
      public:
        ElectronsLoader(const ConstituentsInputConfig &);
        std::tuple<std::string, FlavorTagDiscriminants::Inputs, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::Vertex& vertex) const override ;
        std::string getName() const override;
        ConstituentsType getType() const override;
      protected:
        // typedefs
        typedef xAOD::Vertex Vertex;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // iparticle typedefs
        typedef std::vector<const xAOD::Electron*> Electrons;
        typedef std::vector<const xAOD::IParticle*> Particles;
        typedef std::function<double(const xAOD::Electron*,
                                    const Vertex&)> ElectronSortVar;

        // getter function
        typedef std::function<NamedSeq(const Vertex&, const Electrons&)> SeqFromElectrons;

        // usings for Electron getter
        using AE = SG::AuxElement;
        using IPC = xAOD::ElectronContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::Electron*>;

        ElectronSortVar iparticleSortVar(ConstituentsSortOrder);

        std::vector<const xAOD::Electron*> getElectronsFromVertex(const xAOD::Vertex& vertex) const;

        ElectronSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter<xAOD::Electron> m_customSequenceGetter;
        std::function<IPV(const Vertex&)> m_associator;
    };
}

#endif
