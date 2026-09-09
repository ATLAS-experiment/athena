/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ADD_LZ4_TO_PLIST_H
#define ADD_LZ4_TO_PLIST_H

#include "LZ4Plugin/H5Zlz4.h"

namespace H5 {
  class DSetCreatPropList;
}

void addLz4ToPlist(H5::DSetCreatPropList&);

#endif
