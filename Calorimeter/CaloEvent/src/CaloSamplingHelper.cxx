/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//*****************************************************************************
//
//  DESCRIPTION:
//     Implementation comments only.  Class level comments go in .h file.
//
//  HISTORY:
//
//  BUGS:
//    See also .h file.
//    
//  Updated May 25, 2001 HMA
//  LAr Sampling bug fix.
//
//  FCAL Sampling bug fix  (sam = 1-3, not 0-2) 
//  Feb 20, 2002, HMA 
//  fix for EM endcap inner wheel (SR)
//  Oct 2003: use caloDDE (DR)
//*****************************************************************************

#include "CaloEvent/CaloSamplingHelper.h"
#include "CaloEvent/CaloCell.h"
#include "CaloDetDescr/CaloDetDescrElement.h"
#include "Identifier/Identifier.h"
#include "CaloIdentifier/CaloID.h"

const unsigned int CaloSamplingHelper::m_EMMask     = 0x000000ff;
const unsigned int CaloSamplingHelper::m_HADMask    = 0x00ffff00;
const unsigned int CaloSamplingHelper::m_BarrelMask = 0x001ff00f;
const unsigned int CaloSamplingHelper::m_EndCapMask = 0x00e00ff0;
const unsigned int CaloSamplingHelper::m_LArMask    = 0x00e00fff;
const unsigned int CaloSamplingHelper::m_TileMask   = 0x001ff000;


CaloSamplingHelper::CaloSample
CaloSamplingHelper::getSampling( const CaloCell& cell ) {

  const CaloDetDescrElement * theCaloDDE= cell.caloDDE() ;
  if (theCaloDDE!=nullptr) {
    return (CaloSamplingHelper::CaloSample) theCaloDDE->getSampling();
      }
  else{
    return Unknown;
    
  }
}
  
unsigned int CaloSamplingHelper::getSamplingBit(const CaloCell& rCell)
{
  return getSamplingBit(getSampling(rCell));
}


bool CaloSamplingHelper::isEMSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_EMMask);
}

bool CaloSamplingHelper::isHADSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_HADMask);
}

bool CaloSamplingHelper::isBarrelSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_BarrelMask);
}

bool CaloSamplingHelper::isEndCapSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_EndCapMask);
}

bool CaloSamplingHelper::isLArSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_LArMask);
}

bool CaloSamplingHelper::isTileSampling(const CaloSample& rSample)
{
  return matchPattern(rSample,m_TileMask);
}

bool CaloSamplingHelper::matchPattern(const CaloSample& rSample,
				const unsigned int& mask)
{
  unsigned int bitPattern = getSamplingBit(rSample);
  return ( bitPattern & mask ) == bitPattern;
}

