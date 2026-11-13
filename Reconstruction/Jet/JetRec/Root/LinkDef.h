/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include <JetRec/FastJetInterfaceTool.h>
#include <JetRec/JetFilterTool.h>
#include <JetRec/JetFromPseudojet.h>
#include <JetRec/JetModifierBase.h>
#include <JetRec/JetPseudojetCopier.h>
#include <JetRec/JetPseudojetRetriever.h>
#include <JetRec/JetSorter.h>
#include <JetRec/MuonSegmentPseudoJetGetter.h>
#include <JetRec/PseudoJetGetter.h>
#include <JetRec/PseudoJetGetterRegistry.h>

#ifdef __CINT__

#pragma link off all globals;
#pragma link off all classes;
#pragma link off all functions;
#pragma link C++ nestedclass;

#pragma link C++ namespace FastJetInterface;
#pragma link C++ class FastJetInterfaceTool+;

#pragma link C++ class JetFilterTool;
#pragma link C++ class JetFromPseudojet+;
#pragma link C++ class JetModifierBase+;
#pragma link C++ class JetPseudojetCopier+;
#pragma link C++ class JetPseudojetRetriever+;
#pragma link C++ class JetSorter+;
#pragma link C++ class MuonSegmentPseudoJetGetter+;
#pragma link C++ class PseudoJetGetter+;
#pragma link C++ class PseudoJetGetterRegistry+;

#endif
