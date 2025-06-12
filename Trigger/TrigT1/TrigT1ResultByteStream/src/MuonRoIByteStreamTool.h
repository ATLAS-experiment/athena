/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRIGT1RESULTBYTESTREAM_MUONROIBYTESTREAMTOOL_H
#define TRIGT1RESULTBYTESTREAM_MUONROIBYTESTREAMTOOL_H


// Trigger includes
#include "TrigT1ResultByteStream/IL1TriggerByteStreamTool.h"
#include "TrigT1Interfaces/ITrigT1MuonRecRoiTool.h"
#include "TrigT1Interfaces/ITrigThresholdDecisionTool.h"
#include "xAODTrigger/MuonRoIContainer.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"
#include "TrigT1MuctpiPhase1/L1TopoLUT.h"

// Athena includes
#include "AthenaBaseComps/AthAlgTool.h"
#include "AthenaMonitoringKernel/Monitored.h"

/** @class MuonRoIByteStreamTool
 *  @brief Tool for converting MUCTPI ROB from BS to xAOD and from xAOD to BS
 *  (IL1TriggerByteStreamTool interface)
 **/
class MuonRoIByteStreamTool : public extends<AthAlgTool, IL1TriggerByteStreamTool> {
public:
  MuonRoIByteStreamTool(const std::string& type, const std::string& name, const IInterface* parent);
  virtual ~MuonRoIByteStreamTool() override = default;

  // ------------------------- IAlgTool methods --------------------------------
  virtual StatusCode initialize() override;

  // ------------------------- IL1TriggerByteStreamTool methods ----------------
  /// BS->xAOD conversion
  virtual StatusCode convertFromBS(const std::vector<const OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment*>& vrobf,
                                   const EventContext& eventContext) const override;
  /// xAOD->BS conversion
  virtual StatusCode convertToBS(std::vector<OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment*>& vrobf,
                                 const EventContext& eventContext) override;
  /// Declare ROB IDs for conversion
  virtual const std::vector<uint32_t>& robIds() const override {return m_robIds.value();}

private:
  // ------------------------- Tool handles ------------------------------------
  ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_rpcTool{
    this, "RPCRecRoiTool", "LVL1::TrigT1RPCRecRoiTool/LVL1__TrigT1RPCRecRoiTool",
    "Tool to get the eta/phi coordinates in the RPC"};
  ToolHandle<LVL1::ITrigT1MuonRecRoiTool> m_tgcTool{
    this, "TGCRecRoiTool", "LVL1::TrigT1TGCRecRoiTool/LVL1__TrigT1TGCRecRoiTool",
    "Tool to get the eta/phi coordinates in the TGC"};
  ToolHandle<LVL1::ITrigThresholdDecisionTool> m_thresholdTool{
    this, "TrigThresholdDecisionTool", "LVL1::TrigThresholdDecisionTool/LVL1__TrigThresholdDecisionTool",
    "Tool to get pass/fail of each trigger threshold"};
  ToolHandle<GenericMonitoringTool> m_monTool{
    this, "MonTool", "", "Monitoring tool"};

  // ------------------------- Data handles ------------------------------------
  // Only write keys should be set to non-empty array in python configuration if the tool is in BS->xAOD mode of operation
  SG::WriteHandleKeyArray<xAOD::MuonRoIContainer> m_roiWriteKeys {
    this, "MuonRoIContainerWriteKeys", {},
    "Write handle keys to MuonRoIContainer for conversion from ByteStream (one key per BC in the readout window)"};
  // Only read keys should be set to non-empty array in python configuration if the tool is in xAOD->BS mode of operation
  SG::ReadHandleKeyArray<xAOD::MuonRoIContainer> m_roiReadKeys {
    this, "MuonRoIContainerReadKeys", {},
    "Read handle keys to MuonRoIContainer for conversion to ByteStream (one key per BC in the readout window)"};
  SG::WriteHandleKeyArray<xAOD::MuonRoIContainer> m_MuCTPIL1TopoKeys {
    this, "L1TopoOutputLocID", {},
    "Output keys for MuCTPItoL1Topo, one per time slice"};

  // ------------------------- Other properties --------------------------------
  Gaudi::Property<std::vector<uint32_t>> m_robIds {
    this, "ROBIDs", {}, "List of ROB IDs required for conversion to/from xAOD RoI"};
  Gaudi::Property<bool> m_doTopo {
    this, "DoTopo", false, "Enable decoding/encoding MUCTPI Topo TOBs"};
  Gaudi::Property<bool> m_writeDecodedMuonRoIs {
    this, "WriteDecodedMuonRoIs", true, "Write the MuonRoIs"};

  const std::string m_barrelRoIFile            = "TrigConfMuctpi/Data_ROI_Mapping_Barrel_040422.txt";
  const std::string m_ecfRoIFile               = "TrigConfMuctpi/Data_RoI_Mapping_EF_040422.txt";
  const std::string m_side0LUTFile             = "TrigConfMuctpi/lookup_0_040422.json";
  const std::string m_side1LUTFile             = "TrigConfMuctpi/lookup_1_040422.json";

  // ------------------------- Helper members ----------------------------------
  /// Expected readout window size calculated from the size of data handle key arrays, should be 1, 3 or 5
  short int m_readoutWindow{-1};
  /// Muctpi Topo TOB word lookup table interface: hemi, detector, sector, roi(EC&FWD)/lut-numbers(Barrel) -> eta & phi coordinates - also L1Muon pT-Threshold mapping 
  LVL1MUCTPIPHASE1::L1TopoLUT m_l1topoLUT ;
  /// Process raw RoI candidate words in all slices, convert and fill the output EDM
  StatusCode decodeRoiSlices(const uint32_t* data,
                             const std::vector<std::pair<size_t,size_t>>& slices,
                             std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>>& handles,
                             size_t outputOffset,
                             const EventContext& eventContext) const;
  /// Process raw Topo TOB words in all slices, convert and fill the output EDM
  StatusCode decodeTopoSlices(const uint32_t* data,
                              const std::vector<std::pair<size_t,size_t>>& slices,
                              std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>>& handles,
                              size_t outputOffset,
                              const EventContext& eventContext) const;
};

#endif // TRIGT1RESULTBYTESTREAM_MUONROIBYTESTREAMTOOL_H
