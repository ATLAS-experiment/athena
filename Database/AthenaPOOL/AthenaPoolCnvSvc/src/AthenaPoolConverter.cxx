/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolConverter.cxx
 *  @brief This file contains the implementation for the AthenaPoolConverter base class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc/AthenaPoolConverter.h"
#include "AthenaPoolCnvSvc/IAthenaPoolCnvSvc.h"

#include "SGTools/DataProxy.h"

#include "PersistentDataModel/Guid.h"
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"
#include "StorageSvc/DbType.h"
#include "StorageSvc/APRDefaults.h"

#include <format>

//__________________________________________________________________________
AthenaPoolConverter::~AthenaPoolConverter() {
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::initialize() {
   ATH_CHECK(::Converter::initialize());

   // We do not retrieve m_detStore as that store may not always be available!

   // Retrieve AthenaPoolCnvSvc
   ATH_CHECK( m_athenaPoolCnvSvc.retrieve() );

   // Retrieve PoolSvc
   ATH_CHECK(m_poolSvc.retrieve());
   StringProperty defContainerType("DefaultContainerType", "ROOTTREEINDEX");
   if(IProperty* propertyServer = dynamic_cast<IProperty*>(m_poolSvc.get())) {
      propertyServer->getProperty(&defContainerType).ignore();
   }
   m_defContainerType = pool::DbType::getType(defContainerType).type();

   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::finalize() {
   // Release AthenaPoolCnvSvc
   if (!m_athenaPoolCnvSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release AthenaPoolCnvSvc.");
   }
   return(::Converter::finalize());
}
//__________________________________________________________________________
long AthenaPoolConverter::repSvcType() const {
   return pool::POOL_StorageType.type();
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::createObj(IOpaqueAddress* pAddr, DataObject*& pObj) {
   TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(pAddr);
   
   bool ownTokAddr = false;
   if (tokAddr == nullptr || tokAddr->getToken() == nullptr) {
      ownTokAddr = true;
      auto token = std::make_unique<Token>();
      token->fromString(*(pAddr->par()));
      GenericAddress* genAddr = dynamic_cast<GenericAddress*>(pAddr);
      if (not genAddr){
        ATH_MSG_ERROR("Dynamic cast failed in AthenaPoolConverter::createObj");
        //clean up
        return StatusCode::FAILURE;
      }
      tokAddr = new TokenAddress(*genAddr, std::move(token));
   }
   if( tokAddr->ipar()[0] > 0 and tokAddr->getToken()->auxString().empty() ) {
      char text[32];
      const std::string contextStr = std::format("[CTXT={:08X}]", static_cast<int>(*(pAddr->ipar())));
      std::strncpy(text, contextStr.c_str(), sizeof(text) - 1);
      text[sizeof(text) - 1] = '\0';
      tokAddr->getToken()->setAuxString(text);
   }
   ATH_MSG_VERBOSE("createObj: " << tokAddr->getToken()->toString() << ", CTX=" << tokAddr->ipar()[0]
                   << ", auxStr=" << tokAddr->getToken()->auxString() );
   try {
      std::string key = pAddr->par()[1];
      if (!PoolToDataObject(pObj, tokAddr->getToken(), key).isSuccess()) {
         ATH_MSG_ERROR("createObj PoolToDataObject() failed, Token = " << (tokAddr->getToken() ? tokAddr->getToken()->toString() : "NULL"));
         pObj = nullptr;
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("createObj - caught exception: " << e.what());
      pObj = nullptr;
   }
   if (pObj == nullptr) {
      ATH_MSG_ERROR("createObj failed to get DataObject, Token = " << (tokAddr->getToken() ? tokAddr->getToken()->toString() : "NULL"));
   }
   if (ownTokAddr) {
      delete tokAddr; tokAddr = nullptr;
   }
   if (pObj == nullptr) {
      return StatusCode::FAILURE;
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::createRep(DataObject* pObj, IOpaqueAddress*& pAddr) {
   const SG::DataProxy* proxy = dynamic_cast<SG::DataProxy*>(pObj->registry());
   if (proxy == nullptr) {
      ATH_MSG_ERROR("AthenaPoolConverter CreateRep failed to cast DataProxy, key = " << pObj->name());
      return StatusCode::FAILURE;
   }
   const CLID clid = proxy->clID();
   if (pAddr == nullptr) {
      // Create a IOpaqueAddress for this object.
      pAddr = new TokenAddress(this->storageType(), clid, "", "", 0, 0);
   } else {
      GenericAddress* gAddr = dynamic_cast<GenericAddress*>(pAddr);
      if (gAddr != nullptr) {
         gAddr->setSvcType(this->storageType());
      }
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::fillRepRefs(IOpaqueAddress* pAddr, DataObject* pObj) {
   try {
      if (!DataObjectToPool(pAddr, pObj).isSuccess()) {
         ATH_MSG_ERROR("FillRepRefs failed, key = " << pObj->name());
         return StatusCode::FAILURE;
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("fillRepRefs - caught exception: " << e.what());
      return StatusCode::FAILURE;
   }
   return StatusCode::SUCCESS;
}
//__________________________________________________________________________
long AthenaPoolConverter::storageType() {
   return pool::POOL_StorageType.type();
}
//__________________________________________________________________________
AthenaPoolConverter::AthenaPoolConverter(const CLID& myCLID, ISvcLocator* pSvcLocator,
                                         const char* name /*= nullptr*/) :
    ::Converter(storageType(), myCLID, pSvcLocator),
    ::AthMessaging((pSvcLocator != nullptr ? msgSvc() : nullptr),
                               name ? name : "AthenaPoolConverter"),
  m_detStore("DetectorStore", name ? name : "AthenaPoolConverter"),
  m_athenaPoolCnvSvc(pSvcLocator && pSvcLocator->existsService("AthenaPoolSharedIOCnvSvc") ? "AthenaPoolSharedIOCnvSvc" : "AthenaPoolCnvSvc", name ? name : "AthenaPoolConverter"),
  m_poolSvc("PoolSvc", name ? name : "AthenaPoolConverter"),
  m_defContainerType(0) {
}
//__________________________________________________________________________
Placement AthenaPoolConverter::setPlacementWithType(const std::string& tname, const std::string& key, const std::string& output) {
   // Resulting placement
   Placement placement;

   // Extract the file name and global technology (if available)
   std::string::size_type pos1 = output.find('[');
   std::string outputConnectionSpec = output.substr(0, pos1);
   placement.setFileName(outputConnectionSpec);

   // Override streaming parameters from StreamTool if requested.
   std::string containerPrefix{APRDefaults::WriteConfig::getEventDataName()};
   std::string dhContainerPrefix{APRDefaults::WriteConfig::getDataHeaderName()};
   std::string containerName{""};
   std::string containerNameHint{""};
   std::string branchNameHint{""};
   std::string containerFriendPostfix{""};
   while (pos1 != std::string::npos) {
      const std::string::size_type pos2 = output.find('=', pos1);
      const std::string thisKey = output.substr(pos1 + 1, pos2 - pos1 - 1);
      const std::string::size_type pos3 = output.find(']', pos2);
      const std::string value = output.substr(pos2 + 1, pos3 - pos2 - 1);
      if (thisKey == "OutputCollection") {
         dhContainerPrefix = std::move(value);
      } else if (thisKey == "PoolContainerPrefix") {
         containerPrefix = std::move(value);
      } else if (thisKey == "TopLevelContainerName") {
        containerNameHint = std::move(value);
      } else if (thisKey == "SubLevelBranchName") {
         branchNameHint = std::move(value);
      } else if (thisKey == "PoolContainerFriendPostfix") {
         containerFriendPostfix = std::move(value);
      }
      pos1 = output.find('[', pos3);
   }

   // Extract the technology from the container prefix (if available)
   int tech = m_defContainerType;
   if (auto colonPost = containerPrefix.find(':'); colonPost != std::string::npos) {
      tech = pool::DbType::getType(containerPrefix.substr(0, colonPost)).type();
      containerPrefix.erase(0, colonPost + 1); // Note that DataHeader and EventTag bypass this...
   }

   // ---  Special types:   DataHeader & Form
   if ( tname.starts_with(APRDefaults::DataHeaderTypeName) ) {
      containerName = std::format("{}{}({}{})",
         dhContainerPrefix,
         tname.starts_with(APRDefaults::DataHeaderFormTypeName) ? "Form" : "",
         key.back() == '/' ? key : "",
         tname);
   }
   // AttributeList - writing attributes separately to EventTag container group
   else if ( tname.starts_with(APRDefaults::EventTagTypeName) ) {
      containerName = std::format("{}({})",
         APRDefaults::WriteConfig::getEventTagName(),
         key);
   }
   // all other object types
   else {
      constexpr std::string_view typeTok = "<type>", keyTok = "<key>";
      containerName = std::format("{}{}{}{}",
                           containerPrefix,
                           containerFriendPostfix,
                           containerNameHint,
                           branchNameHint.empty() ? "" : std::format("({})", branchNameHint));
      if (auto pos = containerName.find(typeTok); pos != std::string::npos) {
         containerName.replace(pos, typeTok.size(), tname);
      }
      if (auto pos = containerName.find(keyTok); pos != std::string::npos) {
         containerName.replace(pos, keyTok.size(), key.empty() ? tname : key);
      }
   }

   // Set the container name and technology
   placement.setContainerName(containerName);
   placement.setTechnology(tech);
   return(placement);
}
//__________________________________________________________________________
bool AthenaPoolConverter::compareClassGuid(const Token* token, const Guid &guid) const {
   return(token ? (guid == token->classID()) : false);
}
//__________________________________________________________________________
StatusCode AthenaPoolConverter::cleanUp(const std::string& /*output*/) {
   ATH_MSG_DEBUG("AthenaPoolConverter cleanUp called for base class.");
   return StatusCode::SUCCESS;
}
