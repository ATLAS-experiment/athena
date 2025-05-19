/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef DL2_HIGH_LEVEL_HH
#define DL2_HIGH_LEVEL_HH

#include "FlavorTagInference/FlipTagEnums.h"
#include "FlavorTagInference/AssociationEnums.h"
#include "FlavorTagInference/FTagDataDependencyNames.h"
#include "FlavorTagInference/SaltModelGraphConfig.h"

// EDM includes
#include "xAODBTagging/BTaggingFwd.h"
#include "xAODJet/JetFwd.h"
#include "AthContainers/AuxElement.h"

#include <memory>
#include <string>
#include <map>
#include <cmath>

namespace FlavorTagDiscriminants {

  using FlavorTagInference::FlipTagConfig;
  using FlavorTagInference::TrackLinkType;
  using FlavorTagInference::FTagDataDependencyNames;

  class DL2;

  class DL2HighLevel
  {
  public:
    DL2HighLevel(const std::string& nn_file_name,
                 FlipTagConfig = FlipTagConfig::STANDARD,
                 std::map<std::string, std::string> remap_scalar = {},
                 TrackLinkType = TrackLinkType::TRACK_PARTICLE,
                 float default_output_value = NAN);
    DL2HighLevel(DL2HighLevel&&);
    DL2HighLevel(const DL2HighLevel&);
    ~DL2HighLevel();
    void decorate(const xAOD::BTagging& btag) const;
    void decorate(const xAOD::Jet& jet) const;
    void decorateWithDefaults(const SG::AuxElement& jet) const;
    FTagDataDependencyNames getDataDependencyNames() const;
  private:
    std::shared_ptr<const DL2> m_dl2;
  };

}

#endif
