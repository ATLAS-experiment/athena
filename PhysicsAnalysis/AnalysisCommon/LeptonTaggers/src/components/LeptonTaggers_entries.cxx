/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "LeptonTaggers/PrimaryVertexReFitter.h"
#include "LeptonTaggers/VertexFittingTool.h"
#include "LeptonTaggers/VertexIterativeFitMergingTool.h"
#include "LeptonTaggers/NonPromptLeptonVertexingAlg.h"
#include "LeptonTaggers/RNNTool.h"
#include "LeptonTaggers/DecoratePromptLeptonRNN.h"
#include "LeptonTaggers/DecoratePromptLeptonImproved.h"
#include "LeptonTaggers/DecoratePLIT.h"

DECLARE_COMPONENT(Prompt::PrimaryVertexReFitter)
DECLARE_COMPONENT(Prompt::VertexFittingTool)
DECLARE_COMPONENT(Prompt::VertexIterativeFitMergingTool)
DECLARE_COMPONENT(Prompt::NonPromptLeptonVertexingAlg)
DECLARE_COMPONENT(Prompt::RNNTool)
DECLARE_COMPONENT(Prompt::DecoratePromptLeptonRNN)
DECLARE_COMPONENT(Prompt::DecoratePromptLeptonImproved)
DECLARE_COMPONENT(Prompt::DecoratePLIT)
