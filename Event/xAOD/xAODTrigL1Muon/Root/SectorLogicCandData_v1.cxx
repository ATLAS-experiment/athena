/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// System include(s):
#include <sstream>

// xAOD include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODTrigL1Muon/versions/SectorLogicCandData_v1.h"

// Trigger include(s):
#include "TrigT1MuctpiBits/MuCTPI_Bits.h"

namespace xAOD {

   

   // Default constructor just creates empty AuxElement
   SectorLogicCandData_v1::SectorLogicCandData_v1()
      : SG::AuxElement() {
   }

   // Intitialise using input bits from RPC & TGC SL
   void SectorLogicCandData_v1::initialize(const std::vector<uint32_t>& data, int offset, uint16_t bID, uint16_t fID) {
      
      // Check input format
      if (data.size() != 2) {
         setCandWord(0);
         setCandExtraWord(0);
      } else {
         setCandWord(data[0]);
         setCandExtraWord(data[1]);
      }
      setBoardID(bID);        // Set the board ID to provide information of where the candidate originates (i.e. what SL board)
      setFiberID(fID);        // Set the fiber ID to provide information of what link the candidate arrives on
      setVeto(0);             // Always initialise candidate with no veto -> veto comes from overlap handling
      setBCIDOffset(offset);  // Set the BCID offset, i.e. which timeslice it is associated to

   }

   // Save the information of the object to a string. Useful for debugging.
   // Dump function with padded hex for all fields
   const std::string SectorLogicCandData_v1::dump() const {
      std::ostringstream s;
      s << "\n*BEGIN* xAOD::SectorLogicCandData" << std::endl;
      s << "   Trigger Candidate ID:              " << TCID() << std::endl;
      s << "   Veto:                              " << veto() << std::endl;
      s << "   Board ID:                          " << boardID() << std::endl;
      s << "   Fiber ID:                          " << fiberID() << std::endl;
      s << "   BCID offset:                       " << BCIDOffset() << std::endl;
      s << "   Position in phi:                   " << phi() << std::endl;
      s << "   Position in eta:                   " << eta() << std::endl;
      s << "   pT value:                          " << pT() << std::endl;
      s << "   pT threshold:                      " << ptThresh() << std::endl;
      s << "   Charge:                            " << (charge() ? "Positive" : "Negative") << std::endl;
      s << "   MDT processing flag [0:15]:        " << mdtFlag() << std::endl;
      s << "   Processed by MDT or RPC/TGC:       " << (isMDT() ? "MDT" : "RPC/TGC") << std::endl;
      s << "   MDT segment quality:               " << mdtSegQual() << std::endl;
      s << "   Number of associated MDT segments: " << numMDTSeg() << std::endl;
      s << "   Exotic trigger:                    " << exotTrig() << std::endl;
      s << "   Presence of TILE Coincidence:      " << (tileCoin() ? "Yes" : "No") << std::endl;
      s << "   RPC/TGC coincidence type:          " << coinType() << std::endl;
      s << "*END* xAOD::SectorLogicCandData" << std::endl;
      return s.str();
   }

   // Get/set candidate word information
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, uint32_t, candWord, setCandWord)

   // Get/set the extra candidate word information
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, uint32_t, candExtraWord, setCandExtraWord)

   // Get/set the BoardID
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, uint16_t, boardID, setBoardID)

   // Get/set the FiberID
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, uint16_t, fiberID, setFiberID)

   // Get/set the BCID offset
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, int, BCIDOffset, setBCIDOffset)

   // Get/set the veto flag from overlap handling
   AUXSTORE_PRIMITIVE_SETTER_AND_GETTER(SectorLogicCandData_v1, unsigned short, veto, setVeto)

   // Get the pT value from word
   uint32_t SectorLogicCandData_v1::pT() const {
      return (candWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PT_VAL_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PT_VAL_MASK;
   }

   // Get the Charge from word
   uint32_t SectorLogicCandData_v1::charge() const {
      return (candWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_CHARGE_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_CHARGE_MASK;
   }

   // Get bits for phi position from word
   uint32_t SectorLogicCandData_v1::rawPhi() const {
      return (candWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PHI_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PHI_MASK;
   }

   // Get float phi value from phi bits
   float SectorLogicCandData_v1::phi() const {
      return (static_cast<float>(rawPhi()) / PHI_MAX_RAW) * PHI_MAX;
   }

   // Get bits for eta position from word
   uint32_t SectorLogicCandData_v1::rawEta() const {
      return (candWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_ETA_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_ETA_MASK;
   }

   // Get float eta value from eta bits
   float SectorLogicCandData_v1::eta() const {
      return ETA_MIN + (static_cast<float>(rawEta()) / ETA_MAX_RAW) * (ETA_MAX - ETA_MIN);
   }

   // Get the pT threshold from word
   uint32_t SectorLogicCandData_v1::ptThresh() const {
   return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PTTHRESHOLD_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_PTTHRESHOLD_MASK;
   }

   // Get the TCID from word
   uint32_t SectorLogicCandData_v1::TCID() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_TCID_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_TCID_MASK;
   }

   // Get the isMDT flag from word
   uint32_t SectorLogicCandData_v1::isMDT() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_MDT_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_MDT_MASK;
   }

   // Get the coincidence type flag from word
   uint32_t SectorLogicCandData_v1::coinType() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_COINTYPE_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_COINTYPE_MASK;
   }

   // Get the tile coincidence presence flag from word
   uint32_t SectorLogicCandData_v1::tileCoin() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_TC_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_TC_MASK;
   }

   // Get the exotic trigger flag from word
   uint32_t SectorLogicCandData_v1::exotTrig() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_ET_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_ET_MASK;
   }

   // Get the MDT flag from word
   uint32_t SectorLogicCandData_v1::mdtFlag() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_MDTFLAG_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_MDTFLAG_MASK;
   }

   // Get the MDT segment number from word
   uint32_t SectorLogicCandData_v1::numMDTSeg() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_SEGNUM_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_SEGNUM_MASK;
   }

   // Get the MDT segment quality from word
   uint32_t SectorLogicCandData_v1::mdtSegQual() const {
      return (candExtraWord() >> L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_SEGQUAL_SHIFT) & L0Muon::MuCTPIBits::RUN4_SL2MUCTPI_SEGQUAL_MASK;
   }

} // namespace xAOD
