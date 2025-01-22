/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/// @author Diego Baron

#ifndef B_TAGGING_SCORES_ALG_H
#define B_TAGGING_SCORES_ALG_H

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
    // configurable properties
    Gaudi::Property<std::string> m_taggerName {this, "taggerName", "", "name of the b-tagging algorithm to extract info from"};

    // inputs needed for retrieving b-tagging scores
    SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey{
      this, "jets", "", "the jet container to use"};

    Gaudi::Property< std::vector<std::string> > m_vars{
      this, "vars", {}, "variables to retrieve from the b-tagging object and decorate onto the jets"};

    // pair input and output variables via accessors and decorators
    std::vector< std::pair<
		   SG::AuxElement::ConstAccessor<float>,
		   SG::AuxElement::Decorator<float>
		   > > m_accdecs;

  };

} // namespace

#endif
