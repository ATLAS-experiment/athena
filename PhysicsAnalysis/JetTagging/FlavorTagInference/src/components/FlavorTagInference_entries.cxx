/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/JetTagDecoratorAlg.h"
#include "FlavorTagInference/JetTagConditionalDecoratorAlg.h"
#include "FlavorTagInference/GNNTool.h"
#include "FlavorTagInference/NNSharingOnnxSvc.h"
#include "FlavorTagInference/MultifoldGNNTool.h"
#include "FlavorTagInference/GNNDataLoader.h"

#ifndef XAOD_ANALYSIS
#include "FlavorTagInference/NNSharingTritonSvc.h"
#endif

#include "src/FoldDecoratorAlg.h"

using namespace FlavorTagInference;

DECLARE_COMPONENT(JetTagDecoratorAlg)
DECLARE_COMPONENT(JetTagConditionalDecoratorAlg)  
DECLARE_COMPONENT(GNNTool)
DECLARE_COMPONENT(NNSharingOnnxSvc)
DECLARE_COMPONENT(MultifoldGNNTool)
DECLARE_COMPONENT(FoldDecoratorAlg)

#ifndef XAOD_ANALYSIS
DECLARE_COMPONENT(NNSharingTritonSvc)
#endif
