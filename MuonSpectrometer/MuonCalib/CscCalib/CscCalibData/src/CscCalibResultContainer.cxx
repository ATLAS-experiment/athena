/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/*********************
Author Caleb Parnell-Lampen <lampen@physics.arionz.edu>
University of Arizona
****************************/
#include "CscCalibData/CscCalibResultContainer.h"
const std::string& CscCalibResultContainer::calibType() const
{
    return m_calibType;
}

