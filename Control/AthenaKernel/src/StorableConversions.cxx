/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaKernel/src/StorableConversions.cxx
 *  @brief convert to and from a SG storable
 * @author ATLAS Collaboration
 **/


#include "AthenaKernel/StorableConversions.h"
#include "AthenaKernel/CLIDRegistry.h"
#include "GaudiKernel/System.h"

#include "AthenaKernel/getMessageSvc.h"
#include "GaudiKernel/MsgStream.h"


namespace SG {


/**
 * @brief Try to get the pointer back from a @a DataObject,
 *        converted to be of type @a clid.
 * @param pDObj The @a DataObject.
 * @param clid The ID of the class to which to convert.
 * @param tinfo type_info for the object being converted (optional).
 * @param quiet If true, suppress warning messages.
 * @param irt To be called if we make a new instance.
 * @param isConst True if the object being converted is regarded as const.
 *
 * Only works if the @a DataObject is a @a DataBucket.
 * Returns 0 on failure,
 */
void* fromStorable(DataObject* pDObj, CLID clid,
                   const std::type_info* tinfo /*= nullptr*/,
                   bool quiet [[maybe_unused]] /*= false*/,
                   IRegisterTransient*irt/*= 0*/,
                   bool isConst /*= true*/)
{
  //check inputs
  if (0 == pDObj) {
    MsgStream gLog(Athena::getMessageSvc(), "SG::fromStorable");
    gLog << MSG::WARNING << "null input pointer " << endmsg;
    return nullptr;
  }

  // get T* from DataBucket:
  // All objects in the event store nowadays are instances
  // of DataBucket, so just do a static_cast.
  DataBucketBase* b = static_cast<DataBucketBase*>(pDObj);
  // Use BaseInfo information to convert pointers.

  void* ret = nullptr;
  if (tinfo)
    ret = b->cast (clid, *tinfo, irt, isConst);
  else
    ret = b->cast (clid, irt, isConst);
  if (!quiet && !ret) {
    const std::type_info* tid = CLIDRegistry::CLIDToTypeinfo (clid);
    MsgStream gLog(Athena::getMessageSvc(), "SG::fromStorable");
    gLog << MSG::WARNING 
         << "can't convert stored DataObject " << pDObj 
         << " to type ("
         << (tid ? System::typeinfoName (*tid) : "[unknown]")
         << ")\n Unless you are following a symlink,"
         << " it probably means you have a duplicate "
         << "CLID = "  << pDObj->clID() 
         << endmsg;
  }
  return ret;
}


/**
 * @brief Try to get the pointer back from a @a DataObject,
 *        converted to be of type @a clid.
 * @param pDObj The @a DataObject.
 * @param clid The ID of the class to which to convert.
 * @param tinfo type_info for the object being converted (optional).
 * @param quiet If true, suppress warning messages.
 * @param irt To be called if we make a new instance.
 * @param isConst True if the object being converted is regarded as const.
 *
 * Only works if the @a DataObject is a @a DataBucket.
 * Returns 0 on failure,
 */
void* Storable_cast(DataObject* pDObj, CLID clid,
                    const std::type_info* tinfo /*= nullptr*/,
                    bool quiet /*= false*/,
                    IRegisterTransient* irt /*= 0*/,
                    bool isConst /*= true*/)
{
  return fromStorable (pDObj, clid, tinfo, quiet, irt, isConst);
}


} // namespace SG
