/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef EFTRACKING_XRT_PARAMETERS
#define EFTRACKING_XRT_PARAMETERS

namespace EFTrackingXrtParameters {
enum InterfaceMode {
  INPUT,
  OUTPUT
};

// In the python we "subtract" the members of EmptyEnum from InterfaceMode to 
// get InterfaceMode's fields. 
enum EmptyEnum {};
}

#endif

