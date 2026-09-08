/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
/**
 * @file AthenaPoolCnvSvc/src/TPCnvElt.cxx
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Helper for calling a TP converter from an Athena converter.
 */


#include "AthenaPoolCnvSvc/TPCnvElt.h"
#include "AthenaPoolCnvSvc/exceptions.h"
#include "StorageSvc/DbReflex.h"
#include "StorageSvc/DbTypeInfo.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/System.h"
#include "DataModelRoot/RootType.h"


namespace AthenaPoolCnvSvc {


/**
 * @brief Given a @c type_info, get the corresponding pool guid.
 * @param ti @c type_info to look for.
 *
 * Throws an exception on errors.
 */
Guid guidFromTypeinfo (const std::type_info& ti)
{
  RootType temp = RootType(ti);
  return pool::DbReflex::guid ((temp) ? temp : RootType(pool::DbTypeInfo::typeName(ti)));
}


} // namespace AthenaPoolCnvSvc
