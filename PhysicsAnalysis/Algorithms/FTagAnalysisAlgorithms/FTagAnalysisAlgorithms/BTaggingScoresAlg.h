/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/// @author Diego Baron

#ifndef F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_SCORES_ALG_H
#define F_TAG_ANALYSIS_ALGORITHMS__B_TAGGING_SCORES_ALG_H

// Algorithm includes
#include <AnaAlgorithm/AnaReentrantAlgorithm.h>
#include <AsgTools/PropertyWrapper.h>
#include <AsgDataHandles/ReadHandleKey.h>
#include <AsgDataHandles/ReadHandle.h>

// Framework includes
#include <xAODJet/JetContainer.h>

namespace CP {

  class BTaggingScoresAlg final : public EL::AnaReentrantAlgorithm
  {

  public:
    using EL::AnaReentrantAlgorithm::AnaReentrantAlgorithm;
    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext &ctx) const override;

  private:
    // inputs needed for retrieving b-tagging scores
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{
      this, "jets", "", "the jet container to use"};

    Gaudi::Property< std::vector<std::string> > m_vars{
      this, "vars", {}, "variables to retrieve from the b-tagging object and decorate onto the jets"};

    // pair input and output variables via accessors and decorators
    std::vector< std::pair<
                   SG::ConstAccessor<float>,
                   SG::Decorator<float>
                   > > m_accdecs;

  };

} // namespace

#endif
