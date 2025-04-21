/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHEXSTOREGATEEXAMPLE_MAPSTRINGFLOAT_H
#define ATHEXSTOREGATEEXAMPLE_MAPSTRINGFLOAT_H

// CLIDs
#include "SGTools/StlMapClids.h"

/// a map of float keyed by string
typedef std::map<std::string,float> MapStringFloat;

//describe the container category to ElementLink
#include "AthLinks/DeclareIndexingPolicy.h"
CONTAINER_IS_MAP( MapStringFloat )


#endif // MAPSTRINGFLOAT_H
