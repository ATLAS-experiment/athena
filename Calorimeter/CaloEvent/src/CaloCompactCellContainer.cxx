/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

//-----------------------------------------------------------------------
// File and Version Information:
// $Id: CaloCompactCellContainer.cxx,v 1.2 2004-08-05 22:15:01 menke Exp $
//
// Description: see CaloCompactCellContainer.h
//
// Environment:
//      Software developed for the ATLAS Detector at the CERN LHC
//
// Author List:
//      Sven Menke
//
//-----------------------------------------------------------------------

#include "CaloEvent/CaloCompactCellContainer.h"


const std::vector<CaloCompactCellContainer::value_type> CaloCompactCellContainer::getHeader() const
{
  std::vector<value_type> theHeader(m_compactData[0]);
  for(int i=0;i<m_compactData[0];i++)
    theHeader[i] = m_compactData[i];
  return theHeader;
}

void CaloCompactCellContainer::resize (const unsigned int & totalSize)
{
  m_compactData.resize(totalSize);
}

void CaloCompactCellContainer::setHeader
(const std::vector<CaloCompactCellContainer::value_type> & theHeader) 
{
  for(unsigned int i=0;i<theHeader.size();i++)
    m_compactData[i] = theHeader[i];
}
