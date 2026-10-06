/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODL0MuonCand/versions/NSWCandData_v1.h"
#include "xAODMuonPrepData/versions/AccessorMacros.h"

#include "xAODL0MuonCand/NSWTPBits.h"
#include <iostream>

namespace {
  static const std::string preFixStr{"L0Mu_"};
}

namespace xAOD
{

  // cppcheck-suppress unknownMacro
  IMPLEMENT_SETTER_GETTER( NSWCandData_v1, uint16_t, bcid, setBCID )

  // cppcheck-suppress unknownMacro
  IMPLEMENT_SETTER_GETTER( NSWCandData_v1, uint8_t, nSegments, setNSegments )

  // cppcheck-suppress unknownMacro
  IMPLEMENT_SETTER_GETTER( NSWCandData_v1, uint8_t, fiberId, setFiberId )

  // cppcheck-suppress unknownMacro
  IMPLEMENT_SETTER_GETTER( NSWCandData_v1, uint8_t, boardId, setBoardId )

  bool NSWCandData_v1::overflow() const {
    static const SG::Accessor<uint8_t> acc(preFixStr + "overflow");
    return acc(*this); 
  }

  void NSWCandData_v1::setOverflow(bool value) {
    static const SG::Accessor<uint8_t> acc(preFixStr + "overflow");
    acc(*this) = value; 
  }

  // Get the complete vector of packed 32-bit segment words
  const std::vector<uint32_t>& NSWCandData_v1::segmentWords() const {
    static const SG::Accessor<std::vector<uint32_t>> acc{preFixStr + "segmentWord"};
    return acc(*this);
  }

  // Getting Element-by-Element
  uint16_t NSWCandData_v1::segEtaIndex(size_t i) const { 
    return (segmentWords().at(i) >> L1Muon::NSWTPBits::RUN4_NSWTP_ETA_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_ETA_MASK; 
  }

  uint16_t NSWCandData_v1::segPhiIndex(size_t i) const { 
    return (segmentWords().at(i) >> L1Muon::NSWTPBits::RUN4_NSWTP_PHI_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_PHI_MASK; 
  }

  uint8_t NSWCandData_v1::segDeltaThetaIndex(size_t i) const { 
    return (segmentWords().at(i) >> L1Muon::NSWTPBits::RUN4_NSWTP_DTH_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_DTH_MASK; 
  }

  uint8_t NSWCandData_v1::segQuality(size_t i) const { 
    return (segmentWords().at(i) >> L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_MASK; 
  }

  float NSWCandData_v1::segEta(size_t i) const {
    return L1Muon::NSWTPBits::ETA_MIN + 
      (static_cast<float>(segEtaIndex(i)) / L1Muon::NSWTPBits::ETA_MAX_RAW) * (L1Muon::NSWTPBits::ETA_MAX - L1Muon::NSWTPBits::ETA_MIN);
  }

  float NSWCandData_v1::segPhi(size_t i) const {
    return (static_cast<float>(segPhiIndex(i)) / L1Muon::NSWTPBits::PHI_MAX_RAW) * L1Muon::NSWTPBits::PHI_MAX;
  }

  // Add Segment 
  void NSWCandData_v1::addSegment(uint16_t etaIndex, uint16_t phiIndex, uint8_t deltaThetaIndex, uint8_t quality) {
    static const SG::Accessor<std::vector<uint32_t>> accWord{preFixStr + "segmentWord"};
  
    // Pack the fields into a single 32-bit container word
    uint32_t packedWord = 0;
    packedWord |= (static_cast<uint32_t>(etaIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_ETA_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_ETA_SHIFT;
    packedWord |= (static_cast<uint32_t>(phiIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_PHI_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_PHI_SHIFT;
    packedWord |= (static_cast<uint32_t>(deltaThetaIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_DTH_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_DTH_SHIFT;
    packedWord |= (static_cast<uint32_t>(quality) & L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_SHIFT;

    accWord(*this).push_back(packedWord);
  }

  // Clear segments 
  void NSWCandData_v1::clearSegments() {
    static const SG::Accessor<std::vector<uint32_t>> accWord{preFixStr + "segmentWord"};
    accWord(*this).clear();
  }

  // Debugging 
  std::ostream& operator<<(std::ostream& os, const NSWCandData_v1& obj){
    os << "\n*BEGIN* xAOD::NSWCandData" << std::endl;
    os << "   BCID:                             " << obj.bcid() << std::endl;
    os << "   Board ID:                         " << static_cast<int>(obj.boardId()) << std::endl;
    os << "   Fiber ID:                         " << static_cast<int>(obj.fiberId()) << std::endl;
    os << "   Number of Segments:               " << static_cast<int>(obj.nSegments()) << std::endl;
    os << "   Overflow Flag:                    " << (obj.overflow() ? "TRUE" : "FALSE") << std::endl;
    
    const auto& words = obj.segmentWords();
    os << "   Stored Packed Segment Count:      " << words.size() << std::endl;
    
    for (size_t i = 0; i < words.size(); ++i) {
      os << "   -- Segment #" << i << std::endl;
      os << "      Eta Index:                     " << obj.segEtaIndex(i) << " (eta ~ " << obj.segEta(i) << ")" << std::endl;
      os << "      Phi Index:                     " << obj.segPhiIndex(i) << " (phi ~ " << obj.segPhi(i) << ")" << std::endl;
      os << "      Delta Theta Index:             " << static_cast<int>(obj.segDeltaThetaIndex(i)) << std::endl;
      os << "      Quality Flag:                  " << static_cast<int>(obj.segQuality(i)) << std::endl;
    }
    os << "*END* xAOD::NSWCandData" << std::endl;
    return os;
  }
  
} // namespace xAOD
