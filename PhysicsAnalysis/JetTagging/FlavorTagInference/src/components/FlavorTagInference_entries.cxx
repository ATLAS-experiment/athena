/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "FlavorTagInference/BTagDecoratorAlg.h"
#include "FlavorTagInference/JetTagDecoratorAlg.h"
#include "FlavorTagInference/BTagConditionalDecoratorAlg.h"
#include "FlavorTagInference/JetTagConditionalDecoratorAlg.h"
#include "FlavorTagInference/GNNTool.h"
#include "FlavorTagInference/NNSharingSvc.h"
#include "FlavorTagInference/MultifoldGNNTool.h"
#include "FlavorTagInference/GNNDataLoader.h"


#include "src/FoldDecoratorAlg.h"

using namespace FlavorTagInference;

DECLARE_COMPONENT(BTagDecoratorAlg)
DECLARE_COMPONENT(JetTagDecoratorAlg)
DECLARE_COMPONENT(BTagConditionalDecoratorAlg)
DECLARE_COMPONENT(JetTagConditionalDecoratorAlg)  
DECLARE_COMPONENT(GNNTool)
DECLARE_COMPONENT(NNSharingSvc)
DECLARE_COMPONENT(MultifoldGNNTool)
DECLARE_COMPONENT(FoldDecoratorAlg)
