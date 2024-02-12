/*
  Copyright (C) 2002-2020 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DL2_H
#define DL2_H

// local includes
#include "FlavorTagDiscriminants/DataPrepUtilities.h"
#include "FlavorTagDiscriminants/TracksLoader.h"

// forward declarations
namespace lwt {
  class NanReplacer;
  class LightweightGraph;
}

namespace FlavorTagDiscriminants {

  class DL2 {
  public:
    DL2(const lwt::GraphConfig&,
        const std::vector<FTagInputConfig>&,
        const std::vector<FTagConstituentsSequenceConfig>& = {},
        const FTagOptions& = FTagOptions());
    void decorate(const xAOD::BTagging& btag) const;
    void decorate(const xAOD::Jet& jet) const;
    void decorateWithDefaults(const SG::AuxElement&) const;
    void decorate(const xAOD::Jet& jet, const SG::AuxElement& decorated) const;

    // functions to report data depdedencies
    FTagDataDependencyNames getDataDependencyNames() const;

  private:
    SG::AuxElement::ConstAccessor<ElementLink<xAOD::JetContainer>> m_jetLink;
    std::string m_input_node_name;
    std::unique_ptr<lwt::LightweightGraph> m_graph;
    std::unique_ptr<lwt::NanReplacer> m_variable_cleaner;
    std::vector<internal::VarFromBTag> m_varsFromBTag;
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
