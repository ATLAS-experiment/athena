/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Implementation of LArRODBlockStructure class

#include <cstdio>
#include <iostream>
#include "LArByteStream/LArRodBlockStructure.h"
#include "LArRawEvent/LArRawChannel.h"
#include "LArRawEvent/LArDigit.h"
#include "LArRawEvent/LArAccumulatedDigit.h"
#include "LArRawEvent/LArCalibDigit.h"
#include "LArRawEvent/LArAccumulatedCalibDigit.h"

const uint32_t LArRodBlockStructure::m_RawToOfflineGainMap[4]={0, 2, 1,0};
const uint32_t LArRodBlockStructure::m_OfflineToRawGainMap[3]={3,2,1};
LArRodBlockStructure::LArRodBlockStructure(IMessageSvc* msgSvc,
                                           const std::string& blockType)
  : m_Ex(0),
    m_Ey(0),
    m_Ez(0),
    m_SumE(0),
    m_iHeadBlockSize(0),
    m_error_next_feb(false),
    m_virtualROBJump(0),
    m_ROB_to_decode(0),
    m_logstr(msgSvc, blockType)
{
  m_channelsPerFEB=128;
  m_RodBlockSize=0;
  m_FebBlockSize=0;
  m_RodBlock=NULL;
  m_FebBlock=NULL;
  m_MiddleHeaderSize=7; 
  m_rearrangeFirstSample=0;
#ifndef NDEBUG
  m_numberOfEx=0;
  m_numberOfEy=0;
  m_numberOfEz=0;
  m_numberOfSumE=0;
#endif
  m_pRODblock=NULL;
  m_vFragment=NULL;
  m_virtualROBPointer=NULL;
  m_virtualROBPointerLocal=NULL;
}

LArRodBlockStructure::~LArRodBlockStructure()
{
}

//Error reporting for not implemented methods
void LArRodBlockStructure::setDAC(const uint16_t)
{
  m_logstr << MSG::ERROR << "Error: Function setDAC not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setDelay(const uint16_t)
{
  m_logstr << MSG::ERROR << "Error: Function setDelay not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setPulsed (const unsigned)
{
  m_logstr << MSG::ERROR << "Error: Function setPulsed not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setNTrigger (const uint16_t)
{
  m_logstr << MSG::ERROR << "Error: Function setNTrigger not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void  LArRodBlockStructure::concatinateFEBs()
{
  m_logstr << MSG::ERROR << "Error: Function concatinateFEBs not implemented in this instance of LArRodBlockStructure!" << endmsg;
}


int   LArRodBlockStructure::getNextRawData(int&, std::vector<short>&, uint32_t&)
{
  m_logstr << MSG::ERROR << "Error: Function getNextRawData not implemented in this instance of LArRodBlockStructure!" << endmsg;
  return 0;
}

int   LArRodBlockStructure::getNextAccumulatedCalibDigit(int&, std::vector < uint64_t >&, std::vector < uint64_t >& , uint32_t&, uint32_t&)
{
  m_logstr << MSG::ERROR << "Error: Function getNextAccumulatedCalibDigit not implemented in this instance of LArRodBlockStructure!" << endmsg;
  return 0;
}

int   LArRodBlockStructure::getNextAccumulatedDigit(int&, std::vector<uint64_t>&, std::vector < uint64_t >& , uint32_t&)
{
  m_logstr << MSG::ERROR << "Error: Function getNextAccumulatedDigit not implemented in this instance of LArRodBlockStructure!" << endmsg;
  return 0;
}

void LArRodBlockStructure::setNextEnergy(const int, const int32_t, const int32_t, const int32_t, const uint32_t)
{
  m_logstr << MSG::ERROR << "Error: Function setNextEnergy not implemented in this instance of LArRodBlockStructure!" << endmsg;
}


void LArRodBlockStructure::setRawData(const int , const std::vector<short>& , const uint32_t)
{
  m_logstr << MSG::ERROR << "Error: Function setRawData not implemented in this instance of LArRodBlockStructure!" << endmsg;
}


void LArRodBlockStructure::setEtQ(const int, const int32_t, const int32_t, const int32_t, const uint32_t)
{
  m_logstr << MSG::ERROR << "Error: Function setEtQ not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setRawDataFixed(const int , const std::vector<short>&, const uint32_t )
{
  m_logstr << MSG::ERROR << "Error: Function setRawDataFixed not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setNumberOfSamples(const uint8_t)
{
  m_logstr << MSG::ERROR << "Error: Function setNumberOfSamples not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setNumberOfGains(const uint8_t )
{
  m_logstr << MSG::ERROR << "Error: Function setNumberOfGains not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::setTDCPhase(const uint8_t)
{
  m_logstr << MSG::ERROR << "Error: Function setTDCPhase not implemented in this instance of LArRodBlockStructure!" << endmsg;
}
uint8_t LArRodBlockStructure::getTDCPhase() const
{
  return 0;
}

uint32_t LArRodBlockStructure::onlineCheckSum() const
{
  int offset   = getNumberOfWords()-1;
  if(offset>=m_FebBlockSize) return 0;
  if(offset<0)               return 0;
  return m_FebBlock[offset];
}

uint32_t LArRodBlockStructure::offlineCheckSum() const
{
  int end      = getNumberOfWords()-3;
  int start    = 1;
  uint32_t sum = 0;
  if(end>m_FebBlockSize) end = m_FebBlockSize;
  for(int i=start;i<end;i++) {
    sum += m_FebBlock[i];
  }
  return sum & 0x7fffffff;
}

void LArRodBlockStructure::dumpFragment()
{
  m_logstr << MSG::ERROR << "Error: Function dumpFragment() not implemented in this instance of LArRodBlockStructure!" << endmsg;
}


void LArRodBlockStructure::initializeFEB(const uint32_t)
{
  m_logstr << MSG::ERROR << "Error: Function initializeFEB not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::initializeFragment(std::vector<uint32_t>& )
{
  m_logstr << MSG::ERROR << "Error: Function initializeFragment not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void LArRodBlockStructure::finalizeFEB()
{
  m_logstr << MSG::ERROR << "Error: Function finalizeFEB not implemented in this instance of LArRodBlockStructure!" << endmsg;
}

void  LArRodBlockStructure::setEx(const double)
{
  // Do not set anything
}

void  LArRodBlockStructure::setEy(const double)
{
  // Do not set anything
}

void  LArRodBlockStructure::setEz(const double)
{
  // Do not set anything
}

void  LArRodBlockStructure::setSumE(const double)
{
  // Do not set anything
}

int  LArRodBlockStructure::setGain(const int)
{
  return 1;
}

//Default instances of sort-method. Default is to leave things unsorted.
void  LArRodBlockStructure::sortDataVector(std::vector<const LArRawChannel*>& )
{}

void  LArRodBlockStructure::sortDataVector( std::vector<const LArDigit*>& )
{}

void  LArRodBlockStructure::sortDataVector( std::vector<const LArCalibDigit*>& )
{}

void  LArRodBlockStructure::sortDataVector( std::vector<const LArAccumulatedCalibDigit*>& )
{}

void  LArRodBlockStructure::sortDataVector( std::vector<const LArAccumulatedDigit*>& )
{}

uint32_t LArRodBlockStructure::getNumberOfSamples()  const { return 0; }
uint32_t LArRodBlockStructure::getNumberOfGains()  const { return 0; }
uint32_t LArRodBlockStructure::getRadd(uint32_t /*adc*/, uint32_t /*sample*/)  const { return 0; }
uint16_t LArRodBlockStructure::getCtrl1(uint32_t /*adc*/)  const { return 0; }
uint16_t LArRodBlockStructure::getCtrl2(uint32_t /*adc*/)  const { return 0; }
uint16_t LArRodBlockStructure::getCtrl3(uint32_t /*adc*/)  const { return 0; }
uint32_t LArRodBlockStructure::getStatus()  const { return 0; }
uint16_t LArRodBlockStructure::getResults1Size() const { return 0; }
uint16_t LArRodBlockStructure::getResults2Size() const { return 0; }
uint16_t LArRodBlockStructure::getRawDataSize()  const { return 0; }
uint16_t LArRodBlockStructure::getNbSweetCells1()  const { return 0; }
uint16_t LArRodBlockStructure::getNbSweetCells2()  const { return 0; }

uint32_t LArRodBlockStructure::getDspCodeVersion()  const
{
  int i=getNumberOfWords()-2;
  return m_FebBlock[i]; 
}

int32_t LArRodBlockStructure::getDspEventCounter()  const
{ 
  int i=getNumberOfWords()-1;
  return m_FebBlock[i]; 
}
