/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general Photons from the vertex 
  and extract their features for the NN evaluation.
*/

#ifndef INDET_PHOTONS_LOADER_H
#define INDET_PHOTONS_LOADER_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "xAODEgamma/Photon.h"
#include "xAODEgamma/PhotonContainer.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace InDetGNNHardScatterSelection {

    // Subclass for Photons loader inherited from abstract IConstituentsLoader class
    class PhotonsLoader : public IConstituentsLoader {
      public:
        PhotonsLoader(ConstituentsInputConfig);
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
        typedef std::vector<const xAOD::Photon*> Photons;
        typedef std::vector<const xAOD::IParticle*> Particles;
        typedef std::function<double(const xAOD::Photon*,
                                    const Vertex&)> PhotonSortVar;

        // usings for Photon getter
        using AE = SG::AuxElement;
        using IPC = xAOD::PhotonContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::Photon*>;

        PhotonSortVar iparticleSortVar(ConstituentsSortOrder);

        std::vector<const xAOD::Photon*> getPhotonsFromVertex(const xAOD::Vertex& vertex) const;

        PhotonSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter<xAOD::Photon> m_customSequenceGetter;
        std::function<IPV(const Vertex&)> m_associator;
    };
}

#endif
