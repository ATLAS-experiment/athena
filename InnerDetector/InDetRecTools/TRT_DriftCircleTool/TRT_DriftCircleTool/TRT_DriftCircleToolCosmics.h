/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  TRT_DriftCircleToolCosmics
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Interface for TRT RDI collection production
///////////////////////////////////////////////////////////////////
// Version 1.0 18/02/2003 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#ifndef TRT_DriftCircleToolCosmics_H
#define TRT_DriftCircleToolCosmics_H

#include "TRT_DriftCircleTool/ITRT_DriftCircleTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "TrkPrepRawData/PrepRawDataCLASS_DEF.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"
#include "CommissionEvent/ComTime.h"
#include "StoreGate/ReadCondHandleKey.h"

class IInDetConditionsSvc;
class ITRT_DriftFunctionTool;
class TRT_ID;
class TRT_RDORawData;
class TRT_DriftCircleCollection;
class TRT_DriftCircle;


namespace InDet {
/** @class TRT_DriftCircleToolCosmics
Class for converting a RDO collection to a DriftCircle collection
Performs trigger phase corrections for in-cavern cosmics
*/
class TRT_DriftCircleToolCosmics: public AthAlgTool, virtual public ITRT_DriftCircleTool
 
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:
  /** constructor */
  TRT_DriftCircleToolCosmics(const std::string&,const std::string&,const IInterface*);
  /** constructor */
  virtual ~TRT_DriftCircleToolCosmics ();
  /** initialize needed services */
  virtual StatusCode initialize();
  /** finalize */
  virtual StatusCode finalize  ();
  /** make the conversion from RDOs to DriftCircles */
  virtual InDet::TRT_DriftCircleCollection*  
  convert(int,const InDetRawDataCollection<TRT_RDORawData>*, const EventContext& ctx,
          DataPool<TRT_DriftCircle>* dataItemsPool,
          const bool  CTBBadChannels) const;
  virtual bool passValidityGate(unsigned int word, float lowGate, float highGate, float t0) const;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
  
 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
  SG::ReadHandleKey<ComTime> m_evtPhaseKey{this, "TRT_Phase", "TRT_Phase",
      "RHK to retrieve TRT Phase"};
  SG::ReadCondHandleKey<InDetDD::TRT_DetElementContainer> m_trtDetEleContKey
    {this, "TRTDetEleContKey", "TRT_DetElementContainer",
     "Key of TRT_DetElementContainer for TRT"};

  ToolHandle< ITRT_DriftFunctionTool > m_driftFunctionTool
    {this, "TRTDriftFunctionTool", "TRT_DriftFunctionTool"};
  ToolHandle<ITRT_StrawStatusSummaryTool> m_ConditionsSummary
    {this, "ConditionsSummaryTool", "TRT_StrawStatusSummaryTool",
     "The ConditionsSummaryTool"};

  BooleanProperty m_useConditionsStatus{this, "UseConditionsStatus", false,
    "Shall the ConditionsSummaryTool be used?"};
  const TRT_ID* m_trtid = nullptr;  //!< TRT id helper handle
  FloatProperty m_global_offset{this, "GlobalPhaseOffset", 0.,
    "Global offset to TRT Phase"};
  BooleanProperty m_useToTCorrection{this, "useDriftTimeToTCorrection", false,
    "Shall the Time over Threshold correction be used?"};
  BooleanProperty m_useHTCorrection{this, "useDriftTimeHTCorrection", false,
    "Shall the High Threshold correction be used?"};

};

} // end of namespace

#endif // TRT_DriftCircleToolCosmics_H
