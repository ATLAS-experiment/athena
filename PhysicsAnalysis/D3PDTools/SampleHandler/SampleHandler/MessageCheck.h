/*
  Copyright (C) 2002-2019 CERN for the benefit of the ATLAS collaboration
*/

/// @author Nils Krumnack



#ifndef SAMPLE_HANDLER__MESSAGE_CHECK_H
#define SAMPLE_HANDLER__MESSAGE_CHECK_H

#include <SampleHandler/Global.h>

#include <AsgMessaging/MessageCheck.h>

namespace SH
{
  ANA_MSG_HEADER (msgScanDir)
  ANA_MSG_HEADER (msgFetch)
  ANA_MSG_HEADER (msgDiscovery)
  ANA_MSG_HEADER (msgSplit)
  ANA_MSG_HEADER (msgDuplicates)
}

#endif
