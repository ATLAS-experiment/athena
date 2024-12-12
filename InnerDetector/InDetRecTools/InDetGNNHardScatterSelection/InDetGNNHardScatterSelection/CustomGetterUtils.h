/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// The CustomGetterUtils file is a catch-all for various getter functinos
// that need to be hard coded for whatever reason. Some of these are
// accessing methods like `pt` which have no name in the EDM, others
// can't be stored in the edm directly for various reasons.

#ifndef HSGNN_CUSTOM_GETTER_UTILS_H
#define HSGNN_CUSTOM_GETTER_UTILS_H

// EDM includes
#include "xAODTracking/VertexFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/AuxElement.h"
#include "InDetGNNHardScatterSelection/DataPrepUtilities.h"


#include <functional>
#include <string>
#include <set>
#include <vector>


namespace InDetGNNHardScatterSelection {

  /// Utils to produce Constituent -> vector<double> functions
  ///
  /// The GNN configures its inputs when the algorithm is initalized,
  /// meaning that the list of vertex and constituent properties that are used
  /// as inputs won't be known at compile time. Instead we build an
  /// array of "getter" functions, each of which returns one input for
  /// the tagger. The function here returns those getter functions.
  ///
  /// Many of the getter functions are trivial: they will, for example,
  /// read one double of auxdata off of the Vertex object. The
  /// sequence input getters tend to be more complicated. Since we'd
  /// like to avoid reimplementing the logic in these functions in
  /// multiple places, they are exposed here.
  ///
  /// NOTE: This file is for experts only, don't expect support.
  ///

  namespace getter_utils {

    using IParticles = std::vector<const xAOD::IParticle*>;
    using Tracks = std::vector<const xAOD::TrackParticle*>;

    using SequenceFromIParticles = std::function<std::vector<double>(
            const xAOD::Vertex&,
            const IParticles&)>;
    using SequenceFromTracks = std::function<std::vector<double>(
            const xAOD::Vertex&,
            const Tracks&)>;
    

    std::function<std::pair<std::string, double>(const xAOD::Vertex&)>
    customGetterAndName(const std::string&);

    template <typename T>
    std::pair<
        std::function<std::vector<double>(
          const xAOD::Vertex&,
          const std::vector<const T*>&)>,
        std::set<std::string>>
    customSequenceGetterWithDeps(const std::string& name,
                                const std::string& prefix);

    /**
     * @brief Template class to extract features from sequence of constituents
     * 
     * @tparam T constituent type
     * 
     * It supports the following types of constituents:
     * - xAOD::IParticle
     * - xAOD::TrackParticle
    */
    template <typename T>
    class CustomSequenceGetter {
        public:
          using Constituents = std::vector<const T*>;
          using NamedSequenceFromConstituents = std::function<std::pair<std::string, std::vector<double>>(
              const xAOD::Vertex&,
              const Constituents&)>;
          CustomSequenceGetter(const std::vector<InputVariableConfig> & inputs);

          std::pair<std::vector<float>, std::vector<int64_t>> getFeats(const xAOD::Vertex& jet, const Constituents& constituents) const;
          
        private:
          std::pair<NamedSequenceFromConstituents, std::set<std::string>> customNamedSeqGetterWithDeps(
            const std::string& name);
          std::pair<NamedSequenceFromConstituents, std::set<std::string>> seqFromConsituents(
            const InputVariableConfig& cfg); 
          std::vector<NamedSequenceFromConstituents> m_sequencesFromConstituents;
        };
    }
}

#endif
