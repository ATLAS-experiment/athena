/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

  This is a subclass of IConstituentsLoader. It is used to load the general IParticles from the vertex
  and extract their features for the NN evaluation.
*/

#ifndef INDET_IPARTICLES_LOADER_H
#define INDET_IPARTICLES_LOADER_H

// local includes
#include "InDetGNNHardScatterSelection/ConstituentsLoader.h"
#include "InDetGNNHardScatterSelection/CustomGetterUtils.h"

// EDM includes
#include "xAODTracking/VertexFwd.h"

// STL includes
#include <string>
#include <vector>
#include <functional>
#include <exception>
#include <type_traits>
#include <regex>

namespace InDetGNNHardScatterSelection {

    // Subclass for IParticles loader inherited from abstract IConstituentsLoader class
    class IParticlesLoader : public IConstituentsLoader {
      public:
        IParticlesLoader(const ConstituentsInputConfig &);
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
        typedef std::vector<const xAOD::IParticle*> IParticles;
        typedef std::function<double(const xAOD::IParticle*,
                                    const Vertex&)> IParticleSortVar;

        // getter function
        typedef std::function<NamedSeq(const Vertex&, const IParticles&)> SeqFromIParticles;

        // usings for IParticle getter
        using AE = SG::AuxElement;
        using IPC = xAOD::IParticleContainer;
        using PartLinks = std::vector<ElementLink<IPC>>;
        using IPV = std::vector<const xAOD::IParticle*>;

        IParticleSortVar iparticleSortVar(ConstituentsSortOrder);

        std::vector<const xAOD::IParticle*> getIParticlesFromVertex(const xAOD::Vertex& vertex) const;

        IParticleSortVar m_iparticleSortVar;
        getter_utils::CustomSequenceGetter<xAOD::IParticle> m_customSequenceGetter;
        std::function<IPV(const Vertex&)> m_associator;
    };
}

#endif
