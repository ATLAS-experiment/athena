/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

// The CustomGetterUtils file is a catch-all for various getter functinos
// that need to be hard coded for whatever reason. Some of these are
// accessing methods like `pt` which have no name in the EDM, others
// can't be stored in the edm directly for various reasons.


// EDM includes
#include "xAODJet/JetFwd.h"
#include "xAODTracking/TrackParticleFwd.h"
#include "xAODBase/IParticle.h"
#include "AthContainers/AuxElement.h"
#include "FlavorTagDiscriminants/DataPrepUtilities.h"


#include <functional>
#include <string>
#include <set>

#ifndef CUSTOM_GETTER_UTILS_H
#define CUSTOM_GETTER_UTILS_H

namespace FlavorTagDiscriminants {

  /// Utils to produce IParticle -> vector<double> functions
  ///
  /// DL2 configures the its inputs when the algorithm is initalized,
  /// meaning that the list of track and jet properties that are used
  /// as inputs won't be known at compile time. Instead we build an
  /// array of "getter" functions, each of which returns one input for
  /// the tagger. The function here returns those getter functions.
  ///
  /// Many of the getter functions are trivial: they will, for example,
  /// read one double of auxdata off of the BTagging object. The
  /// sequence input getters tend to be more complicated. Since we'd
  /// like to avoid reimplementing the logic in these functions in
  /// multiple places, they are exposed here.
  ///
  /// NOTE: This file is for experts only, don't expect support.
  ///

  namespace getter_utils {

    using IParticles = std::vector<const xAOD::IParticle*>;

    using SequenceFromConstituents = std::function<std::vector<double>(
            const xAOD::Jet&,
            const IParticles&)>;
    using NamedSequenceFromConstituents = std::function<std::pair<std::string, std::vector<double>>(
      const xAOD::Jet&,
      const IParticles&)>;

    std::function<std::pair<std::string, double>(const xAOD::Jet&)>
    customGetterAndName(const std::string&);

    std::pair<SequenceFromConstituents, std::set<std::string>>
    customSequenceGetterWithDeps(const std::string& name,
                                const std::string& prefix);

    class CustomSequenceGetter {
        public:
          CustomSequenceGetter(std::vector<InputVariableConfig> inputs,
                              const FTagOptions& options);

          std::pair<std::vector<float>, std::vector<int64_t>> getFeats(const xAOD::Jet& jet, const IParticles& constituents) const;
          std::map<std::string, std::vector<double>> getDL2Feats(const xAOD::Jet& jet, const IParticles& constituents) const;

          std::set<std::string> getDependencies() const;
          std::set<std::string> getUsedRemap() const;
          
        private:
          std::pair<NamedSequenceFromConstituents, std::set<std::string>> seqFromConsituents(
            const InputVariableConfig& cfg, 
            const FTagOptions& options);

          std::vector<NamedSequenceFromConstituents> m_sequencesFromConstituents;
          std::set<std::string> m_deps;
          std::set<std::string> m_used_remap;        
        };
    }
}

#endif
