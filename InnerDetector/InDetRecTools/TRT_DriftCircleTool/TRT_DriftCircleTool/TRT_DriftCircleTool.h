/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
//  Header file for class  TRT_DriftCircleTool
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Interface for TRT RDI collection production
///////////////////////////////////////////////////////////////////
// Version 1.0 18/02/2003 I.Gavrilenko
///////////////////////////////////////////////////////////////////

#ifndef TRT_DriftCircleTool_H
#define TRT_DriftCircleTool_H

#include "TRT_DriftCircleTool/ITRT_DriftCircleTool.h"

#include "AthenaBaseComps/AthAlgTool.h"
#include "InDetRawData/InDetRawDataCLASS_DEF.h"
#include "TrkPrepRawData/PrepRawDataCLASS_DEF.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TRT_ConditionsServices/ITRT_StrawStatusSummaryTool.h"
#include "TRT_DriftFunctionTool/ITRT_DriftFunctionTool.h"
#include "TRT_ReadoutGeometry/TRT_DetElementContainer.h"

#include "LumiBlockData/LuminosityCondData.h"
#include "StoreGate/ReadCondHandleKey.h"

class IInDetConditionsSvc;
class TRT_ID;
class TRT_RDORawData;
class TRT_DriftCircleCollection;
class TRT_DriftCircle;


namespace InDet {

/** @class TRT_DriftCircleTool
Class for converting a RDO collection to a DriftCircle collection
Performs trigger phase corrections in case of CTB data
*/
class TRT_DriftCircleTool final: public AthAlgTool, virtual public ITRT_DriftCircleTool
 
{
  ///////////////////////////////////////////////////////////////////
  // Public methods:
  ///////////////////////////////////////////////////////////////////

public:
  /** constructor */
  TRT_DriftCircleTool(const std::string&,const std::string&,const IInterface*);
  /** destructor */
  virtual ~TRT_DriftCircleTool ();
  /** initialize needed services */
  virtual StatusCode initialize() override;
  /** finalize */
  virtual StatusCode finalize  () override;
  /** make the conversion from RDOs to DriftCircles */
  virtual InDet::TRT_DriftCircleCollection*  
  convert(int,
          const InDetRawDataCollection<TRT_RDORawData>*,
          const EventContext& ctx,
          DataPool<TRT_DriftCircle>* dataItemsPool,
          const bool CTBBadChannels) const override;
  /** test validity gate for corrected drift times */
  virtual bool passValidityGate(unsigned int word, float lowGate, float highGate, float t0) const override;

  ///////////////////////////////////////////////////////////////////
  // Private methods:
  ///////////////////////////////////////////////////////////////////
  
 private:

  ///////////////////////////////////////////////////////////////////
  // Private data:
  ///////////////////////////////////////////////////////////////////
  SG::ReadCondHandleKey<InDetDD::TRT_DetElementContainer> m_trtDetEleContKey{this, "TRTDetEleContKey", "TRT_DetElementContainer", "Key of TRT_DetElementContainer for TRT"};
  SG::ReadCondHandleKey<LuminosityCondData>     m_lumiDataKey
      {this, "LumiDataKey", "", "SG key for luminosity data"};

  ToolHandle< ITRT_DriftFunctionTool > m_driftFunctionTool{
    this, "TRTDriftFunctionTool", "TRT_DriftFunctionTool"};
  ToolHandle<ITRT_StrawStatusSummaryTool> m_ConditionsSummary{
    this, "ConditionsSummaryTool", "TRT_StrawStatusSummaryTool"};

  BooleanProperty m_useConditionsStatus{this, "UseConditionsStatus", false,
    "Shall the ConditionsSummaryTool be used?"};
  BooleanProperty m_useConditionsHTStatus{this, "UseConditionsHTStatus", false,
    "Shall the ConditionsSummaryTool be used for HT to find argon straws?"};
  BooleanProperty m_useToTCorrection{this, "useDriftTimeToTCorrection", false,
    "Shall the Time over Threshold correction be used?"};
  BooleanProperty m_useHTCorrection{this, "useDriftTimeHTCorrection", false,
    "Shall the High Threshold correction be used?"};
  const TRT_ID* m_trtid = nullptr; //!< ID helper
  BooleanProperty m_reject_if_first_bit{this, "RejectIfFirstBit", false,
    "If true, reject this DC if first bit high"};
  BooleanProperty m_reject_if_first_bit_argon{this, "RejectIfFirstBitArgon", true,
    "If true, reject this DC if first bit high"};
  FloatProperty m_min_trailing_edge{this, "MinTrailingEdge", 11.0*CLHEP::ns,
    "Min raw trailing edge position to not reject this DC"};
  FloatProperty m_min_trailing_edge_argon{this, "MinTrailingEdgeArgon", 11.0*CLHEP::ns,
    "Min raw trailing edge position to not reject this DC"};
  FloatProperty m_max_drift_time{this, "MaxDriftTime", 60.0*CLHEP::ns,
    "Max allowed raw drift time to not reject this DC"};
  FloatProperty m_max_drift_time_argon{this, "MaxDriftTimeArgon", 60.0*CLHEP::ns,
    "Max allowed raw drift time to not reject this DC"};
  BooleanProperty m_out_of_time_supression{this, "SimpleOutOfTimePileupSupression", false,
    "Turn this on to turn on the OOT options!"};
  BooleanProperty m_out_of_time_supression_argon
    {this, "SimpleOutOfTimePileupSupressionArgon", false,
     "Turn this on to turn on the OOT options!"};
  BooleanProperty m_validity_gate_suppression{this, "ValidityGateSuppression", false,
    "Turn on suppression using validity gates"};
  BooleanProperty m_validity_gate_suppression_argon
    {this, "ValidityGateSuppressionArgon", false, "Turn on suppression using validity gates"};
  FloatProperty m_low_gate{this, "LowGate", 18.0*CLHEP::ns, "Low value for gate"};
  FloatProperty m_low_gate_argon{this, "LowGateArgon", 18.0*CLHEP::ns, "Low value for gate"};
  FloatProperty m_high_gate{this, "HighGate", 38.0*CLHEP::ns, "High value for gate"};
  FloatProperty m_high_gate_argon{this, "HighGateArgon", 38.0*CLHEP::ns, "High value for gate"};
};

} // end of namespace

#endif // TRT_DriftCircleTool_H
