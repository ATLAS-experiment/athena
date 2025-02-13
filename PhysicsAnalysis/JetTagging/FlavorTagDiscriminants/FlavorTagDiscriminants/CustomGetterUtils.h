/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/// This file contains "getter" functions used for accessing tagger inputs
/// from the EDM. In the most basic case, inputs can be directly retrieved
/// from the EDM using accessors. In other cases, inputs require custom code
/// called "custom getters" to produce the desired values.
///
/// - a basic "getter" directly retrieves decorated values from an object
/// - a "custom getter" executes custom code to produce inputs (e.g. particle.pt()).
///   Custom getters are used for inputs that cannot be directly retrieved from the 
///   EDM using accessors or which require on-the-fly calculations, such as IP variables.
/// - a "sequence getter" is a wrapper around a getter that broadcasts it over a 
///   vector of associated objects (for example over all the tracks in a jet).
///
/// Inputs to tagging algorithms are configured when the algorithm is initialised
/// which means that the list of track and jet features that are used as inputs is 
/// not known at compile time. Instead we build an array of "getter" functions, 
/// each of which returns one feature for the tagger. 
///
/// NOTE: This file is for experts only, don't expect support.
///


// EDM includes
#include "xAODJet/JetFwd.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"

#include <functional>
#include <string>
#include <set>

#ifndef CUSTOM_GETTER_UTILS_H
#define CUSTOM_GETTER_UTILS_H

namespace FlavorTagDiscriminants {

  namespace getter_utils {
    
    // -------------------------------------------------------
    // type aliases
    template <typename T>
    using Constituents = std::vector<const T*>;

    template <typename T>
    using SequenceGetterFunc = std::function<std::vector<double>(const xAOD::Jet&, const Constituents<T>&)>;
    // -------------------------------------------------------

    std::function<std::pair<std::string, double>(const xAOD::Jet&)>
    namedCustomJetGetter(const std::string&);

    template <typename T>
    std::pair<SequenceGetterFunc<T>, std::set<std::string>>
    buildCustomSeqGetter(const std::string& name, const std::string& prefix);

    /**
     * @brief Template class to extract features from sequence of constituents
     * 
     * @tparam T constituent type
     * 
     * It supports the following types of constituents:
     * - xAOD::FlowElement
     * - xAOD::TrackParticle
     * - xAOD::TrackMeasurementValidation
     * - xAOD::Electron
    */
    template <typename T>
    class SeqGetter {
        public:
          using Const = Constituents<T>;
          using InputSequence = std::function<std::pair<std::string, std::vector<double>>(
              const xAOD::Jet&,
              const Const&)>;

          SeqGetter(const std::vector<InputVariableConfig>& inputs, const FTagOptions& options);

          std::pair<std::vector<float>, std::vector<int64_t>> getFeats(const xAOD::Jet& jet, const Const& constituents) const;
          std::map<std::string, std::vector<double>> getDL2Feats(const xAOD::Jet& jet, const Const& constituents) const;

          const std::set<std::string>& getDependencies() const;
          const std::set<std::string>& getUsedRemap() const;

        private:
          std::pair<InputSequence, std::set<std::string>> getNamedCustomSeqGetter(
            const std::string& name,
            const std::string& prefix);
          std::pair<InputSequence, std::set<std::string>> seqFromConsituents(
            const InputVariableConfig& cfg, 
            const FTagOptions& options);
          std::vector<InputSequence> m_sequence_getters;
          std::set<std::string> m_deps;
          std::set<std::string> m_used_remap;        
        };
    }
}

#endif
