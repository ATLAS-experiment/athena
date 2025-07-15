/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// Local includes
#include "MuonRoIByteStreamTool.h"

// Trigger includes
#include "TrigConfData/L1Menu.h"
#include "TrigT1MuctpiBits/HelpersPhase1.h"
#include "xAODTrigger/MuonRoI.h"
#include "xAODTrigger/MuonRoIAuxContainer.h"
// Athena includes
#include "PathResolver/PathResolver.h"

// TDAQ includes
#include "eformat/SourceIdentifier.h"

// get bitsmasks from common definition source:
#include "TrigT1MuctpiBits/MuCTPI_Bits.h"

#include <span>

using namespace LVL1::MuCTPIBits;

using ROBF = OFFLINE_FRAGMENTS_NAMESPACE::ROBFragment;
using WROBF = OFFLINE_FRAGMENTS_NAMESPACE_WRITE::ROBFragment;

// -----------------------------------------------------------------------------
// Helpers
// -----------------------------------------------------------------------------
namespace {
  /// x-axis values for word type count histogram
  constexpr static std::array<size_t,static_cast<size_t>(LVL1::MuCTPIBits::WordType::MAX)> s_wordTypes = {0,1,2,3,4,5};
  using namespace std::literals::string_view_literals;
  /// Labels to build sector-specific eta/phi monitored variable name
  constexpr static std::array<std::string_view,4> s_sectorNames = {{"Undefined"sv, "Barrel"sv, "Forward"sv, "Endcap"sv}};
  /// Number of BCIDs in full LHC orbit
  constexpr static int s_bcidsFullOrbit{3564};
  /// BCID diff wrapped into the range [-s_bcidsFullOrbit/2, s_bcidsFullOrbit/2)
  constexpr int bcidDiff(int a, int b) {
    int diff = a - b;
    while (diff < -s_bcidsFullOrbit/2) {diff += s_bcidsFullOrbit;}
    while (diff > s_bcidsFullOrbit/2 - 1) {diff -= s_bcidsFullOrbit;}
    return diff;
  }
  /// BCID sum wrapped into the range [0, s_bcidsFullOrbit)
  constexpr int bcidSum(int a, int b) {
    int sum = a + b;
    while (sum < 0) {sum += s_bcidsFullOrbit;}
    while (sum >= s_bcidsFullOrbit) {sum -= s_bcidsFullOrbit;}
    return sum;
  }
  /// Add a special bit flagging Run-3 format for offline use in xAOD::MuonRoI
  constexpr uint32_t roiWordAddOfflineRun3Flag (uint32_t candidateWord) {
    return (candidateWord | 0x1u<<31);
  }
  /// Remove a special bit flagging Run-3 format for offline use in xAOD::MuonRoI
  constexpr uint32_t roiWordRemoveOfflineRun3Flag (uint32_t candidateWord) {
    return (candidateWord & ~(0x1u<<31));
  }
}

// -----------------------------------------------------------------------------
// Constructor
// -----------------------------------------------------------------------------
MuonRoIByteStreamTool::MuonRoIByteStreamTool(const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent)
: base_class(type, name, parent) {}

// -----------------------------------------------------------------------------
// Initialisation
// -----------------------------------------------------------------------------
StatusCode MuonRoIByteStreamTool::initialize() {
  ConversionMode mode = getConversionMode(m_roiReadKeys, m_roiWriteKeys, msg());
  ATH_CHECK(mode!=ConversionMode::Undefined);
  ATH_CHECK(m_roiWriteKeys.initialize(mode==ConversionMode::Decoding));
  ATH_CHECK(m_roiReadKeys.initialize(mode==ConversionMode::Encoding));
  ATH_CHECK(m_MuCTPIL1TopoKeys.initialize(m_doTopo.value() && mode==ConversionMode::Decoding));

  ATH_MSG_DEBUG((mode==ConversionMode::Encoding ? "Encoding" : "Decoding") << " ROB IDs: "
                << MSG::hex << m_robIds.value() << MSG::dec);

  if (m_doTopo.value() && mode==ConversionMode::Decoding && m_MuCTPIL1TopoKeys.size() != m_roiWriteKeys.size()) {
    ATH_MSG_ERROR("Number of Topo TOB output containers (" << m_MuCTPIL1TopoKeys.size() << ") "
                  << "does not match the number of RoI output containers (" << m_roiWriteKeys.size() << ")");
    return StatusCode::FAILURE;
  }

  m_readoutWindow = mode==ConversionMode::Decoding ? m_roiWriteKeys.size() : m_roiReadKeys.size();
  if (m_readoutWindow!=1 && m_readoutWindow!=3 && m_readoutWindow!=5) {
    ATH_MSG_ERROR("The expected readout window must be 1, 3 or 5, but it is " << m_readoutWindow);
    return StatusCode::FAILURE;
  }

  if (m_robIds.value().size() != 1) {
    ATH_MSG_ERROR("This tool implementation assumes there is exactly one MUCTPI ROB, but "
                  << m_robIds.size() << " were configured");
    return StatusCode::SUCCESS;
  }

  CHECK( m_rpcTool.retrieve() );
  CHECK( m_tgcTool.retrieve() );
  CHECK( m_thresholdTool.retrieve() );
  if (!m_monTool.empty()) ATH_CHECK(m_monTool.retrieve());

  if (m_doTopo.value()) {
    const std::string barrelFileName   = PathResolverFindCalibFile( m_barrelRoIFile );
    const std::string ecfFileName      = PathResolverFindCalibFile( m_ecfRoIFile );
    const std::string side0LUTFileName = PathResolverFindCalibFile( m_side0LUTFile );
    const std::string side1LUTFileName = PathResolverFindCalibFile( m_side1LUTFile );
    
    //CHECK( m_l1topoLUT.initializeBarrelLUT(side0LUTFileName,
		//			   side1LUTFileName) );
    CHECK( m_l1topoLUT.initializeLUT(barrelFileName,
				     ecfFileName,
				     side0LUTFileName,
				     side1LUTFileName) );
  }
  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------
// BS->xAOD conversion
// -----------------------------------------------------------------------------
StatusCode MuonRoIByteStreamTool::convertFromBS(const std::vector<const ROBF*>& vrobf,
                                                const EventContext& eventContext) const {

if(m_writeDecodedMuonRoIs){
  // Create and record the RoI containers
  std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>> roiHandles = m_roiWriteKeys.makeHandles(eventContext);
  for (auto& roiHandle : roiHandles) {
    ATH_CHECK(roiHandle.record(std::make_unique<xAOD::MuonRoIContainer>(),
                               std::make_unique<xAOD::MuonRoIAuxContainer>()));
    ATH_MSG_DEBUG("Recorded MuonRoIContainer with key " << roiHandle.key());
  }

  // Create a WriteHandle for L1Topo output
  std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>> topoHandles;
  if (m_doTopo.value()) {
    topoHandles = m_MuCTPIL1TopoKeys.makeHandles(eventContext);
    for (auto& topoHandle : topoHandles) {
      ATH_CHECK(topoHandle.record(std::make_unique<xAOD::MuonRoIContainer>(),
                               std::make_unique<xAOD::MuonRoIAuxContainer>()));
      ATH_MSG_DEBUG("Recorded MuCTPIL1Topo with key " << topoHandle.key());
    }
  }

  // Find the ROB fragment to decode
  const eformat::helper::SourceIdentifier sid(m_robIds.value().at(0));
  auto it = std::find_if(vrobf.begin(), vrobf.end(), [&sid](const ROBF* rob){return rob->rob_source_id() == sid.code();});
  if (it == vrobf.end()) {
    ATH_MSG_DEBUG("No MUCTPI ROB fragment with ID 0x" << std::hex << sid.code() << std::dec
                  << " was found, MuonRoIContainer will be empty");
    return StatusCode::SUCCESS;
  }

  // Retrieve the ROD data
  const ROBF* rob = *it;
  ATH_MSG_DEBUG("MUCTPI ROB for BCID " << rob->rod_bc_id());
  const uint32_t ndata = rob->rod_ndata();
  const uint32_t* const data = rob->rod_data();

  // Initialise monitoring variables
  Monitored::Scalar<uint32_t> monNumWords{"NumWordsInROD", ndata};
  std::array<size_t,static_cast<size_t>(LVL1::MuCTPIBits::WordType::MAX)> wordTypeCounts{}; // zero-initialised
  auto monWordTypeCount = Monitored::Collection("WordTypeCount", wordTypeCounts);
  auto monWordType = Monitored::Collection("WordType", s_wordTypes);
  std::vector<int> bcidOffsetsWrtROB; // diffs between BCID in timeslice header and BCID in ROB header
  auto monBCIDOffsetsWrtROB = Monitored::Collection("BCIDOffsetsWrtROB", bcidOffsetsWrtROB);

  // Check for empty data
  if (ndata==0) {
    ATH_MSG_ERROR("Empty ROD data in MUCTPI ROB 0x" << std::hex << sid.code() << std::dec);
    Monitored::Group(m_monTool, monNumWords);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Starting to decode " << ndata << " ROD words");

  // We don't assume the window size at this point. Instead, we collect the start and size of candidate list for
  // each time slice and decode them later directly into the right time slice output container.
  std::vector<std::pair<size_t,size_t>> roiSlices; // v of {start, length}
  std::vector<std::pair<size_t,size_t>> topoSlices; // v of {start, length}

  // Iterate over ROD words and decode
  size_t iWord{0};
  for (const uint32_t word : std::span{data, ndata}) {
    ATH_MSG_DEBUG("MUCTPI raw word " << iWord << ": 0x" << std::hex << word << std::dec);
    LVL1::MuCTPIBits::WordType wordType = LVL1::MuCTPIBits::getWordType(word);
    ++wordTypeCounts[static_cast<size_t>(wordType)];

    switch (wordType) {
      case LVL1::MuCTPIBits::WordType::Timeslice: {
        const auto header = LVL1::MuCTPIBits::timesliceHeader(word);
        ATH_MSG_DEBUG("This is a timeslice header word with BCID=" << header.bcid
                      << ", NTOB=" << header.tobCount << ", NCAND=" << header.candCount);
        // create new RoI words slice
        roiSlices.emplace_back(0,0);
        // create new Topo words slice
        topoSlices.emplace_back(0,0);
        // monitor BCID offset
        bcidOffsetsWrtROB.push_back(bcidDiff(header.bcid, rob->rod_bc_id()));
        break;
      }
      case LVL1::MuCTPIBits::WordType::Multiplicity: {
        uint32_t tmNum = LVL1::MuCTPIBits::multiplicityWordNumber(word);
        ATH_MSG_DEBUG("This is a multiplicity word #" << tmNum);
        break;
      }
      case LVL1::MuCTPIBits::WordType::Candidate: {
        ATH_MSG_DEBUG("This is a RoI candidate word");
        if (roiSlices.empty()) {
          ATH_MSG_ERROR("Unexpected data format - found candidate word before any timeslice header");
          Monitored::Group(m_monTool, monNumWords, monWordType, monWordTypeCount, monBCIDOffsetsWrtROB);
          return StatusCode::FAILURE;
        }
        // advance slice edges
        std::pair<size_t,size_t>& slice = roiSlices.back();
        if (slice.first==0) slice.first = iWord;
        slice.second = iWord - slice.first + 1;
        break;
      }
      case LVL1::MuCTPIBits::WordType::Topo: {
        ATH_MSG_DEBUG("This is a Topo TOB word");
        if (not m_doTopo.value()) {break;}
        if (topoSlices.empty()) {
          ATH_MSG_ERROR("Unexpected data format - found Topo TOB word before any timeslice header");
          return StatusCode::FAILURE;
        }
        // advance slice edges
        std::pair<size_t,size_t>& slice = topoSlices.back();
        if (slice.first==0) slice.first = iWord;
        slice.second = iWord - slice.first + 1;
        break;
      }
      case LVL1::MuCTPIBits::WordType::Status: {
        ATH_MSG_DEBUG("This is a status word");
        std::vector<size_t> errorBits = LVL1::MuCTPIBits::getDataStatusWordErrors(word);
        // TODO: Decide on the action in case of errors, ATR-25069
        if (!errorBits.empty()) {
          ATH_MSG_DEBUG("MUCTPI ROD data flagged with errors. The data status word is 0x" << std::hex << word << std::dec);
          for (size_t bit : errorBits) {
            ATH_MSG_DEBUG("Error bit " << bit << ": " << LVL1::MuCTPIBits::DataStatusWordErrors.at(bit));
          }
          auto monErrorBits = Monitored::Collection("DataStatusWordErrors", errorBits);
          Monitored::Group(m_monTool, monErrorBits);
        }
        break;
      }
      default: {
        ATH_MSG_ERROR("The MUCTPI word 0x" << std::hex << word << std::dec << " does not match any known word type");
        Monitored::Group(m_monTool, monNumWords, monWordType, monWordTypeCount, monBCIDOffsetsWrtROB);
        return StatusCode::FAILURE;
      }
    }
    ++iWord;
  } // Loop over all ROD words

  // Fill data format monitoring histograms
  Monitored::Group(m_monTool, monNumWords, monWordType, monWordTypeCount, monBCIDOffsetsWrtROB);

  // Validate the number of slices and decode the RoI candidate words in each time slice
  const size_t nSlices{roiSlices.size()};
  const size_t nOutputSlices{static_cast<size_t>(m_readoutWindow)};
  if (nSlices > nOutputSlices) {
    ATH_MSG_ERROR("Found " << nSlices << " time slices, but only " << m_readoutWindow << " outputs are configured");
    return StatusCode::FAILURE;
  } else if (nSlices != static_cast<size_t>(rob->rod_detev_type())) {
    ATH_MSG_ERROR("Found " << nSlices << " time slices, but Detector Event Type word indicates there should be "
                  << rob->rod_detev_type());
    return StatusCode::FAILURE;
  } else if (nSlices!=1 && nSlices!=3 && nSlices!=5) {
    ATH_MSG_ERROR("Expected 1, 3 or 5 time slices but found " << nSlices);
    return StatusCode::FAILURE;
  }
  const size_t outputOffset = nOutputSlices/2 - nSlices/2;
  ATH_CHECK(decodeRoiSlices(data, roiSlices, roiHandles, outputOffset, eventContext));

  // Validate the number of slices and decode the Topo TOB words in each time slice
  if (m_doTopo.value()) {
    const size_t nTopoSlices{topoSlices.size()};
    const size_t nTopoOutputSlices{static_cast<size_t>(m_readoutWindow)};
    if (nTopoSlices > nTopoOutputSlices) {
      ATH_MSG_ERROR("Found " << nTopoSlices << " TOPO TOB time slices, but only " << m_readoutWindow << " outputs are configured");
      return StatusCode::FAILURE;
    } else if (nTopoSlices != static_cast<size_t>(rob->rod_detev_type())) {
      ATH_MSG_ERROR("Found " << nTopoSlices << " time slices, but Detector Event Type word indicates there should be "
                    << rob->rod_detev_type());
      return StatusCode::FAILURE;
    } else if (nTopoSlices!=1 && nTopoSlices!=3 && nTopoSlices!=5) {
      ATH_MSG_ERROR("Expected 1, 3 or 5 time slices but found " << nTopoSlices);
      return StatusCode::FAILURE;
    }
    const size_t topoOutputOffset = nTopoOutputSlices/2 - nTopoSlices/2;
    ATH_CHECK(decodeTopoSlices(data, topoSlices, topoHandles, topoOutputOffset, eventContext));
  }

  // Output monitoring
  short bcOffset{static_cast<short>(5/2 - m_readoutWindow/2 - 2)};
  auto topoHandleIt = topoHandles.begin();
  for (auto& roiHandle : roiHandles) {
    auto& topoHandle = *topoHandleIt;
    Monitored::Scalar<short> monBCOffset{"BCOffset", bcOffset};
    Monitored::Scalar<size_t> monNumRoIs{"NumOutputRoIs", roiHandle->size()};
    if (m_doTopo.value()) {
      Monitored::Scalar<size_t> monNumTopo{"NumOutputTopoTOBs", topoHandle->size()};
      Monitored::Scalar<int> monNumDiff{"NumOutputDiffRoITopo", static_cast<int>(monNumRoIs)-static_cast<int>(monNumTopo)};
      ATH_MSG_DEBUG("Decoded " << monNumRoIs << " RoIs into the " << roiHandle.key() << " container "
                    "and " << monNumTopo << " Topo TOBs into the " << topoHandle.key() << " container");
      Monitored::Group(m_monTool, monBCOffset, monNumRoIs, monNumTopo, monNumDiff);
      ++topoHandleIt;
    } else {
      ATH_MSG_DEBUG("Decoded " << monNumRoIs << " RoIs into the " << roiHandle.key() << " container");
      Monitored::Group(m_monTool, monBCOffset, monNumRoIs);
    }
    ++bcOffset;
  }

  return StatusCode::SUCCESS;
 }

 else{ //Just decode the MUCTPI
 
  // Create a WriteHandle for L1Topo output
  std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>> topoHandles;
  if (m_doTopo.value()) {
    topoHandles = m_MuCTPIL1TopoKeys.makeHandles(eventContext);
    for (auto& topoHandle : topoHandles) {
      ATH_CHECK(topoHandle.record(std::make_unique<xAOD::MuonRoIContainer>(),
                               std::make_unique<xAOD::MuonRoIAuxContainer>()));
      ATH_MSG_DEBUG("Recorded MuCTPIL1Topo with key " << topoHandle.key());
    }
  }

  // Find the ROB fragment to decode
  const eformat::helper::SourceIdentifier sid(m_robIds.value().at(0));
  auto it = std::find_if(vrobf.begin(), vrobf.end(), [&sid](const ROBF* rob){return rob->rob_source_id() == sid.code();});
  if (it == vrobf.end()) {
    ATH_MSG_DEBUG("No MUCTPI ROB fragment with ID 0x" << std::hex << sid.code() << std::dec
                  << " was found, MuonRoIContainer will be empty");
    return StatusCode::SUCCESS;
  }

  // Retrieve the ROD data
  const ROBF* rob = *it;
  ATH_MSG_DEBUG("MUCTPI ROB for BCID " << rob->rod_bc_id());
  const uint32_t ndata = rob->rod_ndata();
  const uint32_t* const data = rob->rod_data();

  // Initialise monitoring variables
  Monitored::Scalar<uint32_t> monNumWords{"NumWordsInROD", ndata};
  std::array<size_t,static_cast<size_t>(LVL1::MuCTPIBits::WordType::MAX)> wordTypeCounts{}; // zero-initialised
  auto monWordTypeCount = Monitored::Collection("WordTypeCount", wordTypeCounts);
  auto monWordType = Monitored::Collection("WordType", s_wordTypes);
  std::vector<int> bcidOffsetsWrtROB; // diffs between BCID in timeslice header and BCID in ROB header
  auto monBCIDOffsetsWrtROB = Monitored::Collection("BCIDOffsetsWrtROB", bcidOffsetsWrtROB);

  // Check for empty data
  if (ndata==0) {
    ATH_MSG_ERROR("Empty ROD data in MUCTPI ROB 0x" << std::hex << sid.code() << std::dec);
    return StatusCode::FAILURE;
  }
  ATH_MSG_DEBUG("Starting to decode " << ndata << " ROD words");

  // We don't assume the window size at this point. Instead, we collect the start and size of candidate list for
  // each time slice and decode them later directly into the right time slice output container.
  std::vector<std::pair<size_t,size_t>> roiSlices; // v of {start, length}
  std::vector<std::pair<size_t,size_t>> topoSlices; // v of {start, length}

  // Iterate over ROD words and decode
  size_t iWord{0};
  for (const uint32_t word : CxxUtils::span{data, ndata}) {
    ATH_MSG_DEBUG("MUCTPI raw word " << iWord << ": 0x" << std::hex << word << std::dec);
    LVL1::MuCTPIBits::WordType wordType = LVL1::MuCTPIBits::getWordType(word);
    ++wordTypeCounts[static_cast<size_t>(wordType)];

    switch (wordType) {
      case LVL1::MuCTPIBits::WordType::Timeslice: {
        const auto header = LVL1::MuCTPIBits::timesliceHeader(word);
        ATH_MSG_DEBUG("This is a timeslice header word with BCID=" << header.bcid
                      << ", NTOB=" << header.tobCount << ", NCAND=" << header.candCount);
        // create new RoI words slice
        roiSlices.emplace_back(0,0);
        // create new Topo words slice
        topoSlices.emplace_back(0,0);
        // monitor BCID offset
        bcidOffsetsWrtROB.push_back(bcidDiff(header.bcid, rob->rod_bc_id()));
        break;
      }
      case LVL1::MuCTPIBits::WordType::Multiplicity: {
        uint32_t tmNum = LVL1::MuCTPIBits::multiplicityWordNumber(word);
        ATH_MSG_DEBUG("This is a multiplicity word #" << tmNum);
        break;
      }
      case LVL1::MuCTPIBits::WordType::Candidate: {
        ATH_MSG_DEBUG("This is a RoI candidate word");
        if (roiSlices.empty()) {
          ATH_MSG_ERROR("Unexpected data format - found candidate word before any timeslice header");
          return StatusCode::FAILURE;
        }
        // advance slice edges
        std::pair<size_t,size_t>& slice = roiSlices.back();
        if (slice.first==0) slice.first = iWord;
        slice.second = iWord - slice.first + 1;
        break;
      }
      case LVL1::MuCTPIBits::WordType::Topo: {
        ATH_MSG_DEBUG("This is a Topo TOB word");
        if (not m_doTopo.value()) {break;}
        if (topoSlices.empty()) {
          ATH_MSG_ERROR("Unexpected data format - found Topo TOB word before any timeslice header");
          return StatusCode::FAILURE;
        }
        // advance slice edges
        std::pair<size_t,size_t>& slice = topoSlices.back();
        if (slice.first==0) slice.first = iWord;
        slice.second = iWord - slice.first + 1;
        break;
      }
      case LVL1::MuCTPIBits::WordType::Status: {
        ATH_MSG_DEBUG("This is a status word");
        std::vector<size_t> errorBits = LVL1::MuCTPIBits::getDataStatusWordErrors(word);
        // TODO: Decide on the action in case of errors, ATR-25069
        if (!errorBits.empty()) {
          ATH_MSG_DEBUG("MUCTPI ROD data flagged with errors. The data status word is 0x" << std::hex << word << std::dec);
          for (size_t bit : errorBits) {
            ATH_MSG_DEBUG("Error bit " << bit << ": " << LVL1::MuCTPIBits::DataStatusWordErrors.at(bit));
          }
          auto monErrorBits = Monitored::Collection("DataStatusWordErrors", errorBits);
        }
        break;
      }
      default: {
        ATH_MSG_ERROR("The MUCTPI word 0x" << std::hex << word << std::dec << " does not match any known word type");
        return StatusCode::FAILURE;
      }
    }
    ++iWord;
  } // Loop over all ROD words

  // Validate the number of slices and decode the RoI candidate words in each time slice
  const size_t nSlices{roiSlices.size()};
  const size_t nOutputSlices{static_cast<size_t>(m_readoutWindow)};
  if (nSlices > nOutputSlices) {
    ATH_MSG_ERROR("Found " << nSlices << " time slices, but only " << m_readoutWindow << " outputs are configured");
    return StatusCode::FAILURE;
  } else if (nSlices != static_cast<size_t>(rob->rod_detev_type())) {
    ATH_MSG_ERROR("Found " << nSlices << " time slices, but Detector Event Type word indicates there should be "
                  << rob->rod_detev_type());
    return StatusCode::FAILURE;
  } else if (nSlices!=1 && nSlices!=3 && nSlices!=5) {
    ATH_MSG_ERROR("Expected 1, 3 or 5 time slices but found " << nSlices);
    return StatusCode::FAILURE;
  }

  // Validate the number of slices and decode the Topo TOB words in each time slice
  if (m_doTopo.value()) {
    const size_t nTopoSlices{topoSlices.size()};
    const size_t nTopoOutputSlices{static_cast<size_t>(m_readoutWindow)};
    if (nTopoSlices > nTopoOutputSlices) {
      ATH_MSG_ERROR("Found " << nTopoSlices << " TOPO TOB time slices, but only " << m_readoutWindow << " outputs are configured");
      return StatusCode::FAILURE;
    } else if (nTopoSlices != static_cast<size_t>(rob->rod_detev_type())) {
      ATH_MSG_ERROR("Found " << nTopoSlices << " time slices, but Detector Event Type word indicates there should be "
                    << rob->rod_detev_type());
      return StatusCode::FAILURE;
    } else if (nTopoSlices!=1 && nTopoSlices!=3 && nTopoSlices!=5) {
      ATH_MSG_ERROR("Expected 1, 3 or 5 time slices but found " << nTopoSlices);
      return StatusCode::FAILURE;
    }
    const size_t topoOutputOffset = nTopoOutputSlices/2 - nTopoSlices/2;
    ATH_CHECK(decodeTopoSlices(data, topoSlices, topoHandles, topoOutputOffset, eventContext));
  }

  return StatusCode::SUCCESS;
 }

}

// -----------------------------------------------------------------------------
// xAOD->BS conversion
// -----------------------------------------------------------------------------
StatusCode MuonRoIByteStreamTool::convertToBS(std::vector<WROBF*>& vrobf,
                                              const EventContext& eventContext) {
  // Retrieve the RoI containers and determine how many time slices will be encoded
  std::vector<SG::ReadHandle<xAOD::MuonRoIContainer>> handles = m_roiReadKeys.makeHandles(eventContext);
  int nSlices{0};
  int iHandle{0};
  size_t rodSize{0};
  for (auto& handle : handles) {
    ATH_CHECK(handle.isValid());
    if (!handle->empty()) {
      rodSize += handle->size();
      nSlices = std::max(nSlices, 1+2*std::abs(m_readoutWindow/2 - iHandle));
    }
    ++iHandle;
  }
  if (nSlices==0) {
    ATH_MSG_DEBUG("There are no muon RoIs to encode in this event");
    // Force at least the triggered BC slice to be encoded
    nSlices = 1;
  }

  // Calculate the number of words we need to allocate (candidate words already counted above)
  rodSize += 4*nSlices + 1; // 1 timeslice header per slice, 3 multiplicity words per slice, 1 status word
  ATH_MSG_DEBUG("Going to encode " << nSlices << " time slices into " << rodSize << " ROD words");

  // Clear BS data cache and allocate new data array
  clearCache(eventContext);
  uint32_t* data = newRodData(eventContext, rodSize); // Owned by the cache

  // Initialise a few monitoring variables
  Monitored::Scalar<size_t> monNumWords{"NumWordsInROD", rodSize};
  std::array<size_t,static_cast<size_t>(LVL1::MuCTPIBits::WordType::MAX)> wordTypeCounts{}; // zero-initialised
  auto monWordTypeCount = Monitored::Collection("WordTypeCount", wordTypeCounts);
  auto monWordType = Monitored::Collection("WordType", s_wordTypes);
  std::vector<int> bcidOffsetsWrtROB; // diffs between BCID in timeslice header and BCID in ROB header
  auto monBCIDOffsetsWrtROB = Monitored::Collection("BCIDOffsetsWrtROB", bcidOffsetsWrtROB);
  auto monitorCandidate = [](const auto& monTool, const xAOD::MuonRoI& roi){
      // Fill per-candidate monitoring histograms
      const uint32_t word = roi.roiWord();
      using SubsysID_ut = std::underlying_type_t<LVL1::MuCTPIBits::SubsysID>;
      const LVL1::MuCTPIBits::SubsysID subsysID = LVL1::MuCTPIBits::getSubsysID(word);
      Monitored::Scalar<SubsysID_ut> monSubsysID{"SubsysID", static_cast<SubsysID_ut>(subsysID)};
      std::string sectorName{s_sectorNames[static_cast<size_t>(subsysID)]};
      Monitored::Scalar<double> monEta{"roiEta_"+sectorName, roi.eta()};
      Monitored::Scalar<double> monPhi{"roiPhi_"+sectorName, roi.phi()};
      Monitored::Group(monTool, monSubsysID, monEta, monPhi);
  };

  // Fill the data words
  auto inputIt = handles.begin();
  std::advance(inputIt, m_readoutWindow/2 - nSlices/2);
  size_t iWord{0};
  for (int iSlice=0; iSlice<nSlices; ++iSlice, ++inputIt) {
    // Timeslice header
    uint32_t bcid = bcidSum(eventContext.eventID().bunch_crossing_id(), iSlice - nSlices/2);
    static constexpr uint32_t tobCount = 0; // Filling Topo words not implemented
    uint32_t candCount = (*inputIt)->size();
    data[iWord++] = LVL1::MuCTPIBits::timesliceHeader(bcid, tobCount, candCount);
    ++wordTypeCounts[static_cast<size_t>(LVL1::MuCTPIBits::WordType::Timeslice)];
    bcidOffsetsWrtROB.push_back(bcidDiff(bcid, eventContext.eventID().bunch_crossing_id()));
    ATH_MSG_DEBUG("Added timeslice header word with BCID=" << bcid << ", NTOB=" << tobCount << ", NCAND=" << candCount);

    // Multiplicity words
    std::array<uint32_t,3> multiplicityWords = LVL1::MuCTPIBits::multiplicityWords(0, 0, false); // Multiplicity words content not implemented
    for (const uint32_t word : multiplicityWords) {
      data[iWord++] = word;
    }
    wordTypeCounts[static_cast<size_t>(LVL1::MuCTPIBits::WordType::Multiplicity)] += multiplicityWords.size();
    ATH_MSG_DEBUG("Added " << multiplicityWords.size() << " multiplicity words");

    // Candidate words
    for (const xAOD::MuonRoI* roi : **inputIt) {
      monitorCandidate(m_monTool, *roi);
      data[iWord++] = roiWordRemoveOfflineRun3Flag(roi->roiWord());
      ATH_MSG_DEBUG("Added RoI word 0x" << std::hex << roi->roiWord() << std::dec);
    }
    wordTypeCounts[static_cast<size_t>(LVL1::MuCTPIBits::WordType::Candidate)] += (*inputIt)->size();
    ATH_MSG_DEBUG("Added " << (*inputIt)->size() << " candidate words");
  }

  // Status word
  data[iWord++] = LVL1::MuCTPIBits::dataStatusWord(0);
  ++wordTypeCounts[static_cast<size_t>(LVL1::MuCTPIBits::WordType::Status)];
  ATH_MSG_DEBUG("Added the data status word");

  // Fill data format monitoring histograms
  Monitored::Group(m_monTool, monNumWords, monWordType, monWordTypeCount, monBCIDOffsetsWrtROB);

  // Check that we filled all words
  if (iWord!=rodSize) {
    ATH_MSG_ERROR("Expected to fill " << rodSize << " ROD words but filled " << iWord);
    return StatusCode::FAILURE;
  }

  // Create a ROBFragment containing the ROD words
  const eformat::helper::SourceIdentifier sid(m_robIds.value().at(0));
  vrobf.push_back(newRobFragment(eventContext, sid.code(), rodSize, data, nSlices));

  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------
// Helper for BS->xAOD conversion of RoI candidate words
// -----------------------------------------------------------------------------
StatusCode MuonRoIByteStreamTool::decodeRoiSlices(const uint32_t* data,
                                                  const std::vector<std::pair<size_t,size_t>>& slices,
                                                  std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>>& handles,
                                                  size_t outputOffset,
                                                  const EventContext& eventContext) const {
  auto outputIt = handles.begin();
  std::advance(outputIt, outputOffset);
  for (const auto& [sliceStart,sliceSize] : slices) {
    for (const uint32_t word : std::span{data+sliceStart, sliceSize}) {
      ATH_MSG_DEBUG("Decoding RoI word 0x" << std::hex << word << std::dec << " into the " << outputIt->key() << " container");

      // Create a new xAOD::MuonRoI object for this candidate in the output container
      (*outputIt)->push_back(std::make_unique<xAOD::MuonRoI>());

      // Decode eta/phi information using the right tool for the subsystem
      LVL1::TrigT1MuonRecRoiData roiData;
      const LVL1::MuCTPIBits::SubsysID subsysID = LVL1::MuCTPIBits::getSubsysID(word);
      switch (subsysID) {
        case LVL1::MuCTPIBits::SubsysID::Endcap: // same for Endcap and Forward
        case LVL1::MuCTPIBits::SubsysID::Forward: {
          ATH_MSG_DEBUG("This is an Endcap/Forward candidate, calling the " << m_tgcTool.typeAndName());
          ATH_CHECK( m_tgcTool->roiData(word,roiData) );
          break;
        }
        case LVL1::MuCTPIBits::SubsysID::Barrel: {
          ATH_MSG_DEBUG("This is a Barrel candidate, calling the " << m_rpcTool.typeAndName());
	  ATH_CHECK( m_rpcTool->roiData(word,roiData) );
          break;
        }
        default: {
          ATH_MSG_ERROR("Failed to determine Sector ID from RoI word 0x" << std::hex << word << std::dec);
          return StatusCode::FAILURE;
        }
      }

      // Get the threshold decisions to find the lowest pt threshold passed
      // This is required by xAOD::MuonRoI::initialize() but not used for HLT seeding (a threshold pattern bit mask is used instead)
      const std::pair<std::string, double> minThrInfo = m_thresholdTool->getMinThresholdNameAndValue(
        m_thresholdTool->getThresholdDecisions(word, eventContext),
        roiData.eta());

      // Fill the xAOD::MuonRoI object
      (*outputIt)->back()->initialize(roiWordAddOfflineRun3Flag(word),
                                      roiData.eta(),
                                      roiData.phi(),
                                      minThrInfo.first,
                                      minThrInfo.second);

      // Fill per-candidate monitoring histograms
      using SubsysID_ut = std::underlying_type_t<LVL1::MuCTPIBits::SubsysID>;
      Monitored::Scalar<SubsysID_ut> monSubsysID{"SubsysID", static_cast<SubsysID_ut>(subsysID)};
      std::string sectorName{s_sectorNames[static_cast<size_t>(subsysID)]};
      Monitored::Scalar<double> monEta{"roiEta_"+sectorName, roiData.eta()};
      Monitored::Scalar<double> monPhi{"roiPhi_"+sectorName, roiData.phi()};
      Monitored::Group(m_monTool, monSubsysID, monEta, monPhi);
    }
    ++outputIt;
  } // Loop over RoI candidate time slices

  return StatusCode::SUCCESS;
}

// -----------------------------------------------------------------------------
// Helper for BS->transient conversion of Topo TOB words
// -----------------------------------------------------------------------------
StatusCode MuonRoIByteStreamTool::decodeTopoSlices(const uint32_t* data,
                                                   const std::vector<std::pair<size_t,size_t>>& slices,
                                                   std::vector<SG::WriteHandle<xAOD::MuonRoIContainer>>& handles,
                                                   size_t outputOffset,
                                                   const EventContext& /*eventContext*/) const {
  int toposliceiterator = -1;
  int nomBCID_slice = slices.size() / 2 ;
  int topobcidOffset = 0;
  unsigned short subsystem = 0;

  float eta=0, phi=0;
  unsigned int et=0;
  //in case something is found to not be correctly decoded by the L1Topo group - can clean this extra-debug printouts later if wished
  constexpr static bool local_topo_debug{true};

  const TrigConf::L1Menu * l1menu = nullptr;
  ATH_CHECK( detStore()->retrieve(l1menu) );
 
  const auto & exMU = l1menu->thrExtraInfo().MU();
  auto tgcPtValues = exMU.knownTgcPtValues();

  auto outputIt = handles.begin();
  std::advance(outputIt, outputOffset);
  // Loop over Topo candidate time slices
  for (const auto& [sliceStart,sliceSize] : slices) {
    toposliceiterator++;
    for (const uint32_t word : std::span{data+sliceStart, sliceSize}) {
      //the cand usage should be optimised!
      std::stringstream sectorName;
      subsystem = 0;

      topobcidOffset = toposliceiterator - nomBCID_slice;
      
      // Create a new xAOD::MuonRoI object for this candidate in the output container
      (*outputIt)->push_back(std::make_unique<xAOD::MuonRoI>());

      ATH_MSG_DEBUG("MuCTPIL1Topo: Decoding Topo word 0x" << std::hex << word << std::dec << " into the " << outputIt->key() << " container");

      // NOTE the convention:
      // HEMISPHERE 0: C-side (-) / 1: A-side (+)
      // Det (bits): Barrel: 00 - EC: 1X - FW: 01
      const auto topoheader = LVL1::MuCTPIBits::topoHeader(word);
      if (local_topo_debug) {
        ATH_MSG_DEBUG("MuCTPIL1Topo: TOPOSLICE data: " <<data << " sliceStart: "<<sliceStart<< " sliceSize: " <<sliceSize );
        ATH_MSG_DEBUG("MuCTPIL1Topo word: 0x"     << std::hex << word << std::dec  );
        ATH_MSG_DEBUG("MuCTPIL1Topo word: 0b"     << std::bitset<32>(word) );
      }
      // Build the sector name
      // topoheader.det is the direct WORD content, but here later in m_l1topoLUT.getCoordinates the definition is different ... see the subsystem settings below
      if (topoheader.det == 0) {
        sectorName<<"B";
        subsystem = 0;
      }
      else if (topoheader.det == 1) {
        sectorName<<"F";
        subsystem = 2;
      }
      else if (topoheader.det == 2) {
        sectorName<<"E";
        subsystem = 1;
      }
      if (topoheader.hemi) sectorName << "A";
      else sectorName<< "C";
      sectorName << topoheader.sec;
      // End of: Build the sector name

      if (local_topo_debug) {
        ATH_MSG_DEBUG("MuCTPIL1Topo det:     " << topoheader.det);
        ATH_MSG_DEBUG("MuCTPIL1Topo hemi:    " << topoheader.hemi );
        ATH_MSG_DEBUG("MuCTPIL1Topo sector:  " << sectorName.str() );
        ATH_MSG_DEBUG("MuCTPIL1Topo etacode: " << topoheader.etacode );
        ATH_MSG_DEBUG("MuCTPIL1Topo phicode: " << topoheader.phicode );
        ATH_MSG_DEBUG("MuCTPIL1Topo sec:     " << topoheader.sec );
        ATH_MSG_DEBUG("MuCTPIL1Topo roi:     " << topoheader.roi );
        ATH_MSG_DEBUG("MuCTPIL1Topo pt:      " << topoheader.pt );
      }

      if (subsystem == 0) // Barrel
        {
          //for barrel topoheader.roi is always 0, so we need to reconstruct it...
          unsigned short roi = m_l1topoLUT.getBarrelROI(topoheader.hemi, topoheader.sec, topoheader.barrel_eta_lookup, topoheader.barrel_phi_lookup);
          LVL1MUCTPIPHASE1::L1TopoCoordinates coord = m_l1topoLUT.getCoordinates(topoheader.hemi ,subsystem ,topoheader.sec ,roi);
          if (local_topo_debug) {
            ATH_MSG_DEBUG("MuCTPIL1Topo: Barrel decoding");
            ATH_MSG_DEBUG("MuCTPIL1Topo barrel_eta_lookup:     " << topoheader.barrel_eta_lookup );
            ATH_MSG_DEBUG("MuCTPIL1Topo barrel_phi_lookup:     " << topoheader.barrel_phi_lookup );
            ATH_MSG_DEBUG("MuCTPIL1Topo eta value: " <<  coord.eta);
            ATH_MSG_DEBUG("MuCTPIL1Topo phi value: " <<  coord.phi);
          }

          // Documentation / translation for the flag setting below
          if (local_topo_debug) {
            ATH_MSG_DEBUG("MuCTPIL1Topo phiOvl(0): " << topoheader.flag0);
            ATH_MSG_DEBUG("MuCTPIL1Topo is2cand(1):" << topoheader.flag1);
          }
	  
	  // Create RoI Word
	  uint32_t roiWord = 0;
	  roiWord |= (static_cast<uint32_t>(topoheader.pt/2) & RUN3_CAND_PT_MASK) << RUN3_CAND_PT_SHIFT;
	  roiWord |= (static_cast<uint32_t>(topoheader.flag1) & ROI_OVERFLOW_MASK) << RUN3_ROI_OVERFLOW_SHIFT;

	  // Fill the xAOD::MuonRoI object
	  (*outputIt)->back()->initialize(roiWordAddOfflineRun3Flag(roiWord),
					  coord.eta,
					  coord.phi,
					  "",
					  tgcPtValues[topoheader.pt]);
	  et = topoheader.pt;
	  eta = coord.eta;
	  phi = coord.phi;
        } // Barrel
      else { // EC and FWD
        LVL1MUCTPIPHASE1::L1TopoCoordinates coord = m_l1topoLUT.getCoordinates(topoheader.hemi ,subsystem ,topoheader.sec ,topoheader.roi);
        if (local_topo_debug) {
          ATH_MSG_DEBUG("MuCTPIL1Topo: EC / FWD decoding");
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.eta     " <<  coord.eta);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.phi     " <<  coord.phi);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.eta_min " <<  coord.eta_min);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.eta_max " <<  coord.eta_max);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.phi_min " <<  coord.phi_min);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.phi_max " <<  coord.phi_max);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.ieta    " <<  coord.ieta);
          ATH_MSG_DEBUG("MuCTPIL1Topo coord.iphi    " <<  coord.iphi);
	}

        // Documentation / translation for the flag setting below
        if (local_topo_debug) {
	  ATH_MSG_DEBUG("MuCTPIL1Topo charge    (0):" << topoheader.flag0);
	  ATH_MSG_DEBUG("MuCTPIL1Topo bw2or3    (1):" << topoheader.flag1);
	  ATH_MSG_DEBUG("MuCTPIL1Topo innerCoin (2):" << topoheader.flag2);
	  ATH_MSG_DEBUG("MuCTPIL1Topo goodMF    (3):" << topoheader.flag3);
        }
	// Create RoI Word
	uint32_t roiWord = 0;
	roiWord |= (static_cast<uint32_t>(topoheader.pt) & RUN3_CAND_PT_MASK) << RUN3_CAND_PT_SHIFT;
	// Only needed to tell this is TGC
	roiWord |= (FORWARD_ADDRESS_MASK & CAND_SECTOR_ADDRESS_MASK) << RUN3_CAND_SECTOR_ADDRESS_SHIFT;
	if (topoheader.flag0) {roiWord |= (0x1) << RUN3_CAND_TGC_CHARGE_SIGN_SHIFT;}
	if (topoheader.flag1) {roiWord |= (0x1) << RUN3_CAND_TGC_BW2OR3_SHIFT;}
	if (topoheader.flag2) {roiWord |= (0x1) << RUN3_CAND_TGC_INNERCOIN_SHIFT;}
	if (topoheader.flag3) {roiWord |= (0x1) << RUN3_CAND_TGC_GOODMF_SHIFT;}

	// Fill the xAOD::MuonRoI object
	(*outputIt)->back()->initialize(roiWordAddOfflineRun3Flag(roiWord),
					coord.eta,
					coord.phi,
					"",
					tgcPtValues[topoheader.pt]);

	et = topoheader.pt;
	eta = coord.eta;
	phi = coord.phi;
      }// EC and FWD
      
      ATH_MSG_DEBUG("MuCTPIL1Topo: L1Topo output recorded to StoreGate with key " << outputIt->key() << " and bcidOffset: " << topobcidOffset);

      // Fill per-candidate monitoring histograms
      using SubsysID_t = LVL1::MuCTPIBits::SubsysID;
      using SubsysID_ut = std::underlying_type_t<SubsysID_t>;
      SubsysID_t subsysID{SubsysID_t::Undefined};
      switch (subsystem) {
      case 0: {subsysID=SubsysID_t::Barrel; break;}
      case 1: {subsysID=SubsysID_t::Endcap; break;} // Mind the swap in numbering E<->F, see comments above
      case 2: {subsysID=SubsysID_t::Forward; break;}
      default: {break;}
      }
      Monitored::Scalar<SubsysID_ut> monSubsysID{"topoSubsysID", static_cast<SubsysID_ut>(subsysID)};
      std::string subsysName{s_sectorNames[static_cast<size_t>(subsysID)]};
      Monitored::Scalar<float> monEta{"topoEta_"+subsysName, eta};
      Monitored::Scalar<float> monPhi{"topoPhi_"+subsysName, phi};
      Monitored::Scalar<unsigned int> monPtThr{"topoPtThreshold_"+subsysName, et};
      Monitored::Group(m_monTool, monSubsysID, monEta, monPhi, monPtThr);
    }
    ++outputIt;
  }  // Loop over Topo candidate time slices

  return StatusCode::SUCCESS;
}
