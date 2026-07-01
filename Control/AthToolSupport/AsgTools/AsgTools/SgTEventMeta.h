/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ASGTOOLS_SGTEVENTMETA_H
#define ASGTOOLS_SGTEVENTMETA_H

#pragma GCC warning "SgTEventMeta.h is deprecated; use SgEventMeta.h"

// Local include(s):
#include "AsgTools/SgEventMeta.h"

// Declare backwards compatible typedef:
namespace asg {
  using SgTEventMeta = SgEventMeta;
} // namespace asg


#endif // ASGTOOLS_SGTEVENTMETA_H
