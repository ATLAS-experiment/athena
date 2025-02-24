/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef INDET_PIXELEMULATEDDEFECTS_H
#define INDET_PIXELEMULATEDDEFECTS_H

#include "EmulatedDefects.h"
#include "PixelModuleHelper.h"

namespace InDet {
   /** Specialization of emulated defects conditions data for ITk pixels
    * Defect conditions data for defects which use addresses created by the PixelModuleHelper
    */
   class PixelEmulatedDefects : public EmulatedDefects<PixelModuleHelper>
   {};
}
#include "AthenaKernel/CLASS_DEF.h"
CLASS_DEF( InDet::PixelEmulatedDefects, 16782614, 1);
#include "AthenaKernel/CondCont.h"
CONDCONT_MIXED_DEF( InDet::PixelEmulatedDefects, 61574136);

#endif
