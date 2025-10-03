/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general Muons from the vertex 
  and extract their features for the NN evaluation.
*/

#ifndef INDET_MUONS_LOADER_H
#define INDET_MUONS_LOADER_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "xAODMuon/Muon.h"
#include "xAODMuon/MuonContainer.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace InDetGNNHardScatterSelection {

    // Subclass for Muons loader inherited from abstract IConstituentsLoader class
    class MuonsLoader : public IConstituentsLoader {
      public:
        MuonsLoader(const ConstituentsInputConfig & cfg);
        std::tuple<std::string, FlavorTagInference::Inputs, std::vector<const xAOD::IParticle*>> getData(
          const xAOD::Vertex& vertex) const override ;
        std::string getName() const override;
        ConstituentsType getType() const override;
      protected:
        // typedefs
        typedef xAOD::Vertex Vertex;
        typedef std::pair<std::string, double> NamedVar;
        typedef std::pair<std::string, std::vector<double> > NamedSeq;
        // iparticle typedefs
        typedef std::vector<const xAOD::Muon*> Muons;
        typedef std::vector<const xAOD::IParticle*> Particles;
        typedef std::function<double(const xAOD::Muon*,
                                    const Vertex&)> MuonSortVar;

        // usings for Muon getter
        using AE = SG::AuxElement;
        using IPC = xAOD::MuonContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::Muon*>;

        static MuonSortVar iparticleSortVar(ConstituentsSortOrder);

        std::vector<const xAOD::Muon*> getMuonsFromVertex(const xAOD::Vertex& vertex) const;

        MuonSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter<xAOD::Muon> m_customSequenceGetter;
        std::function<IPV(const Vertex&)> m_associator;
    };
}

#endif
