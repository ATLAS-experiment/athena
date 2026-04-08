/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "LArByteStream/Mon.h"

#define NSTREAMS 62
#define STREAMNUMBER 0
#define NBCS 32
#define NACTIVESCS 320

Mon::Mon(IMessageSvc* msgSvc)
  : headerMarker(0xFF1234FF), debugMarker(0xDEADBEEF), nStreams(NSTREAMS), streamNumber(STREAMNUMBER),
    m_logstr(msgSvc, "LArByteStream.Mon")
{}

void Mon::fillPacketInfo(uint32_t word) {
  nPackets = static_cast<int>((word & 0xff000000) >> 24);
  packetIndex = static_cast<int>((word & 0xff0000) >> 16);
  uint16_t sizeInBytes = static_cast<uint16_t>(word & 0xffff);
  if (sizeInBytes % 4) m_logstr << MSG::ERROR << "ERROR: Packet size written in the mon header is not multiple of 4 (cannot be converted from [bytes] to [32 bit words])" << endmsg;
  else packetSize = static_cast<int>(sizeInBytes / 4);
}

void Mon::fillRegion(uint32_t word) {
  region = static_cast<int>(word);
  switch (region) {
  case 0: return;
  case 1: return;
  case 2: return;
  case 3: return;
  case 4: return;
  case 5: return;
  default:
    m_logstr << MSG::ERROR << "Unknown calorimeter region word in mon header." << endmsg;
  }
}

void Mon::fillNStreams(uint32_t word) {
  nStreams = static_cast<int>(word);
  if (nStreams != NSTREAMS) m_logstr << MSG::ERROR << "Number of streams word in mon header is not the standard one." << endmsg;
}

void Mon::fillStreamNumber(uint32_t word) {
  streamNumber = static_cast<int>(word);
  if (streamNumber != STREAMNUMBER) m_logstr << MSG::ERROR << "Stream number word in mon header is not the standard " << endmsg;
}

void Mon::fillDataType(uint32_t word, int i) {
  dataType[i] = static_cast<int>(word);
  switch (dataType[i]) {
  case 0: return;
  case 1: return;
  case 2: return;
  case 3: return;
  case 0xff: return;
  default:
    m_logstr << MSG::ERROR << "Unknown calorimeter region word in mon header." << endmsg;
  }
}

void Mon::fillNBCs(uint32_t word, int i) {
  nBCs[i] = static_cast<int>(word);
  if (nBCs[i] != NBCS) m_logstr << MSG::ERROR << "Number of BCs word in mon header is not the standard one." << endmsg;
}

void Mon::fillTimeShift(uint32_t word, int i) {
  timeShift[i] = word;
}

void Mon::fillNActiveSCs(uint32_t word) {
  nActiveSCs = static_cast<int>(word);
  if (nActiveSCs != NACTIVESCS) m_logstr << MSG::ERROR << "Number of BCs word in mon header is not the standard one." << endmsg;
}
