/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "xAODTrigL1Muon/versions/L1NSWCandData_v1.h"
#include "xAODCore/AuxStoreAccessorMacros.h"

#include "xAODTrigL1Muon/NSWTPBits.h"
#include <iostream>

namespace xAOD {

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1NSWCandData_v1, uint16_t, l1Bcid, setL1Bcid )

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1NSWCandData_v1, uint8_t, l1NSegments, setL1NSegments )

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1NSWCandData_v1, uint16_t, fiberID, setFiberID )

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1NSWCandData_v1, uint16_t, boardID, setBoardID )

  AUXSTORE_PRIMITIVE_SETTER_AND_GETTER( L1NSWCandData_v1, uint32_t, l1SegmentWord, setL1SegmentWord )

  bool L1NSWCandData_v1::l1Overflow() const {
    static const SG::Accessor<uint8_t> acc("l1Overflow");
    return acc(*this);
  }

  void L1NSWCandData_v1::setL1Overflow(bool value) {
    static const SG::Accessor<uint8_t> acc("l1Overflow");
    acc(*this) = value;
  }

  uint16_t L1NSWCandData_v1::segEtaIndex() const {
    return (l1SegmentWord() >> L1Muon::NSWTPBits::RUN4_NSWTP_ETA_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_ETA_MASK;
  }

  uint16_t L1NSWCandData_v1::segPhiIndex() const {
    return (l1SegmentWord() >> L1Muon::NSWTPBits::RUN4_NSWTP_PHI_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_PHI_MASK;
  }

  uint8_t L1NSWCandData_v1::segDeltaThetaIndex() const {
    return (l1SegmentWord() >> L1Muon::NSWTPBits::RUN4_NSWTP_DTH_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_DTH_MASK;
  }

  uint8_t L1NSWCandData_v1::segQuality() const {
    return (l1SegmentWord() >> L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_SHIFT) & L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_MASK;
  }

  float L1NSWCandData_v1::segEta() const {
    return L1Muon::NSWTPBits::ETA_MIN +
      (static_cast<float>(segEtaIndex()) / L1Muon::NSWTPBits::ETA_MAX_RAW) * (L1Muon::NSWTPBits::ETA_MAX - L1Muon::NSWTPBits::ETA_MIN);
  }

  float L1NSWCandData_v1::segPhi() const {
    return (static_cast<float>(segPhiIndex()) / L1Muon::NSWTPBits::PHI_MAX_RAW) * L1Muon::NSWTPBits::PHI_MAX;
  }

  void L1NSWCandData_v1::setSegment(uint16_t etaIndex, uint16_t phiIndex, uint8_t deltaThetaIndex, uint8_t quality) {
    // Pack the fields into a single 32-bit container word
    uint32_t packedWord = 0;
    packedWord |= (static_cast<uint32_t>(etaIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_ETA_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_ETA_SHIFT;
    packedWord |= (static_cast<uint32_t>(phiIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_PHI_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_PHI_SHIFT;
    packedWord |= (static_cast<uint32_t>(deltaThetaIndex) & L1Muon::NSWTPBits::RUN4_NSWTP_DTH_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_DTH_SHIFT;
    packedWord |= (static_cast<uint32_t>(quality) & L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_MASK) << L1Muon::NSWTPBits::RUN4_NSWTP_QUAL_SHIFT;

    setL1SegmentWord(packedWord);
  }

  // Debugging
  std::ostream& operator<<(std::ostream& os, const L1NSWCandData_v1& obj){
    os << "\n*BEGIN* xAOD::L1NSWCandData" << std::endl;
    os << "   BCID:                             " << obj.l1Bcid() << std::endl;
    os << "   Board ID:                         " << static_cast<int>(obj.boardID()) << std::endl;
    os << "   Fiber ID:                         " << static_cast<int>(obj.fiberID()) << std::endl;
    os << "   Number of Segments:               " << static_cast<int>(obj.l1NSegments()) << std::endl;
    os << "   Overflow Flag:                    " << (obj.l1Overflow() ? "TRUE" : "FALSE") << std::endl;
    os << "   Eta Index:                        " << obj.segEtaIndex() << " (eta ~ " << obj.segEta() << ")" << std::endl;
    os << "   Phi Index:                        " << obj.segPhiIndex() << " (phi ~ " << obj.segPhi() << ")" << std::endl;
    os << "   Delta Theta Index:                " << static_cast<int>(obj.segDeltaThetaIndex()) << std::endl;
    os << "   Quality Flag:                     " << static_cast<int>(obj.segQuality()) << std::endl;
    os << "*END* xAOD::L1NSWCandData" << std::endl;
    return os;
  }

} // namespace xAOD
