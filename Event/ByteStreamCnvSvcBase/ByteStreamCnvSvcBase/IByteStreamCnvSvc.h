/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BYTESTREAMCNVSVCBASE_IBYTESTREAMCNVSVC_H
#define BYTESTREAMCNVSVCBASE_IBYTESTREAMCNVSVC_H

#include "GaudiKernel/IInterface.h"

#include "AthenaKernel/errorcheck.h"
#include "ByteStreamData/RawEvent.h"
#include "ByteStreamCnvSvcBase/FullEventAssembler.h"


/** @class IByteStreamCnvSvc
 * @brief interface for bytestream conversion services
 */
class IByteStreamCnvSvc : virtual public IInterface {
public:
   /// Gaudi interface id
   DeclareInterfaceID(IByteStreamCnvSvc, 1, 0);

   /// Access to FullEventAssembler
   template <class T> StatusCode getFullEventAssembler(T*&t, const std::string& name);

protected:

   /** Find FullEventAssembler with given name.
    *  @return pointer to FullEventAssembler or nullptr if it does not exist
    */
   virtual FullEventAssemblerBase* findFullEventAssembler(const std::string& name) const = 0;

   /** Store FullEventAssembler under given name.
    *  @return Success or failure.
    */
   virtual StatusCode storeFullEventAssembler(std::unique_ptr<FullEventAssemblerBase> fea, const std::string& name) = 0;
};


// Implementation of template method:
template <class T>
StatusCode IByteStreamCnvSvc::getFullEventAssembler(T*& t, const std::string& name)
{
   // Find existing FEA
   FullEventAssemblerBase* fea = findFullEventAssembler(name);
   if (fea) {
      t = dynamic_cast<T*>(fea);
      if (t == nullptr) {
         REPORT_MESSAGE_WITH_CONTEXT(MSG::ERROR, "IByteStreamCnvSvc")
            << "FullEventAssembler with key '" << name << "' exists, but of different type" << endmsg;
         return StatusCode::FAILURE;
      }
      return StatusCode::SUCCESS;
   }
   else { // create new FEA
      auto ptr = std::make_unique<T>();
      t = ptr.get();
      return storeFullEventAssembler(std::move(ptr), name);
   }
}

#endif
