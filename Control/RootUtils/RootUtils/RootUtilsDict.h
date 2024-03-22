// This file's extension implies that it's C, but it's really -*- C++ -*-.

/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file  RootUtils/RootUtilsDict.h
 * @author scott snyder
 * @date Oct 2007
 * @brief Dictionary header for RootUtils.
 */

namespace RootUtilsTest {

// Small class used for unit testing.
struct TreeTest
{
  int i;
  float f;
};

}

#include "RootUtils/InitHist.h"
#include "RootUtils/ILogger.h"
#include "RootUtils/ScatterH2.h"

// Default names of ROOT file storage elements used by APR
#include "RootUtils/APRDefaults.h"


