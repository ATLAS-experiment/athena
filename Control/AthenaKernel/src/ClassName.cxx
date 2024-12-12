/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file  AthenaKernel/src/ClassName.cxx
 * @author scott snyder
 * @date Nov 2024
 * @brief An interface for getting the name of a class as a string.
 */


#include "AthenaKernel/ClassName.h"
#include "GaudiKernel/System.h"


/**
 * Convert a @c type_info to a demangled string.
 * @param ti The type_info to convert.
 *
 * A wrapper around System::typeinfoName.  Moved out-of-line so that this
 * header does not need to depend on System.h.
 */
std::string Athena::typeinfoName (const std::type_info& ti)
{
  return System::typeinfoName (ti);
}
