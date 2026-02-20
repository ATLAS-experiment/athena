// Dear emacs, this is -*- c++ -*-

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#pragma once

// EDM include(s):
#include "xAODTau/DiTauJet.h"
#include "DiTauToolBase.h"

#include "AsgTools/PropertyWrapper.h"
#include "AsgDataHandles/WriteDecorHandleKeyArray.h"

class DiTauWPDecorator 
  : public DiTauToolBase
{
public:

  DiTauWPDecorator( const std::string& type, const std::string& name, const IInterface * parent);

  virtual ~DiTauWPDecorator();

  // initialize the tool
  virtual StatusCode initialize() override;

  virtual StatusCode executeObj(xAOD::DiTauJet& xDiTau, const EventContext& ctx ) const override;  

  bool passOmniWP(float score, float wp_thrshold ) const;

private:

  Gaudi::Property<std::string> m_scoreName{this, "ScoreName", "", "Name of the original score"};
  Gaudi::Property<std::vector<std::string>> m_decorWPs{this, "DecorWPNames", {}, "Name of WPs"};
  Gaudi::Property<std::vector<float>> m_decorWPCuts{this, "DecorWPCuts", {}, "Cut on each WP to be docorated for ditaus"}; 
  
  std::vector<SG::AuxElement::Accessor<char>> m_charDecors;
  
  Gaudi::Property<std::string> m_ditauContainerName{this, "DiTauContainerName", "", "Name of DiTauJetContainer, must be set when using "};

};


