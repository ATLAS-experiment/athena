/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGTOOLS_SGTEVENT_H
#define ASGTOOLS_SGTEVENT_H

#pragma GCC warning "SgTEvent.h is deprecated; use SgEvent.h"

// Local include(s):
#include "AsgTools/SgEvent.h"

// Declare backwards compatible typedef:
namespace asg {
  using SgTEvent = SgEvent;
} // namespace asg

#endif // ASGTOOLS_SGTEVENT_H
