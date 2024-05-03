/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef L1TOPOONLINEMONITORING_L1TopoOnlineMonitor_h
#define L1TOPOONLINEMONITORING_L1TopoOnlineMonitor_h

// Trigger includes
#include "xAODTrigL1Calo/L1TopoRawDataContainer.h"
#include "xAODTrigger/L1TopoSimResultsContainer.h"
#include "TrigT1Interfaces/TrigT1StoreGateKeys.h"
#include "TrigT1Result/CTP_RDO.h"
#include "TrigConfData/L1Menu.h"


// Athena includes
#include "AthenaMonitoring/AthMonitorAlgorithm.h"
#include "AthenaMonitoringKernel/Monitored.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteHandleKey.h"

#include "GaudiKernel/LockedHandle.h"

// System includes
#include <bitset>
#include <optional>

/**
 *  @class L1TopoOnlineMonitor
 *  @brief Algorithm to monitor L1Topo transmission and simulation errors for the (Run-3) L1Topo boards
 *  @author Anil Sonay
 *
 *  This algorithm tend to catch L1Topo sim xAOD to record decision bits into online monitoring
 *  Decision bits are from phase1 simulations or hardware
 **/

class L1TopoOnlineMonitor : public AthMonitorAlgorithm {
public:
  L1TopoOnlineMonitor(const std::string& name, ISvcLocator* svcLoc);
  
  // ------------------------- AthReentrantAlgorithm methods -------------------
  virtual StatusCode initialize() override;
  virtual StatusCode start() override;
  virtual StatusCode fillHistograms( const EventContext& ctx ) const override;

private:
  // ------------------------- Private types and constants ---------------------
  /// Number of CTP outputs, used for histogram ranges and loops
  static constexpr size_t s_nTopoCTPOutputs{128};
  
  /// Helper structure holding decision bitsets, passed between the monitoring methods
  struct DecisionBits {
    std::optional<std::bitset<s_nTopoCTPOutputs>> triggerBits;
    std::optional<std::bitset<s_nTopoCTPOutputs>> overflowBits;
    std::optional<std::bitset<s_nTopoCTPOutputs>> triggerBitsSim;
    std::optional<std::bitset<s_nTopoCTPOutputs>> overflowBitsSim;
    std::optional<std::bitset<s_nTopoCTPOutputs>> ambiguityBitsSim;
    std::optional<std::bitset<s_nTopoCTPOutputs>> triggerBitsCtp;
    /// Helper method to create a new bitset and get a reference to it
    static std::bitset<s_nTopoCTPOutputs>& createBits(std::optional<std::bitset<s_nTopoCTPOutputs>>& opt) {
      opt = std::bitset<s_nTopoCTPOutputs>{};
      return opt.value();
    }
  };

  std::unique_ptr<float[]> m_rateHdwNotSim;
  std::unique_ptr<float[]> m_rateSimNotHdw;
  std::unique_ptr<float[]> m_rateHdwAndSim;
  std::unique_ptr<float[]> m_rateHdwSim;
  std::unique_ptr<float[]> m_countHdwNotSim;
  std::unique_ptr<float[]> m_countSimNotHdw;
  std::unique_ptr<float[]> m_countHdwSim;
  std::unique_ptr<float[]> m_countHdw;
  std::unique_ptr<float[]> m_countSim;
  std::unique_ptr<float[]> m_countAny;
  std::unique_ptr<float[]> m_overflow_rateHdwNotSim;
  std::unique_ptr<float[]> m_overflow_rateSimNotHdw;
  std::unique_ptr<float[]> m_overflow_rateHdwAndSim;
  std::unique_ptr<float[]> m_overflow_rateHdwSim;
  std::unique_ptr<float[]> m_overflow_countHdwNotSim;
  std::unique_ptr<float[]> m_overflow_countSimNotHdw;
  std::unique_ptr<float[]> m_overflow_countHdwSim;
  std::unique_ptr<float[]> m_overflow_countHdw;
  std::unique_ptr<float[]> m_overflow_countSim;
  std::unique_ptr<float[]> m_overflow_countAny;

  std::vector<unsigned> m_ctpIds;

  std::vector<std::vector<std::pair<unsigned,unsigned>>> m_startbit;

  //RateVars m_rateVars{};
  // ------------------------- Properties and handles --------------------------
  ToolHandle<GenericMonitoringTool> m_monTool {
    this, "MonTool", "" ,
    "Monitoring tool to create online histograms"};
  SG::WriteHandleKey<xAOD::TrigCompositeContainer> m_errorFlagsKey {
    this, "ErrorFlagsKey", "L1TopoErrorFlags",
    "Key of the output TrigCompositeContainer with L1Topo error flags"};
  Gaudi::Property<bool> m_doSimMon {
    this, "doSimMon", true, "Enable L1Topo simulation decision monitoring"};
  Gaudi::Property<bool> m_doHwMonCTP {
    this, "doHwMonCTP", true, "Enable L1Topo HW readout from CTP"};
  Gaudi::Property<bool> m_doHwMon {
    this, "doHwMon", true, "Enable L1Topo HW readout from RAW"};
  Gaudi::Property<bool> m_doComp {
    this, "doComp", true, "Enable L1Topo HW/Sim comparison"};
  Gaudi::Property<bool> m_doMultComp {
    this, "doMultComp", false, "Enable L1Topo Multiplicity HW/Sim comparison"};
  Gaudi::Property<bool> m_forceCTPasHdw {
    this, "forceCTPasHdw", false, "Force to CTP monitoring as primary in Sim/Hdw comparison"};

  ServiceHandle<StoreGateSvc> m_detStore { this, "DetectorStore", "StoreGateSvc/DetectorStore", "Detector store to get the menu" };

  
  SG::ReadHandleKey<xAOD::L1TopoSimResultsContainer> m_l1topoKey {
    this, "L1_TopoKey", "L1_TopoSimResults", "l1topo EDM"};
  
  SG::ReadHandleKey<xAOD::L1TopoRawDataContainer> m_l1topoRawDataKey {
    this, "L1_TopoRawDataKey", "L1_Phase1L1TopoRAWData", "l1topo Raw Data"};
  
  
  SG::ReadHandleKey<CTP_RDO> m_ctpRdoKey {
    this, "CTPRDOLocation", LVL1CTP::DEFAULT_RDOOutputLocation,
    "Key of the CTP RDO object"};

  // ------------------------- Private methods ---------------------------------
  
  /// Monitor the simulated bits
  StatusCode doSimMon(DecisionBits& decisionBits, std::vector<std::vector<unsigned>> &multWeights, const EventContext& ctx) const;
  
  /// Monitor the Hw bits from CTP
  StatusCode doHwMonCTP(DecisionBits& decisionBits, const EventContext& ctx) const;
  
  /// Monitor the Hw bits from RAW data
  StatusCode doHwMon(DecisionBits& decisionBits, std::vector<std::vector<unsigned>> &multWeights, const EventContext& ctx) const;
  
  /// Compare hardware and simulation
  StatusCode doComp(DecisionBits& decisionBits) const;

  /// Compare hardware and simulation for the multiplicity algorithms
  StatusCode doMultComp(std::vector<std::vector<unsigned>> &multWeightsSim, std::vector<std::vector<unsigned>> &multWeightsHdw) const;
    
  /// Get CTP ids from menu
  std::vector<unsigned> getCtpIds(const TrigConf::L1Menu& l1menu);

  std::vector<std::vector<std::pair<unsigned,unsigned>>> getStartBits(const TrigConf::L1Menu& l1menu);

  // Define and set all error flags to false (Now only TrivialFlag set to true)
  void resetFlags(xAOD::TrigComposite& errorFlags) const;

};

#endif // L1TOPOONLINEMONITORING_L1TopoOnlineMonitor_h
