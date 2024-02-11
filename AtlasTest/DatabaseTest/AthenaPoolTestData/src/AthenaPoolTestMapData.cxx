/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file AthenaPoolTestMapData.cxx 
 * 
 * @brief
 * 
 * @author RD Schaffer <R.D.Schaffer@cern.ch>
 * 
 * $Id: AthenaPoolTestMapData.cxx,v 1.2 2009-02-15 12:56:15 schaffer Exp $
 */

//<<<<<< INCLUDES                                                       >>>>>>

#include "AthenaPoolTestData/AthenaPoolTestMapData.h"
#include <iostream>

AthenaPoolTestMapData::AthenaPoolTestMapData() :
  m_word(0)
{
  
}

AthenaPoolTestMapData::AthenaPoolTestMapData (const std::vector<Deposit>& deposits, 
			    int simDataWord)
    : m_word( ((deposits.size() << 29) & 0xe0000000) | (simDataWord & 0x1fffffff)),
      m_deposits (deposits)
{
}

AthenaPoolTestMapData::~AthenaPoolTestMapData()
{
}

