/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TrigPartialEventBuilding_PEBInfoWriterAlg_h
#define TrigPartialEventBuilding_PEBInfoWriterAlg_h

#include "DecisionHandling/HypoBase.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"
#include "TrigPartialEventBuilding/PEBInfoWriterToolBase.h"
#include "Gaudi/Parsers/Factory.h"

/** @class PEBInfoWriterAlg
 *  @brief Pass-through hypo algorithm writing decisions with attached partial event building information
 **/
class PEBInfoWriterAlg : public HypoBase {
public:
  /// Standard constructor
  PEBInfoWriterAlg(const std::string& name, ISvcLocator* svcLoc);
  /// Standard destructor
  virtual ~PEBInfoWriterAlg();

  // ------------------------- AthReentrantAlgorithm methods -------------------
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  virtual StatusCode execute(const EventContext& eventContext) const override;

private:
  ToolHandleArray<PEBInfoWriterToolBase> m_hypoTools {this, "HypoTools", {}, "Tools to create the PEB Info"};
  SG::ReadHandleKey<xAOD::EventInfo> m_eventInfoKey {this,"EventInfo","EventInfo","input data key"};
  Gaudi::Property<std::vector<uint8_t> > m_matchTriggerType {this, "MatchTriggerType", {}, "L1 trigger type to match"};
  SG::WriteHandleKey< TrigRoiDescriptorCollection > m_superRoisWriteHandleKey {this,"SuperRoisWriteHandleKey","",
    "Name of the Super ROI collection produced by this tool."};

};

#endif // TrigPartialEventBuilding_PEBInfoWriterAlg_h
