/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "L1TopoOnlineMonitor.h"

// Trigger includes
#include "xAODTrigger/L1TopoSimResults.h"
#include "TrigT1Result/CTP_Decoder.h"
#include "L1TopoRDO/Helpers.h"
#include "L1TopoRDO/L1TopoROD.h"
#include "L1TopoRDO/L1TopoFPGA.h"
#include "L1TopoRDO/L1TopoResult.h"

// System includes
#include <utility>


// Local helper constants and functions
namespace {
  /// Return a vector of indices corresponding to bits set to 1 in the bitset
  template<size_t N>
  std::vector<size_t> bitsetIndices(const std::bitset<N>& bits) {
    std::vector<size_t> indices;
    indices.reserve(bits.count());
    for (size_t i=0; i<bits.size(); ++i) {
      if (bits[i]) indices.push_back(i);
    }
    return indices;
  }
}

// =============================================================================
// Standard constructor
// =============================================================================
L1TopoOnlineMonitor::L1TopoOnlineMonitor(const std::string& name, ISvcLocator* svcLoc)
  : AthMonitorAlgorithm(name, svcLoc),
    m_ctpIds(0)
{}

// =============================================================================
// Implementation of AthReentrantAlgorithm::initialize
// =============================================================================
StatusCode L1TopoOnlineMonitor::initialize() {

  m_rateHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_rateSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_rateHdwAndSim.reset(new float[s_nTopoCTPOutputs]);
  m_rateHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_countHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_countSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_countHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_countHdw.reset(new float[s_nTopoCTPOutputs]);
  m_countSim.reset(new float[s_nTopoCTPOutputs]);
  m_countAny.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_rateHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_rateSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_rateHdwAndSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_rateHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countHdwNotSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countSimNotHdw.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countHdwSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countHdw.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countSim.reset(new float[s_nTopoCTPOutputs]);
  m_overflow_countAny.reset(new float[s_nTopoCTPOutputs]);

  for (size_t i=0;i<s_nTopoCTPOutputs;i++){
    m_rateHdwNotSim[i] = 0;
    m_rateSimNotHdw[i] = 0;
    m_rateHdwAndSim[i] = 0;
    m_rateHdwSim[i] = 0;
    m_countHdwNotSim[i] = 0;
    m_countSimNotHdw[i] = 0;
    m_countHdwSim[i] = 0;
    m_countHdw[i] = 0;
    m_countSim[i] = 0;
    m_countAny[i] = 0;
    m_overflow_rateHdwNotSim[i] = 0;
    m_overflow_rateSimNotHdw[i] = 0;
    m_overflow_rateHdwAndSim[i] = 0;
    m_overflow_rateHdwSim[i] = 0;
    m_overflow_countHdwNotSim[i] = 0;
    m_overflow_countSimNotHdw[i] = 0;
    m_overflow_countHdwSim[i] = 0;
    m_overflow_countHdw[i] = 0;
    m_overflow_countSim[i] = 0;
    m_overflow_countAny[i] = 0;
  }

  ATH_CHECK(m_l1topoKey.initialize());
  ATH_CHECK(m_ctpRdoKey.initialize(m_doHwMonCTP));
  ATH_CHECK(m_l1topoRawDataKey.initialize(m_doHwMon));
  ATH_CHECK(m_monTool.retrieve(DisableTool{m_monTool.name().empty()}));
  ATH_CHECK(m_errorFlagsKey.initialize());

  return AthMonitorAlgorithm::initialize();
}

StatusCode L1TopoOnlineMonitor::start() {

   const TrigConf::L1Menu * l1menu = nullptr;
   ATH_CHECK( m_detStore->retrieve(l1menu) ); 

   m_ctpIds = getCtpIds(*l1menu);

   m_startbit = getStartBits(*l1menu);

   return StatusCode::SUCCESS;
}


StatusCode L1TopoOnlineMonitor::fillHistograms( const EventContext& ctx ) const {
  
  // Create and record the ErrorFlags container
  SG::WriteHandle<xAOD::TrigCompositeContainer> errorFlagsCont(m_errorFlagsKey, ctx);
  ATH_CHECK(errorFlagsCont.record(std::make_unique<xAOD::TrigCompositeContainer>(),
                              std::make_unique<xAOD::TrigCompositeAuxContainer>()));
  ATH_MSG_DEBUG("Recorded TrigCompositeContainer with key " << m_errorFlagsKey.key());

  // Create the ErrorFlags object
  errorFlagsCont->push_back(new xAOD::TrigComposite);
  xAOD::TrigComposite& errorFlags = *(errorFlagsCont->back());
  resetFlags(errorFlags);


  DecisionBits decisionBits{};
  enum class MonFunction : uint8_t {doSimMon=0, doHwMonCTP, doHwMon, doComp, doMultComp};
  std::vector<uint8_t> failedMonFunctions;
  std::vector<std::vector<unsigned>> multWeightsSim;
  std::vector<std::vector<unsigned>> multWeightsHdw;

  
  if (m_doHwMon) {
    StatusCode sc = doHwMon(decisionBits,multWeightsHdw,ctx);
    ATH_MSG_DEBUG("Executed doHWMon: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doHwMon));
    }    
  }

  if (m_doHwMonCTP) {
    StatusCode sc = doHwMonCTP(decisionBits,ctx);
    ATH_MSG_DEBUG("Executed doHWMonCTP: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doHwMonCTP));
    }    
  }
  
  if (m_doSimMon) {
    StatusCode sc = doSimMon(decisionBits,multWeightsSim,ctx);
    ATH_MSG_DEBUG("Executed doSimMon: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doSimMon));
    }    
  }

  if (m_doComp) {
    StatusCode sc = doComp(decisionBits);
    ATH_MSG_DEBUG("Executed doComp: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doComp));
    }    
  }

  if (m_doMultComp) {
    StatusCode sc = doMultComp(multWeightsSim,multWeightsHdw);
    ATH_MSG_DEBUG("Executed doMultComp: " << (sc.isFailure() ? "failed" : "ok"));
    if (sc.isFailure()) {
      failedMonFunctions.push_back(static_cast<uint8_t>(MonFunction::doMultComp));
    }    
  }

  auto monFailedMonFunctions = Monitored::Collection("MonitoringFailures", failedMonFunctions);
  Monitored::Group(m_monTool, monFailedMonFunctions);

  return StatusCode::SUCCESS;
}


StatusCode L1TopoOnlineMonitor::doSimMon( DecisionBits& decisionBits, std::vector<std::vector<unsigned>> &multWeights, const EventContext& ctx ) const {
  
  
  SG::ReadHandle<xAOD::L1TopoSimResultsContainer> cont(m_l1topoKey, ctx);
  if(!cont.isValid()){
    ATH_MSG_FATAL("Could not retrieve L1Topo EDM Container from the Simulation.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("----got L1Topo container: " << cont.key());

  std::bitset<s_nTopoCTPOutputs>& triggerBitsSim = DecisionBits::createBits(decisionBits.triggerBitsSim);
  std::bitset<s_nTopoCTPOutputs>& overflowBitsSim = DecisionBits::createBits(decisionBits.overflowBitsSim);
  std::bitset<s_nTopoCTPOutputs>& ambiguityBitsSim = DecisionBits::createBits(decisionBits.ambiguityBitsSim);
  std::unordered_map<unsigned,std::bitset<s_nTopoCTPOutputs>> multWeightsMap;
  for(const auto l1topo_dec : * cont){
    ATH_MSG_DEBUG( "Reading L1Topo EDM:: Connection ID: " << l1topo_dec->connectionId() << " Clock: " << l1topo_dec->clock() << " Bit-length: " << l1topo_dec->bitWidth() << " Word: " << l1topo_dec->topoWord() << " Word64: " << l1topo_dec->topoWord64() );

    if (l1topo_dec->bitWidth() == 32) {
      std::vector<unsigned> topoword;
      std::vector<unsigned> topowordOverflow;
      for(unsigned int i=0; i<32; ++i) {
        uint32_t mask = 0x1; mask <<= i;
        if ((l1topo_dec->topoWord() & mask) !=0) {
          if (l1topo_dec->connectionId()==2 || l1topo_dec->connectionId()==3) { // TOPO2EL and TOPO3EL (L1TopoCommon/Types.h)
            topoword.push_back(32*l1topo_dec->clock()+i);
            uint32_t pos = 32*(l1topo_dec->clock()+(l1topo_dec->connectionId()==2 ? 0 : 2))+i;
            triggerBitsSim[pos] = ((!decisionBits.triggerBits.has_value() || m_forceCTPasHdw) && m_ctpIds[pos]>=512) ? false : true;
          }
          if (l1topo_dec->connectionId()==22 || l1topo_dec->connectionId()==23) { // AMBIGUITYTOPO2EL and AMBIGUITYTOPO3EL
            uint32_t pos_ambiguity = 32*(l1topo_dec->clock()+(l1topo_dec->connectionId()==22 ? 0 : 2))+i;
            ambiguityBitsSim[pos_ambiguity] = ((!decisionBits.ambiguityBitsSim.has_value() || m_forceCTPasHdw) && m_ctpIds[pos_ambiguity]>=512) ? false : true;
          }
	}
	if ((l1topo_dec->topoWordOverflow() & mask) !=0) {
	  topowordOverflow.push_back(32*l1topo_dec->clock()+i);
	  uint32_t pus = 32*(l1topo_dec->clock()+(l1topo_dec->connectionId()==12 ? 0 : 2))+i;
	  overflowBitsSim[pus] = ((!decisionBits.overflowBitsSim.has_value() || m_forceCTPasHdw) && m_ctpIds[pus]>=512) ? false : true;
	}
      }
      std::string name = "CableElec_";
      name += std::to_string(l1topo_dec->connectionId());
      auto monTopoDec = Monitored::Collection(name, topoword);
      Monitored::Group(m_monTool,monTopoDec);
    }
    else if (l1topo_dec->bitWidth() == 64) {
      for (size_t i=0;i<64;i++) {
        unsigned index = i+l1topo_dec->clock()*64;
        uint64_t mask = 0x1; mask <<= i;
        if ((l1topo_dec->topoWord64() & mask) !=0) {
          multWeightsMap[static_cast<unsigned>(l1topo_dec->connectionId() - 4)].set(index);
        }
      }
    }
    else {
      ATH_MSG_DEBUG( "Unknown Bit-length: " << l1topo_dec->bitWidth() );
      return StatusCode::FAILURE;
    }
  }
  
  for (unsigned key=0;key<4;key++) {
    std::vector<unsigned> vecCount, vecIndices;
    unsigned indices = 0;
    for (auto startbit : m_startbit[key]) {
      unsigned count = 0;
      for (size_t i=0;i<startbit.second;i++){
        if (multWeightsMap[key][startbit.first+i]) {
          count += 1 * pow(2,i);
        }
      }
      vecCount.push_back(count);
      vecIndices.push_back(indices);
      indices++;
    }
    multWeights.push_back(vecCount);
    std::string name = "CableOpti_"+std::to_string(key);
    auto monMult = Monitored::Collection(name, vecIndices);
    auto monMultWeight = Monitored::Collection(name+"_weight", vecCount);
    Monitored::Group(m_monTool,monMult,monMultWeight);
  }
  
  std::vector<size_t> triggerBitIndicesSim = bitsetIndices(triggerBitsSim);
  std::vector<size_t> overflowBitIndicesSim = bitsetIndices(overflowBitsSim);
  std::vector<size_t> ambiguityBitIndicesSim = bitsetIndices(ambiguityBitsSim);
  auto monTopoSim = Monitored::Collection("TopoSim", triggerBitIndicesSim);
  auto monTopoSimOverflow = Monitored::Collection("TopoSim_overflows", overflowBitIndicesSim);
  auto monTopoSimAmbiguity = Monitored::Collection("TopoSim_ambiguity", ambiguityBitIndicesSim);
  Monitored::Group(m_monTool,monTopoSim);
  Monitored::Group(m_monTool,monTopoSimOverflow);
  Monitored::Group(m_monTool,monTopoSimAmbiguity);
  
  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doHwMonCTP( DecisionBits& decisionBits, const EventContext& ctx ) const {
  
  // Retrieve CTP DAQ data for comparison
  SG::ReadHandle<CTP_RDO> ctpRdo{m_ctpRdoKey, ctx};
  if (!ctpRdo.isValid()) {
    ATH_MSG_DEBUG("Failed to retrieve CTP_RDO object (converted from CTP DAQ ROB) with key \""
                  << m_ctpRdoKey.key() << "\". Skipping CTP hardware comparison");
    return StatusCode::FAILURE;
  }

  // CTP RDO contains 17 TBP words for a number of BCs, so use CTP_Decoder to access accepted BC
  CTP_Decoder ctp;
  ctp.setRDO(ctpRdo.cptr());
  const uint32_t l1aPos = ctpRdo->getL1AcceptBunchPosition();
  if (l1aPos >= ctp.getBunchCrossings().size()) {
    ATH_MSG_DEBUG("CTP_RDO gave invalid l1aPos. Skipping CTP hardware comparison");
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("CTP l1aPos, size: " << l1aPos << ", " << ctp.getBunchCrossings().size());
  const CTP_BC& ctpL1a = ctp.getBunchCrossings().at(l1aPos);

  // Fill decision bits from CTP RDO
  std::bitset<s_nTopoCTPOutputs>& triggerBitsCtp = DecisionBits::createBits(decisionBits.triggerBitsCtp);
  static constexpr size_t ctpTBPSize{512};
  const std::bitset<ctpTBPSize>& tbp = ctpL1a.getTBP();
  ATH_MSG_VERBOSE("CTP TBP bits: " << tbp.to_string());

  for (size_t i=0; i<s_nTopoCTPOutputs; ++i) {
    if (m_ctpIds[i] < 512)
      {triggerBitsCtp[i] = tbp.test(m_ctpIds[i]);}
    else
      {triggerBitsCtp[i] = false;}
  }

  std::vector<size_t> triggerBitIndicesCtp = bitsetIndices(triggerBitsCtp);
  auto monTopoCtp = Monitored::Collection("TopoCTP", triggerBitIndicesCtp);
  Monitored::Group(m_monTool,monTopoCtp);

  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doHwMon( DecisionBits& decisionBits, std::vector<std::vector<unsigned>> &multWeights, const EventContext& ctx ) const {
  
  SG::ReadHandle<xAOD::L1TopoRawDataContainer> cont(m_l1topoRawDataKey, ctx);
  if(!cont.isValid()){
    ATH_MSG_WARNING("Could not retrieve L1Topo RAW Data Container from the BS data.");
    return StatusCode::FAILURE;
  }

  ATH_MSG_DEBUG("----got L1Topo Raw Data container: " << cont.key());

  std::bitset<s_nTopoCTPOutputs>& triggerBits = DecisionBits::createBits(decisionBits.triggerBits);
  std::bitset<s_nTopoCTPOutputs>& overflowBits = DecisionBits::createBits(decisionBits.overflowBits);

  std::unique_ptr<L1Topo::L1TopoResult> l1topoResult = std::make_unique<L1Topo::L1TopoResult>(*cont);
  if (!l1topoResult->getStatus()) {
    ATH_MSG_WARNING("Decoding L1Topo results failed!!");
    return StatusCode::FAILURE;
  }

  // Error monitoring ---------------------------------------------------------
  enum class MonFunction : uint8_t {doRODct, doRODpc, doRODhc, doRODpe, doRODlm, doRODhm, doRODpt};
  std::vector<uint8_t> rodErrors;
  if (l1topoResult->getROD(0)->ct() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODct)); }
  if (l1topoResult->getROD(0)->pc() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODpc)); }
  if (l1topoResult->getROD(0)->hc() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODhc)); }
  if (l1topoResult->getROD(0)->pe() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODpe)); }
  if (l1topoResult->getROD(0)->lm() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODlm)); }
  if (l1topoResult->getROD(0)->hm() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODhm)); }
  if (l1topoResult->getROD(0)->pt() != 0) { rodErrors.push_back(static_cast<uint8_t>(MonFunction::doRODpt)); }
  auto monErrorsROD = Monitored::Collection("ROD_Errors", rodErrors);
  Monitored::Group(m_monTool, monErrorsROD);

  for (unsigned i=0;i<l1topoResult->getFPGASize();i++) {
    unsigned topoNumber = l1topoResult->getFPGA(i)->topoNumber();
    unsigned fpgaNumber = l1topoResult->getFPGA(i)->fpgaNumber();

    auto mon_fpga_error = Monitored::Scalar<unsigned>("FPGA_Errors");
    auto mon_fpga_labels = Monitored::Scalar("FPGA_Labels", (topoNumber*2)-fpgaNumber-1);

    if (l1topoResult->getFPGA(i)->ct() != 0) {
      mon_fpga_error = 0;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
    if (l1topoResult->getFPGA(i)->sm() != 0) { 
      mon_fpga_error = 1;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
    if (l1topoResult->getFPGA(i)->pe() != 0) {
      mon_fpga_error = 2;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
    if (l1topoResult->getFPGA(i)->lm() != 0) {
      mon_fpga_error = 3;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
    if (l1topoResult->getFPGA(i)->hm() != 0) {
      mon_fpga_error = 4;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
    if (l1topoResult->getFPGA(i)->pt() != 0) {
      mon_fpga_error = 5;
      Monitored::Group(m_monTool, mon_fpga_error, mon_fpga_labels);
    }
  }
  
  // Multiplicities ---------------------------------------------------------
  std::vector<unsigned> topo1Opt0,topo1Opt1,topo1Opt2,topo1Opt3;
  std::vector<unsigned> topo1Opt0Indices,topo1Opt1Indices,topo1Opt2Indices,topo1Opt3Indices;

  unsigned indices=0;
  for (auto startbit : m_startbit[0]) {
    unsigned count = 0;
    for (size_t i=0;i<startbit.second;i++){
      if (l1topoResult->getTopo1Opt0()[startbit.first+i]) {
	count += 1 * pow(2,i);
      }
    }
    topo1Opt0.push_back(count);
    topo1Opt0Indices.push_back(indices);
    indices++;
  }
  indices=0;
  for (auto startbit : m_startbit[1]) {
    unsigned count = 0;
    for (size_t i=0;i<startbit.second;i++){
      if (l1topoResult->getTopo1Opt1()[startbit.first+i]) {
	count += 1 * pow(2,i);
      }
    }
    topo1Opt1.push_back(count);
    topo1Opt1Indices.push_back(indices);
    indices++;
  }
  indices=0;
  for (auto startbit : m_startbit[2]) {
    unsigned count = 0;
    for (size_t i=0;i<startbit.second;i++){
      if (l1topoResult->getTopo1Opt2()[startbit.first+i]) {
	count += 1 * pow(2,i);
      }
    }
    topo1Opt2.push_back(count);
    topo1Opt2Indices.push_back(indices);
    indices++;
  }
  indices=0;
  for (auto startbit : m_startbit[3]) {
    unsigned count = 0;
    for (size_t i=0;i<startbit.second;i++){
      if (l1topoResult->getTopo1Opt3()[startbit.first+i]) {
	count += 1 * pow(2,i);
      }
    }
    topo1Opt3.push_back(count);
    topo1Opt3Indices.push_back(indices);
    indices++;
  }

  auto monTopo1Opt0 = Monitored::Collection("HdwTopo1Opt0", topo1Opt0Indices);
  auto monTopo1Opt0Weight = Monitored::Collection("HdwTopo1Opt0_weight", topo1Opt0);
  Monitored::Group(m_monTool, monTopo1Opt0, monTopo1Opt0Weight);
  multWeights.push_back(topo1Opt0);

  auto monTopo1Opt1 = Monitored::Collection("HdwTopo1Opt1", topo1Opt1Indices);
  auto monTopo1Opt1Weight = Monitored::Collection("HdwTopo1Opt1_weight", topo1Opt1);
  Monitored::Group(m_monTool, monTopo1Opt1, monTopo1Opt1Weight);
  multWeights.push_back(topo1Opt1);

  auto monTopo1Opt2 = Monitored::Collection("HdwTopo1Opt2", topo1Opt2Indices);
  auto monTopo1Opt2Weight = Monitored::Collection("HdwTopo1Opt2_weight", topo1Opt2);
  Monitored::Group(m_monTool, monTopo1Opt2, monTopo1Opt2Weight);
  multWeights.push_back(topo1Opt2);

  auto monTopo1Opt3 = Monitored::Collection("HdwTopo1Opt3", topo1Opt3Indices);
  auto monTopo1Opt3Weight = Monitored::Collection("HdwTopo1Opt3_weight", topo1Opt3);
  Monitored::Group(m_monTool, monTopo1Opt3, monTopo1Opt3Weight);
  multWeights.push_back(topo1Opt3);
  
  // Decisions ---------------------------------------------------------------
  triggerBits = l1topoResult->getDecisions();
  overflowBits = l1topoResult->getOverflows();

  const std::vector<size_t> triggerBitIndicesHdw = bitsetIndices(triggerBits);
  const std::vector<size_t> overflowBitIndicesHdw = bitsetIndices(overflowBits);
  
  ATH_MSG_VERBOSE("trigger bits: " << triggerBits.to_string() );
  ATH_MSG_VERBOSE("overflow bits: " << overflowBits.to_string() );
 
  auto monHdw = Monitored::Collection("HdwResults", triggerBitIndicesHdw);
  auto monOverflow = Monitored::Collection("OverflowResults", overflowBitIndicesHdw);

  Monitored::Group(m_monTool, monHdw, monOverflow);

  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doComp( DecisionBits& decisionBits ) const {
  if (!decisionBits.triggerBitsSim.has_value()) {
    ATH_MSG_DEBUG("Simulation bits not set. Skipping simulation to hardware comparison");
    return StatusCode::FAILURE;
  }

  std::bitset<s_nTopoCTPOutputs> triggerBitsSim = decisionBits.triggerBitsSim.value(); // Alias
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdw;

  if (decisionBits.triggerBits.has_value() && !m_forceCTPasHdw)
    {triggerBitsHdw = decisionBits.triggerBits.value();}
  else if (decisionBits.triggerBitsCtp.has_value())
    {triggerBitsHdw = decisionBits.triggerBitsCtp.value();}
  else {
    ATH_MSG_DEBUG("Hardware bits not set. Skipping simulation to hardware comparison");
    return StatusCode::FAILURE;
  }

  std::bitset<s_nTopoCTPOutputs> triggerBitsSimNotHdw = triggerBitsSim & (~triggerBitsHdw);
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdwNotSim = triggerBitsHdw & (~triggerBitsSim);
  std::bitset<s_nTopoCTPOutputs> triggerBitsHdwSim = triggerBitsHdw & triggerBitsSim;
  std::bitset<s_nTopoCTPOutputs> triggerBitsAny = triggerBitsHdw | triggerBitsSim;

  std::bitset<s_nTopoCTPOutputs>& overflowBitsSim = decisionBits.overflowBitsSim.value();
  std::bitset<s_nTopoCTPOutputs>& overflowBitsHdw = decisionBits.overflowBits.value();
  std::bitset<s_nTopoCTPOutputs> overflowBitsSimNotHdw = overflowBitsSim & (~overflowBitsHdw);
  std::bitset<s_nTopoCTPOutputs> overflowBitsHdwNotSim = overflowBitsHdw & (~overflowBitsSim);
  std::bitset<s_nTopoCTPOutputs> overflowBitsHdwSim = overflowBitsHdw & overflowBitsSim;
  std::bitset<s_nTopoCTPOutputs> overflowBitsAny = overflowBitsHdw | overflowBitsSim;

  std::bitset<s_nTopoCTPOutputs>& ambiguityBitsSim = decisionBits.ambiguityBitsSim.value();
  std::bitset<s_nTopoCTPOutputs> ambiguitySimANDHdw = ambiguityBitsSim & triggerBitsHdwSim;
  std::bitset<s_nTopoCTPOutputs> ambiguityMismatch = ambiguityBitsSim & (triggerBitsSimNotHdw | triggerBitsHdwNotSim);

  std::vector<size_t> triggerBitIndicesSimNotHdw = bitsetIndices(triggerBitsSimNotHdw);
  std::vector<size_t> triggerBitIndicesHdwNotSim = bitsetIndices(triggerBitsHdwNotSim);
  std::vector<size_t> ambiguitySimANDHdwBitIndices = bitsetIndices(ambiguitySimANDHdw);
  std::vector<size_t> ambiguityMismatchBitIndices = bitsetIndices(ambiguityMismatch);
  auto monSimNotHdw = Monitored::Collection("SimNotHdwL1TopoResult", triggerBitIndicesSimNotHdw);
  auto monHdwNotSim = Monitored::Collection("HdwNotSimL1TopoResult", triggerBitIndicesHdwNotSim);
  auto monAmbiguitySimANDHdw = Monitored::Collection("Ambiguity_SimANDHdwDecisions", ambiguitySimANDHdwBitIndices);
  auto monAmbiguityMismatch = Monitored::Collection("Ambiguity_DecisionMismatches", ambiguityMismatchBitIndices);

  Monitored::Group(m_monTool, monSimNotHdw, monHdwNotSim, monAmbiguitySimANDHdw, monAmbiguityMismatch);

  float rate=0;
  float rate_overflow=0;
  for (size_t i=0;i<4;i++) {
    auto mon_trig = Monitored::Scalar<unsigned>("Phase1TopoTrigger_"+std::to_string(i));
    auto mon_match = Monitored::Scalar<unsigned>("Phase1TopoMissMatch_"+std::to_string(i));
    auto mon_weight = Monitored::Scalar<float>("Phase1TopoWeight_"+std::to_string(i));
    auto mon_OFweight = Monitored::Scalar<float>("Phase1TopoOFWeight_"+std::to_string(i));
    for (size_t j=0;j<32;j++) {
      if (ambiguityBitsSim[32*i+j] == 0) {
        mon_trig = static_cast<unsigned>(j);
        if (overflowBitsHdw[32*i+j] == 1 || overflowBitsSim[32*i+j] == 1) {
          m_overflow_countHdwNotSim[32*i+j]+=overflowBitsHdwNotSim[32*i+j];
          m_overflow_countSimNotHdw[32*i+j]+=overflowBitsSimNotHdw[32*i+j];
          m_overflow_countHdwSim[32*i+j]+=overflowBitsHdwSim[32*i+j];
          m_overflow_countHdw[32*i+j]+=overflowBitsHdw[32*i+j];
          m_overflow_countSim[32*i+j]+=overflowBitsSim[32*i+j];
          m_overflow_countAny[32*i+j]+=overflowBitsAny[32*i+j];
        }
        else {
          m_countHdwNotSim[32*i+j]+=triggerBitsHdwNotSim[32*i+j];
          m_countSimNotHdw[32*i+j]+=triggerBitsSimNotHdw[32*i+j];
          m_countHdwSim[32*i+j]+=triggerBitsHdwSim[32*i+j];
          m_countHdw[32*i+j]+=triggerBitsHdw[32*i+j];
          m_countSim[32*i+j]+=triggerBitsSim[32*i+j];
          m_countAny[32*i+j]+=triggerBitsAny[32*i+j];
        }     
        rate = m_countHdw[32*i+j]>0 ? m_countHdwNotSim[32*i+j]/m_countHdw[32*i+j] : 0;
        if (rate != m_rateHdwNotSim[32*i+j]) {
          mon_match = 0;
          mon_weight = rate-m_rateHdwNotSim[32*i+j];
          m_rateHdwNotSim[32*i+j] = rate;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
        }
        rate_overflow = m_overflow_countHdw[32*i+j]>0 ? m_overflow_countHdwNotSim[32*i+j]/m_overflow_countHdw[32*i+j] : 0;
        if (rate_overflow != m_overflow_rateHdwNotSim[32*i+j]) {
          mon_match = 0;
          mon_OFweight = rate_overflow-m_overflow_rateHdwNotSim[32*i+j];
          m_overflow_rateHdwNotSim[32*i+j] = rate_overflow;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_OFweight);
        }
        rate = m_countSim[32*i+j]>0 ? m_countSimNotHdw[32*i+j]/m_countSim[32*i+j] : 0;
        if (rate != m_rateSimNotHdw[32*i+j]) {
          mon_match = 1;
          mon_weight = rate-m_rateSimNotHdw[32*i+j];
          m_rateSimNotHdw[32*i+j] = rate;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
        }
        rate_overflow = m_overflow_countSim[32*i+j]>0 ? m_overflow_countSimNotHdw[32*i+j]/m_overflow_countSim[32*i+j] : 0;
        if (rate_overflow != m_overflow_rateSimNotHdw[32*i+j]) {
          mon_match = 1;
          mon_OFweight = rate_overflow-m_overflow_rateSimNotHdw[32*i+j];
          m_overflow_rateSimNotHdw[32*i+j] = rate_overflow;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_OFweight);
        }
        rate = m_countAny[32*i+j]>0 ? m_countHdwSim[32*i+j]/m_countAny[32*i+j] : 0;
        if (rate != m_rateHdwAndSim[32*i+j]) {
          mon_match = 2;
          mon_weight = rate-m_rateHdwAndSim[32*i+j];
          m_rateHdwAndSim[32*i+j] = rate;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
        }
        rate_overflow = m_overflow_countAny[32*i+j]>0 ? m_overflow_countHdwSim[32*i+j]/m_overflow_countAny[32*i+j] : 0;
        if (rate_overflow != m_overflow_rateHdwAndSim[32*i+j]) {
          mon_match = 2;
          mon_OFweight = rate_overflow-m_overflow_rateHdwAndSim[32*i+j];
          m_overflow_rateHdwAndSim[32*i+j] = rate_overflow;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_OFweight);
        }
        rate = m_countSim[32*i+j]>0 ? m_countHdw[32*i+j]/m_countSim[32*i+j] : 0;
        if (rate != m_rateHdwSim[32*i+j]) {
          mon_match = 3;
          mon_weight = rate-m_rateHdwSim[32*i+j];
          m_rateHdwSim[32*i+j] = rate;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_weight);
        }
        rate_overflow = m_overflow_countSim[32*i+j]>0 ? m_overflow_countHdw[32*i+j]/m_overflow_countSim[32*i+j] : 0;
        if (rate_overflow != m_overflow_rateHdwSim[32*i+j]) {
          mon_match = 3;
          mon_OFweight = rate_overflow-m_overflow_rateHdwSim[32*i+j];
          m_overflow_rateHdwSim[32*i+j] = rate_overflow;
          Monitored::Group(m_monTool, mon_trig, mon_match, mon_OFweight);
        }
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode L1TopoOnlineMonitor::doMultComp( std::vector<std::vector<unsigned>> &multWeightsSim, std::vector<std::vector<unsigned>> &multWeightsHdw ) const {
  if (multWeightsSim.size() == 0 or multWeightsHdw.size() == 0) {
    ATH_MSG_DEBUG("Multiplicities not set, skipping multiplicities comparison");
    return StatusCode::FAILURE;
  }

  for (size_t i=0;i<multWeightsSim.size();i++) {
    for (size_t k=0;k<multWeightsSim[i].size();k++) {
      std::string colName = "Topo1Opt" + std::to_string(i) + "_" + std::to_string(k);
      auto monMultSim = Monitored::Scalar<unsigned>(colName+"_Sim", multWeightsSim[i][k]);
      auto monMultHdw = Monitored::Scalar<unsigned>(colName+"_Hdw", multWeightsHdw[i][k]);
      Monitored::Group(m_monTool, monMultSim, monMultHdw);
    }
  }
  return StatusCode::SUCCESS;
}

std::vector<std::vector<std::pair<unsigned,unsigned>>> L1TopoOnlineMonitor::getStartBits( const TrigConf::L1Menu& l1menu ) {

  std::vector<std::vector<std::pair<unsigned,unsigned>>> startbit_vec;
  std::vector<std::string> connNames = l1menu.connectorNames();
  for( const std::string connName : {"Topo1Opt0", "Topo1Opt1", "Topo1Opt2", "Topo1Opt3"}) {
    if( find(connNames.begin(), connNames.end(), connName) == connNames.end() ) {
      continue;
    }
    std::vector<std::pair<unsigned,unsigned>> startbit;
	  for(auto & t1 : l1menu.connector(connName).triggerLines()) {
	    startbit.push_back(std::make_pair(t1.startbit(),t1.nbits()));
    }
    startbit_vec.push_back(startbit);
  }
  return startbit_vec;
}
// 

std::vector<unsigned> L1TopoOnlineMonitor::getCtpIds( const TrigConf::L1Menu& l1menu ) {
  
  // Topo
  std::vector<std::string> connNames = l1menu.connectorNames();
  std::vector<std::string> labelsTopoEl(s_nTopoCTPOutputs);
  for( const std::string connName : {"Topo2El", "Topo3El"}) {
    if( find(connNames.begin(), connNames.end(), connName) == connNames.end() ) {
      continue;
    }
    for(uint fpga : {0,1}) {
      for(uint clock : {0,1}) {
	for(auto & tl : l1menu.connector(connName).triggerLines(fpga,clock)) {
	  uint flatIndex = tl.flatindex() + 64 * (connName == "Topo3El"); 
	  labelsTopoEl[flatIndex] = tl.name();
	}
      }
    }
  }

  ATH_MSG_DEBUG("Obtaining CTPIds for Phase1 L1Topo Monitoring");
  std::vector<unsigned> ctpIds(s_nTopoCTPOutputs,999);
  for( const auto & item : l1menu ) {
    std::string definition = item.definition();
    if (definition.substr(0,5) == "TOPO_" &&
    definition.find(' ') == std::string::npos) {
      std::string trigger = definition.substr(0, definition.find('['));
      auto pos = std::find(labelsTopoEl.begin(),labelsTopoEl.end(),trigger);
      if (pos != labelsTopoEl.end()) {
        ATH_MSG_DEBUG("Found one CTP; ,CTPId: " << item.ctpId() << " ,Name: " << item.name() << " ,Definition: " << definition);
        unsigned index = std::distance(labelsTopoEl.begin(),pos);
        ctpIds[index]=item.ctpId();
      }
    }
  }

  return ctpIds;
}

void L1TopoOnlineMonitor::resetFlags(xAOD::TrigComposite& errorFlags) const {
  errorFlags.setDetail("hasTrivialFlag", true);
}
