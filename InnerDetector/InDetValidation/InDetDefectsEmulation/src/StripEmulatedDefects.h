/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_STRIPEMULATEDDEFECTS_H
#define INDET_STRIPEMULATEDDEFECTS_H

#include "EmulatedDefects.h"
#include "StripModuleHelper.h"

namespace InDet {
   /** Specialization of emulated defects conditions data for ITk strips
    * Defect conditions data for defects which use addresses created by the StripModuleHelper
    */
   class StripEmulatedDefects : public EmulatedDefects<StripModuleHelper>
   {};
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( InDet::StripEmulatedDefects, 77695516, 1)

CONDCONT_MIXED_DEF( InDet::StripEmulatedDefects, 221690018);

#endif
