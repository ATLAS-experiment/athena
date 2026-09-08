/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AuxDiscoverySvc.cxx
 *  @brief This file contains the implementation for the AuxDiscoverySvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AuxDiscoverySvc.h"

#include "AthContainersInterfaces/IAuxStoreIO.h"
#include "AthContainersInterfaces/IAuxStoreHolder.h"
#include "AthContainers/AuxStoreInternal.h"
#include "AthContainers/AuxTypeRegistry.h"
#include "AthContainers/tools/AuxVectorInterface.h"
#include "AthContainers/normalizedTypeinfoName.h"
#include "AthContainersRoot/getDynamicAuxID.h"
#include "AthenaKernel/IAthenaSerializeSvc.h"
#include "AthenaKernel/IAthenaIPCTool.h"

#include "RootUtils/Type.h" //FIXME: Avoid new dependency
#include "TClass.h"
#include <cstring>
#include <stdexcept>

class AthenaPoolAuxStore : public SG::AuxStoreInternal {
public:
  AthenaPoolAuxStore(bool standalone) : SG::AuxStoreInternal(standalone) {}
  using SG::AuxStoreInternal::addVector;
};

AuxDiscoverySvc::AuxDiscoverySvc(const IAthenaSerializeSvc* serSvc, IAthenaIPCTool* ipcTool) : AthMessaging("AuxDiscoverySvc"), m_serSvc(serSvc), m_ipcTool(ipcTool) {
}

AuxDiscoverySvc::~AuxDiscoverySvc() {
}

StatusCode AuxDiscoverySvc::receiveStore(TClass* cl, void* obj, int num) {
   TClass* holderTC = cl->GetBaseClass("SG::IAuxStoreHolder");
   if (holderTC == nullptr) {
      return(StatusCode::FAILURE);
   }
   SG::IAuxStoreHolder* storeHolder = reinterpret_cast<SG::IAuxStoreHolder*>((char*)obj + cl->GetBaseClassOffset(holderTC));
   if (storeHolder == nullptr) {
      return(StatusCode::FAILURE);
   }
   bool standalone = storeHolder->getStoreType() == SG::IAuxStoreHolder::AST_ObjectStore;
   std::unique_ptr<AthenaPoolAuxStore> storeInt = std::make_unique<AthenaPoolAuxStore>(standalone);

   void* nameData = nullptr;
   // StreamingTool owns buffer, will stay around until last dynamic attribute is copied
   size_t nbytes = 0;
   while (m_ipcTool->getObject(&nameData, nbytes, num).isSuccess() && nbytes > 0) {
      const char* del1 = static_cast<const char*>(std::memchr(nameData, '\n', nbytes));
      if (del1 == nullptr) continue;
      const char* del2 = static_cast<const char*>(std::memchr(del1 + 1, '\n', nbytes - (del1 - static_cast<const char*>(nameData) + 1)));
      if (del2 == nullptr) continue;
      const std::string dataStr(static_cast<const char*>(nameData));
      const std::string& attrName = dataStr.substr(0, del1 - static_cast<const char*>(nameData));
      const std::string& typeName = dataStr.substr(del1 - static_cast<const char*>(nameData) + 1, del2 - del1 - 1);
      const std::string& elemName = dataStr.substr(del2 - static_cast<const char*>(nameData) + 1, nbytes - (del2 - static_cast<const char*>(nameData) + 1));
      void* buffer = nullptr;
      if (m_ipcTool->getObject(&buffer, nbytes, num).isSuccess()) {
         SG::AuxTypeRegistry& registry = SG::AuxTypeRegistry::instance();
         SG::auxid_t auxid = registry.findAuxID(attrName);
         if (auxid == SG::null_auxid) {
            try {
            RootUtils::Type elemType(elemName);
               const std::type_info* eti = elemType.getTypeInfo();
               if (eti != nullptr) {
                  auxid = SG::getDynamicAuxID(*eti, attrName, elemName, typeName, storeInt->standalone(), SG::null_auxid);
               }
            } catch (const std::runtime_error&) {
               auxid = SG::null_auxid;
            }
         }
         if (auxid != SG::null_auxid) {
            const RootType type(typeName);
            void* dynAttr = nullptr;
            if (type.IsFundamental()) {
               dynAttr = new char[nbytes];
               std::memcpy(dynAttr, buffer, nbytes); buffer = nullptr;
            } else {
               dynAttr = m_serSvc->deserialize(buffer, nbytes, type); buffer = nullptr;
            }
            if (storeInt->standalone()) {
               (void)storeInt->getData(auxid, 1, 1);
               registry.copy(auxid,
                             SG::AuxVectorInterface(*storeInt), 0,
                             SG::AuxVectorInterface(auxid, 1, const_cast<const void*>(dynAttr)), 0, 1);
               if (type.IsFundamental()) {
                  delete [] (char*)dynAttr; dynAttr = nullptr;
               } else {
                  type.Destruct(dynAttr); dynAttr = nullptr;
               }
            } else {
               // Move the data to the dynamic store.
               std::unique_ptr<SG::IAuxTypeVector> vec(registry.makeVectorFromData(auxid, dynAttr, nullptr, false, true));
               storeInt->addVector(std::move(vec), false);
            }
         }
      }
   }
   storeHolder->setStore(storeInt.release());
   return(StatusCode::SUCCESS);
}

StatusCode AuxDiscoverySvc::sendStore(TClass* cl,
                const void* obj,
		const std::string& classId,
		const std::string& contName,
		int num) {
   TClass* storeTC = cl->GetBaseClass("SG::IAuxStoreIO");
   if (storeTC == nullptr) {
      return(StatusCode::SUCCESS);
   }
   const SG::IAuxStoreIO* store = reinterpret_cast<const SG::IAuxStoreIO*>((const char*)obj + cl->GetBaseClassOffset(storeTC));
   if (store == nullptr) {
      return(StatusCode::FAILURE);
   }
   const SG::auxid_set_t& auxIDs = store->getSelectedAuxIDs();
   if (!auxIDs.empty()) {
      if (!m_ipcTool->putObject(classId.c_str(), classId.size() + 1, num).isSuccess()) {
         return(StatusCode::FAILURE);
      }
      if (!m_ipcTool->putObject(contName.c_str(), contName.size() + 1, num).isSuccess()) {
         return(StatusCode::FAILURE);
      }
   }
   for (SG::auxid_t auxid : auxIDs) {
      const std::type_info* typePtr = store->getIOType(auxid);
      if (typePtr == nullptr) continue;
      const std::string& dataStr = SG::AuxTypeRegistry::instance().getName(auxid) + "\n" + SG::normalizedTypeinfoName(*typePtr) + "\n" + SG::AuxTypeRegistry::instance().getTypeName(auxid);
      if (!m_ipcTool->putObject(dataStr.c_str(), dataStr.size() + 1, num).isSuccess()) {
         return(StatusCode::FAILURE);
      }
      const std::type_info* tip = store->getIOType(auxid);
      if (tip == nullptr) {
         return(StatusCode::FAILURE);
      }
      RootType type(*tip);
      StatusCode sc = StatusCode::FAILURE;
      if (type.IsFundamental()) {
         sc = m_ipcTool->putObject(store->getIOData(auxid), type.SizeOf(), num);
      } else {
         size_t nbytes = 0;
         void* buffer = m_serSvc->serialize(store->getIOData(auxid), type, nbytes);
         sc = m_ipcTool->putObject(buffer, nbytes, num);
         delete [] static_cast<char*>(buffer); buffer = nullptr;
      }
      if (!sc.isSuccess()) {
         return(StatusCode::FAILURE);
      }
   }
   return(StatusCode::SUCCESS);
}
