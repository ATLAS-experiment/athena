/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FTAG_TOOL_DEFAULTS_H
#define FTAG_TOOL_DEFAULTS_H

#include <string>

namespace ftag::defaults {
  const std::string tagger = "GN2v01";
  const std::string jet_collection = "AntiKt4EMPFlowJets";
  const std::string cdi_path = "xAODBTaggingEfficiency/13p6TeV/MC23_2024-10-17_GN2v01_v1.root";
  const std::string strategy = "SFEigen";
  const std::string pcbt_op = "Continuous";
}

#endif

