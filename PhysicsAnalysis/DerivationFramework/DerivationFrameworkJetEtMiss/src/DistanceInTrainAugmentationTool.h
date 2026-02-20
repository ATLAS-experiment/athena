/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

////////////////////////////////////////////////////////////////////////////////////////
// Author: Chris Young (christopher.young@cern.ch)
///////////////////////////////////////////////////////////////////
#ifndef DERIVATIONFRAMEWORK_BCDISTANCEAUGMENTATIONTOOL_H
#define DERIVATIONFRAMEWORK_BCDISTANCEAUGMENTATIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkInterfaces/IAugmentationTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "StoreGate/ReadDecorHandleKey.h"

#include "xAODEventInfo/EventInfo.h"
#include "LumiBlockData/BunchCrossingCondData.h"

namespace DerivationFramework {

  class DistanceInTrainAugmentationTool : public extends<AthAlgTool, IAugmentationTool> {

  public:

    using base_class::base_class;

    virtual StatusCode  initialize() override final;

    virtual StatusCode addBranches(const EventContext& ctx) const override final;

  private:
    // Tool to get distance into bunch train
    SG::ReadCondHandleKey<BunchCrossingCondData> m_bunchCrossingKey{this, "BunchCrossingKey", "BunchCrossingData", "Key BunchCrossing CDO" };

    SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo_key{this, "EventInfo", "EventInfo", "Input event information"};

    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDDistanceFrontKey {this, "BCIDDistanceFrontKey", m_eventInfo_key, "DFCommonJets_BCIDDistanceFromFront", "Decoration for BCID distance from front"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDDistanceTailKey {this, "BCIDDistanceTailKey", m_eventInfo_key, "DFCommonJets_BCIDDistanceTail" , "Decoration for BCID distance from tail"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDGapBeforeTrainKey {this, "BCIDGapBeforeTrainKey", m_eventInfo_key, "DFCommonJets_BCIDGapBeforeTrain", "Decoration for BCID gap before train"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDGapAfterTrainKey {this, "BCIDGapAfterTrainKey", m_eventInfo_key, "DFCommonJets_BCIDGapAfterTrain", "Decoration for BCID gap after train"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDTypeKey {this, "BCIDTypeKey", m_eventInfo_key, "DFCommonJets_BCIDType", "Decoration for BCID type"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDGapBeforeTrainMinus12Key {this, "BCIDGapBeforeTrainMinus12Key", m_eventInfo_key, "DFCommonJets_BCIDGapBeforeTrainMinus12", "Decoration for BCID gap before train minus 12 BCIDs"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDGapAfterTrainMinus12Key {this, "BCIDGapAfterTrainMinus12Key", m_eventInfo_key, "DFCommonJets_BCIDGapAfterTrainMinus12", "Decoration for BCID gap after train minus 12 BCIDs"};
    SG::WriteDecorHandleKey<xAOD::EventInfo> m_BCIDTypeMinus12Key {this, "BCIDTypeMinus12Key", m_eventInfo_key, "DFCommonJets_BCIDTypeMinus12", "Decoration for BCID type minus 12 BCIDs"};

  };

}

#endif
