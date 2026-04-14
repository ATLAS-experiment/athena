/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ATHENAPOOLCNVSVC_T_ATHENAPOOLCUSTCNV_H
#define ATHENAPOOLCNVSVC_T_ATHENAPOOLCUSTCNV_H

/** @file T_AthenaPoolCustCnv.h
 *  @brief his file contains the class definition for the templated T_AthenaPoolCustCnv class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc/T_AthenaPoolCnvBase.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Guid.h"

namespace pool {
   typedef ::Guid Guid;
}

#include <string>
#include <map>

class DataObject;
class StatusCode;

/// Abstract factory to create the converter
template <class TYPE> class CnvFactory;

/** @class T_AthenaPoolCustCnv
 *  @brief This templated class provides the converter to translate an object to/from its persistent POOL representation.
 **/
template <class TRANS, class PERS>
class T_AthenaPoolCustCnv : public T_AthenaPoolCnvBase<TRANS> {
   friend class CnvFactory<T_AthenaPoolCustCnv<TRANS, PERS> >;

protected:
   /// Constructor
   T_AthenaPoolCustCnv(ISvcLocator* pSvcLocator,
                       const char* name = nullptr);

   /// Gaudi Service Interface method implementations:
   virtual StatusCode initialize();

   /// Set POOL placement for type P.
   template <class P>
   Placement setPlacementForP(P& p, const std::string& key, const std::string& output);

   /// Write an object into POOL.
   /// @param pObj [IN] pointer to the transient object.
   /// @param key [IN] StoreGate key (string) - placement hint to generate POOL container name
   virtual StatusCode DataObjectToPool(IOpaqueAddress* pAddr, DataObject* pObj);

   /// Read an object from POOL.
   /// @param pObj [OUT] pointer to the transient object.
   /// @param token [IN] POOL token of the persistent representation.
   /// @param key [IN] SG key of the object being read.
   virtual StatusCode PoolToDataObject(DataObject*& pObj, const Token* token,
                                       const std::string& key);

   /// Write an object into POOL returning its token.
   /// @param pObj  [IN]  pointer to the object to be written.
   /// @param token [OUT] POOL token of the persistent representation.
   /// @param key [IN] StoreGate key (string) - placement hint to generate POOL container name
   template <class P>
   StatusCode objectToPool(P* pObj, std::unique_ptr<Token>& token, const std::string& key, const std::string& output);

   /// Read an object from POOL.
   /// @param token [IN]  POOL token of the persistent representation.
   /// @param pObj  [OUT] pointer to the object read.
   template <class P>
   StatusCode poolToObject(const Token* token, P*& pObj);

   virtual StatusCode transToPers(TRANS* obj, PERS*& persObj) = 0;
   virtual StatusCode persToTrans(TRANS*& transObj, PERS* obj) = 0;

   /// Compare POOL class GUID with the one of the object being read
   /// @param clid [IN] GUID to compare
   /// @return true if the type of the object about to be read matches the GUID
   virtual bool	compareClassGuid(const Token* token, const Guid &clid) const;

public:
   /// @return class ID.
   static CLID classID();

private:
   std::string           m_className;

   typedef std::map<std::string, RootType>            ClassMap;
   ClassMap              m_classDescs;
};

#include "AthenaPoolCnvSvc/T_AthenaPoolCustCnv.icc"
#endif
