/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATA_V1_H
#define XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATA_V1_H

// System include(s):
#include <cstdint>
#include <string>
#include <cmath>

// EDM include(s):
#include "AthContainers/AuxElement.h"

namespace xAOD {


  class SectorLogicCandData_v1 : public SG::AuxElement {

   public:

    /**
    * @enum MuonSystem
    * @brief Identifies the muon detector system
    */
    enum MuonSystem { RPC, TGC };

    /**
    * @enum Side
    * @brief Detector side (A or C)
    */
    enum Side { A, C };

    /**
    * @enum Msp
    * @brief Muon Sector Processor identifier
    */
    enum MSP { MSP_0, MSP_1};

    /**
    * @brief Default constructor
    */
    SectorLogicCandData_v1();

    /**
    * @brief Initialise the object given some input data
    * @param data Input data
    * @param offset BCID offset to know which timeslice the candidate is associated to
    * @param bID Board ID
    * @param fID Fiber ID
    */
    void initialize(const std::vector<uint32_t>& data, int offset, uint16_t bID, uint16_t fID);

    /**
    * @brief Get the information stored in the SectorLogicCandData_v1 object.
    * @return String holding all the object's information.
    */
    const std::string dump() const;

    /**
    * @brief Retrieve the candidate word (first 32-bits)
    * @return Candidate word
    */
    uint32_t candWord() const;

    /**
    * @brief Set the candidate word (first 32-bits)
    * @param word Candidate word to set
    */
    void setCandWord(uint32_t word);

    /**
    * @brief Retrieve the candidate extra word (second 32-bits)
    * @return Candidate extra word
    */
    uint32_t candExtraWord() const;

    /**
    * @brief Set the candidate extra word (second 32-bits)
    * @param word Candidate extra word to set
    */
    void setCandExtraWord(uint32_t word);
    
    /**
    * @brief Retrieve the board ID
    * @return Board ID
    */
    uint16_t boardID() const;

    /**
    * @brief Set the board ID
    * @param id Board ID to set
    */
    void setBoardID(uint16_t id);

    /**
    * @brief Retrieve the fiber ID
    * @return Fiber ID
    */
    uint16_t fiberID() const;

    /**
    * @brief Set the fiber ID
    * @param id Fiber ID to set
    */
    void setFiberID(uint16_t id);

    /**
    * @brief Retrieve the bunch crossing identifier offset
    * @return Bunch crossing identifier offset
    */
    int BCIDOffset() const;

    /**
    * @brief Set the bunch crossing identifier offset
    * @param offset Bunch crossing identifier offset to set
    */
    void setBCIDOffset(int offset);

    /**
    * @brief Retrieve the veto flag
    * @return Veto flag
    */
    unsigned short veto() const;

    /**
    * @brief Set the veto flag
    * @param veto Veto flag to set
    */
    void setVeto(unsigned short veto);

    /**
    * @brief Retrieve the pT value
    * @return The pT value
    */
    uint32_t pT() const;

    /**
    * @brief Retrieve the charge
    * @return The charge
    */
    uint32_t charge() const;

    /**
    * @brief Retrieve the bits for the position in phi
    * @return The bits for the position in phi
    */
    uint32_t rawPhi() const;

    /**
    * @brief Retrieve the position in phi
    * @return The position in phi
    */
    float phi() const;

    /**
    * @brief Retrieve the bits for the position in eta
    * @return The bits for the position in eta
    */
    uint32_t rawEta() const;

    /**
    * @brief Retrieve the position in eta
    * @return The position in eta
    */
    float eta() const;

    /**
    * @brief Retrieve the MDT segment quality flag
    * @return The MDT segment quality flag
    */
    uint32_t mdtSegQual() const;

    /**
    * @brief Retrieve the number of MDT segments
    * @return The number of MDT segments
    */
    uint32_t numMDTSeg() const;

    /**
    * @brief Retrieve the MDT flag
    * @return The MDT flag
    */
    uint32_t mdtFlag() const;

    /**
    * @brief Retrieve the exotic trigger
    * @return The exotic trigger
    */
    uint32_t exotTrig() const;

    /**
    * @brief Retrieve the TILE coincidence presence
    * @return The TILE coincidence presence
    */
    uint32_t tileCoin() const;

    /**
    * @brief Retrieve the RPC/TGC coincidence type
    * @return The RPC/TGC coincidence type
    */
    uint32_t coinType() const;

    /**
    * @brief Retrieve whether the candidate was processed by MDTTP.
    * @return Whether the candidate was processed by MDTTP.
    */
    uint32_t isMDT() const;

    /**
    * @brief Retrieve the Trigger candidate ID
    * @return The Trigger candidate ID
    */
    uint32_t TCID() const;

    /**
    * @brief Retrieve the Trigger candidate ID
    * @return The Trigger candidate ID
    */
    uint32_t ptThresh() const;

  private:

    // Needed for converting from bits to float for eta and phi
    static constexpr float ETA_MIN = -2.7f;
    static constexpr float ETA_MAX =  2.7f;
    static constexpr float ETA_MAX_RAW = 16383.0f;
    static constexpr float PHI_MAX = 2.0f * M_PI;
    static constexpr float PHI_MAX_RAW = 511.0f;

  }; // class SectorLogicCandData_v1

} // namespace xAOD

// Declare the inheritance of the class:
#include "xAODCore/BaseInfo.h"
SG_BASE( xAOD::SectorLogicCandData_v1, SG::AuxElement );

#endif // XAODTRIGL0MUON_VERSIONS_SECTORLOGICCANDDATA_V1_H
