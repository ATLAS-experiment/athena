/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DL2_H
#define DL2_H

// local includes
#include "FlavorTagInference/DataPrepUtilities.h"
#include "FlavorTagInference/TracksLoader.h"

// forward declarations
namespace lwt {
  class NanReplacer;
  class LightweightGraph;
}

namespace FlavorTagDiscriminants {

  using namespace FlavorTagInference;

  class DL2 {
  public:
    DL2(const lwt::GraphConfig&,
        const std::vector<FTagInputConfig>&,
        const std::vector<ConstituentsInputConfig>& = {},
        const FTagOptions& = FTagOptions());
    void decorate(const xAOD::IParticle& i_jet) const;
    void decorateWithDefaults(const SG::AuxElement&) const;


    // functions to report data dependencies
    const FTagDataDependencyNames& getDataDependencyNames() const;

  private:
    void decorate(const xAOD::Jet& jet) const;
    std::string m_input_node_name;
    std::unique_ptr<lwt::LightweightGraph> m_graph;
    std::unique_ptr<lwt::NanReplacer> m_variable_cleaner;
    std::vector<internal::VarFromJet> m_varsFromJet;
    std::vector<std::shared_ptr<TracksLoader>> m_tracksLoaders;
    std::map<std::string, internal::OutNodeFloat> m_decorators;
    float m_defaultValue;
    std::function<char(const internal::Tracks&)> m_invalid_track_checker;
    std::vector<SG::AuxElement::Decorator<char>> m_is_defaults;

    FTagDataDependencyNames m_dataDependencyNames;

  };
}
#endif
