/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general TrackParticles from the vertex 
  and extract their features for the NN evaluation.
*/

#ifndef INDET_TRACKS_LOADER_H
#define INDET_TRACKS_LOADER_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/TrackParticle.h"
#include "xAODTracking/TrackParticleContainer.h"
#include "xAODBase/IParticle.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace InDetGNNHardScatterSelection {

    // Subclass for TrackParticles loader inherited from abstract IConstituentsLoader class
    class TracksLoader : public IConstituentsLoader {
      public:
        TracksLoader(const ConstituentsInputConfig & cfg);
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
        typedef std::vector<const xAOD::TrackParticle*> TrackParticles;
        typedef std::vector<const xAOD::IParticle*> Particles;
        typedef std::function<double(const xAOD::TrackParticle*,
                                    const Vertex&)> TrackParticleSortVar;

        // usings for TrackParticle getter
        using AE = SG::AuxElement;
        using IPC = xAOD::TrackParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::TrackParticle*>;

        TrackParticleSortVar iparticleSortVar(ConstituentsSortOrder);

        std::vector<const xAOD::TrackParticle*> getTrackParticlesFromVertex(const xAOD::Vertex& vertex) const;

        TrackParticleSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter<xAOD::TrackParticle> m_customSequenceGetter;
        std::function<IPV(const Vertex&)> m_associator;
    };
}

#endif
