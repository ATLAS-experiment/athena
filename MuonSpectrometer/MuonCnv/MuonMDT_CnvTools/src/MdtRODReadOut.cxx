/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MdtRODReadOut.h"

#include "AthenaKernel/errorcheck.h"


MdtRODReadOut::MdtRODReadOut() :
    // m_dataWord(0),
    m_subdetId(0),
    m_mrodId(0),
    m_lvl1Id(0),
    m_bcId(0),
    m_triggerTypeId(0) {}

// Decode the ROD header
void MdtRODReadOut::decodeHeader(const std::vector<uint32_t>& p) {

    setZero();

    if (p[0] != s_RODstart) {
      REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "MdtRODReadOut")
        << "ROD Start of header marker not found" << endmsg;
    }
    if (p[1] != s_RODheadersize) {
      REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "MdtRODReadOut")
        << "ROD header size doesn't match " << s_RODheadersize << endmsg;
    }

    // decode the rest of the header
    // Subdetector Id and mrodId
    m_word = p[3];
    m_subdetId = getBits(getBitsWord(15, 8));
    m_mrodId = getBits(getBitsWord(7, 0));

    // Lvl1Id
    m_word = p[4];
    m_lvl1Id = getBits(getBitsWord(23, 0));

    // Bunch crossing identifier
    m_word = p[5];
    m_bcId = getBits(getBitsWord(11, 0));

    // Trigger type Id
    m_word = p[6];
    m_triggerTypeId = getBits(getBitsWord(7, 0));
}

// Decode the ROD footer
void MdtRODReadOut::decodeFooter(const std::vector<uint32_t>& /*p*/) {}

uint32_t MdtRODReadOut::makeRODId(uint16_t subdet, uint16_t mrod) {
    uint16_t inputData[4] = {0, 0, subdet, mrod};
    uint16_t inputPos[4] = {24, 16, 8, 0};
    uint16_t nData = 4;

    return setBits(nData, inputData, inputPos);
}

uint32_t* MdtRODReadOut::encodeFooter() { return nullptr; }

void MdtRODReadOut::setZero() {
    m_subdetId = 0;
    m_mrodId = 0;
    m_lvl1Id = 0;
    m_bcId = 0;
    m_triggerTypeId = 0;
}
