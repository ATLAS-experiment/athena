/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "ByteStreamCnvSvcBase/ByteStreamCnvSvcBase.h"
#include "ByteStreamCnvSvcBase/ByteStreamAddress.h"

#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IConverter.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IClassIDSvc.h"

//______________________________________________________________________________
ByteStreamCnvSvcBase::ByteStreamCnvSvcBase(const std::string& name, ISvcLocator* pSvcLocator)
  : base_class(name, pSvcLocator, ByteStreamAddress::storageType()) {}
//______________________________________________________________________________
/// Standard Destructor
ByteStreamCnvSvcBase::~ByteStreamCnvSvcBase() = default;
//______________________________________________________________________________
/// Initialize the service.
StatusCode ByteStreamCnvSvcBase::initialize() {
   ATH_CHECK(::AthCnvSvc::initialize());

   ServiceHandle<IIncidentSvc> incsvc("IncidentSvc", this->name());
   ATH_CHECK(incsvc.retrieve());
   incsvc->addListener(this, "BeginRun", 0, false, true); // true for singleshot
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
StatusCode ByteStreamCnvSvcBase::updateServiceState(IOpaqueAddress* pAddress) {
   if (pAddress != nullptr) {
      GenericAddress* pAddr = dynamic_cast<GenericAddress*>(pAddress);
      if (pAddr != nullptr) {
         return StatusCode::SUCCESS;
      }
   }
   return StatusCode::FAILURE;
}
//______________________________________________________________________________
void ByteStreamCnvSvcBase::handle(const Incident& /*incident*/) {
   ServiceHandle<IClassIDSvc> clidSvc("ClassIDSvc", name());
   if (!clidSvc.retrieve().isSuccess()) {
      ATH_MSG_ERROR("Cannot get ClassIDSvc.");
      return;
   }
   // Initialize the converters
   for (const std::string& cnv : m_initCnvs.value()) {
      ATH_MSG_DEBUG("Accessing Converter for " << cnv);
      CLID id;
      if (!clidSvc->getIDOfTypeName(cnv, id).isSuccess()) {
         ATH_MSG_WARNING("Cannot get CLID for " << cnv);
      } else {
         IConverter* cnv = converter(id);
         if (cnv == nullptr) {
            ATH_MSG_WARNING("Cannot get converter for " << cnv);
         }
      }
   }
}
