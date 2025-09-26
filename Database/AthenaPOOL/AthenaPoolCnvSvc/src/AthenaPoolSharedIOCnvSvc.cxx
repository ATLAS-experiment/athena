/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolSharedIOCnvSvc.cxx
 *  @brief This file contains the implementation for the AthenaPoolSharedIOCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolSharedIOCnvSvc.h"

#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/GenericAddress.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "AthenaKernel/IAthMetaDataSvc.h"
#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PersistentDataModel/DataHeader.h"


#include "StorageSvc/DbReflex.h"
#include "FileCatalog/IFileCatalog.h"

#include "AuxDiscoverySvc.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

//______________________________________________________________________________
// Initialize the service.
StatusCode AthenaPoolSharedIOCnvSvc::initialize() {
   // Retrieve InputStreamingTool (if configured)
   if (!m_inputStreamingTool.empty()) {
      ATH_CHECK(m_inputStreamingTool.retrieve());
   }
   // Retrieve OutputStreamingTool (if configured)
   if (!m_outputStreamingTool.empty()) {
      ATH_CHECK(m_outputStreamingTool.retrieve());
      if (m_makeStreamingToolClient.value() == -1) {
        // Initialize AthenaRootSharedWriter
        ServiceHandle<IService> arswsvc("AthenaRootSharedWriterSvc", this->name());
        ATH_CHECK(arswsvc.retrieve());
      }
      // Put PoolSvc into share mode to avoid duplicating catalog.
      getPoolSvc()->setShareMode(true);
   }
   if (!m_inputStreamingTool.empty() || !m_outputStreamingTool.empty()) {
      // Retrieve AthenaSerializeSvc
      ATH_CHECK(m_serializeSvc.retrieve());
   }
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
   long int pri = 1000;
   if (!m_outputStreamingTool.empty()) {
      incSvc->addListener(this, "StoreCleared", pri);
      ATH_MSG_DEBUG("Subscribed to StoreCleared");
   }
   return this->AthenaPoolCnvSvc::initialize();
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::finalize() {
   // Release AthenaSerializeSvc
   if (!m_serializeSvc.empty()) {
      if (!m_serializeSvc.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release AthenaSerializeSvc.");
      }
   }
   // Release OutputStreamingTool (if configured)
   if (!m_outputStreamingTool.empty()) {
      if (!m_outputStreamingTool.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release Output AthenaIPCTool.");
      }
   }
   // Release InputStreamingTool (if configured)
   if (!m_inputStreamingTool.empty()) {
      if (!m_inputStreamingTool.release().isSuccess()) {
         ATH_MSG_WARNING("Cannot release Input AthenaIPCTool.");
      }
   }
   return this->AthenaPoolCnvSvc::finalize();
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::connectOutput(const std::string& outputConnectionSpec,
               const std::string& /*openMode*/) {
   return(connectOutput(outputConnectionSpec));
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::connectOutput(const std::string& outputConnectionSpec) {
// This is called before DataObjects are being converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   if (m_makeStreamingToolClient.value() > 0 && !m_outputStreamingTool.empty() && !m_outputStreamingTool->isServer() && !m_outputStreamingTool->isClient()) {
      if (!makeClient(m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolSharedIOCnvSvc a Share Client");
         return(StatusCode::FAILURE);
      }
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient()) {
      if (m_parallelCompression && outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") == std::string::npos) {
         ATH_MSG_DEBUG("connectOutput SKIPPED for metadata-only server: " << outputConnectionSpec);
         return(StatusCode::SUCCESS);
      }
      if (!m_parallelCompression && (!m_outputStreamingTool->isServer() || !m_streamServerActive)) {
         ATH_MSG_DEBUG("connectOutput SKIPPED for expired server.");
         return(StatusCode::SUCCESS);
      }
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient() && m_parallelCompression) {
      outputConnection += m_streamPortString.value();
   }
   std::size_t apend = outputConnectionSpec.find('[');
   if (apend != std::string::npos) {
      outputConnection += outputConnectionSpec.substr(apend);
   }
   return AthenaPoolCnvSvc::connectOutput(outputConnection);
}

//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::commitOutput(const std::string& outputConnectionSpec, bool doCommit) {
   // This is called after all DataObjects are converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      m_outputStreamingTool->lockObject("wait").ignore();
      if (!this->cleanUp(outputConnection).isSuccess()) {
         ATH_MSG_ERROR("commitOutput FAILED to cleanup converters.");
         return(StatusCode::FAILURE);
      }
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient() && !m_outputStreamingTool->isServer()) {
      ATH_MSG_DEBUG("commitOutput SKIPPED for uninitialized server.");
      return(StatusCode::SUCCESS);
   }
   std::map<void*, RootType> commitCache;
   std::string fileName;
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient() && m_outputStreamingTool->isServer() && m_streamServerActive) {
      // Clear object to get Placements for all objects in a Stream
      const char* placementStr = nullptr;
      int num = -1;
      StatusCode sc = m_outputStreamingTool->clearObject(&placementStr, num);
      if (sc.isSuccess() && placementStr != nullptr && strlen(placementStr) > 6 && num > 0) {
         const char * matchedChars = strstr(placementStr, "[FILE=");
         if (not matchedChars){
           ATH_MSG_ERROR("No matching filename in  " << placementStr);
           return abortSharedWrClients(num);
         }
         fileName = matchedChars;
         fileName = fileName.substr(6, fileName.find(']') - 6);
         if (!this->connectOutput(fileName).isSuccess()) {
            ATH_MSG_ERROR("Failed to connectOutput for " << fileName);
            return abortSharedWrClients(num);
         }
         IConverter* DHcnv = converter(ClassID_traits<DataHeader>::ID());
         bool dataHeaderSeen = false;
         std::string dataHeaderID;
         while (num > 0) {
            std::string objName = "ALL";
            if (useDetailChronoStat()) {
               objName = placementStr; //FIXME, better descriptor
            }
            // StopWatch listens from here until the end of this current scope
            {
               PMonUtils::BasicStopWatch stopWatch("cRep_" + objName, this->m_chronoMap);
               std::string_view pStr = placementStr;
               std::string::size_type cpos = pStr.find ("[CONT=");
               if (cpos == std::string::npos) {
                 ATH_MSG_ERROR("No CONT field in placement string: " << pStr);
                 return StatusCode::FAILURE;
               }
               std::string tokenStr (pStr.substr(0, cpos));
               std::string contName (pStr.substr(cpos, std::string::npos));
               std::string::size_type cl1 = contName.find(']');
               if (cl1 == std::string::npos) {
                 ATH_MSG_ERROR("Missing close bracket after CONT field in placement string: " << pStr);
                 return StatusCode::FAILURE;
               }
               tokenStr.append(contName, cl1 + 1);
               contName = contName.substr(6, cl1 - 6);

               std::string::size_type ppos = pStr.find ("[PNAME=");
               if (ppos == std::string::npos) {
                 ATH_MSG_ERROR("No PNAME field in placement string: " << pStr);
                 return StatusCode::FAILURE;
               }
               std::string className (pStr.substr(ppos, std::string::npos));
               std::string::size_type cl2 = className.find(']');
               if (cl2 == std::string::npos) {
                 ATH_MSG_ERROR("Missing close bracket after PNAME field in placement string: " << pStr);
                 return StatusCode::FAILURE;
               }
               className = className.substr(7, cl2 - 7);
               RootType classDesc = RootType::ByNameNoQuiet(className);
               void* obj = nullptr;
               std::ostringstream oss2;
               oss2 << std::dec << num;
               std::string::size_type len = m_metadataContainerProp.value().size();
               bool foundContainer = false;
               std::size_t opPos = contName.find('(');
               if (contName.compare(0, opPos, m_metadataContainerProp.value()) == 0) {
                  foundContainer = true;
               } else {
                  for (const auto& item: m_metadataContainersAug.value()) {
                     if (contName.compare(0, opPos, item) == 0){
                        foundContainer = true;
                        len = item.size();
                        break;
                     }
                  }
               }
               if (len > 0 && foundContainer && contName[len] == '(' ) {
                  ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
                  // For Metadata, before moving to next client, fire file incidents
                  if (m_metadataClient != num) {
                     if (m_metadataClient != 0) {
                        std::ostringstream oss1;
                        oss1 << std::dec << m_metadataClient;
                        std::string memName = "SHM[NUM=" + oss1.str() + "]";
                        FileIncident beginInputIncident(name(), "BeginInputMemFile", memName);
                        incSvc->fireIncident(beginInputIncident);
                        FileIncident endInputIncident(name(), "EndInputMemFile", std::move(memName));
                        incSvc->fireIncident(endInputIncident);
                     }
                     m_metadataClient = num;
                  }
                  // Retrieve MetaDataSvc
                  ServiceHandle<IAthMetaDataSvc> metadataSvc("MetaDataSvc", name());
                  ATH_CHECK(metadataSvc.retrieve());
                  sc = metadataSvc->shmProxy(std::string(pStr) + "[NUM=" + oss2.str() + "]");
                  if (sc.isRecoverable()) {
                     ATH_MSG_WARNING("MetaDataSvc::shmProxy() no proxy added.");
                  } else if (sc.isFailure()) {
                     ATH_MSG_FATAL("MetaDataSvc::shmProxy() failed!");
                     return abortSharedWrClients(num);
                  }
               } else {
                  Token readToken;
                  readToken.setOid(Token::OID_t(num, 0));
                  readToken.setAuxString("[PNAME=" + className + "]");
                  this->setObjPtr(obj, &readToken); // Pull/read Object out of shared memory
                  if (len == 0 || contName.compare(0, len, m_metadataContainerProp.value()) != 0) {
                     // Write object
                     if( m_oneDataHeaderForm.value() ) {
                        auto placementWithSwn = [&] { return std::format("{}[SWN={}]",  placementStr, num); };
                        if( className == "DataHeaderForm_p6" ) {
                           // Pass DHForms to the converter for later writing in the correct order - do not write it now
                           GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                                  "", placementWithSwn());
                           DHcnv->updateRepRefs(&address, static_cast<DataObject*>(obj)).ignore();
                           tokenStr = "";
                        } else {
                           Placement placement;
                           placement.fromString(placementStr);
                           std::unique_ptr<Token> token(registerForWrite(&placement, obj, classDesc));
                           if (token == nullptr) {
                              ATH_MSG_ERROR("Failed to write Data for: " << className);
                              return abortSharedWrClients(num);
                           }
                           tokenStr = token->toString();
                        }
                        if( className == "DataHeader_p6" ) {
                           // Found DataHeader - call the converter to update DHForm Ref
                           GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                                  tokenStr, placementWithSwn());
                           if (!DHcnv->updateRep(&address, static_cast<DataObject*>(obj)).isSuccess()) {
                              ATH_MSG_ERROR("Failed updateRep for obj = " << tokenStr);
                              return abortSharedWrClients(num);
                           }
                        } else 
                        if (className != "Token" && className != "DataHeaderForm_p6" && !classDesc.IsFundamental()) {
                           commitCache.insert(std::pair<void*, RootType>(obj, classDesc));
                        }
                        placementStr = nullptr;
                     } else { 
                        // Multiple shared DataHeaderForms
                        Placement placement;
                        placement.fromString(placementStr); placementStr = nullptr;
                        std::unique_ptr<Token> token(registerForWrite(&placement, obj, classDesc));
                        if (token == nullptr) {
                           ATH_MSG_ERROR("Failed to write Data for: " << className);
                           return abortSharedWrClients(num);
                        }
                        tokenStr = token->toString();
                        if (className == "DataHeader_p6") {
                           // Found DataHeader
                           GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                                  tokenStr, placement.auxString());
                           // call DH converter to add the ref to DHForm (stored earlier) and to itself
                           if (!DHcnv->updateRep(&address, static_cast<DataObject*>(obj)).isSuccess()) {
                              ATH_MSG_ERROR("Failed updateRep for obj = " << tokenStr);
                              return abortSharedWrClients(num);
                           }
                           dataHeaderSeen = true;
                           // This dataHeaderID is used in DataHeaderCnv to index the DataHeaderForm cache.
                           // It must be unique per worker per stream so that we have a correct DataHeader(Form) association.
                           // This is achieved by building it as "CONTID/WORKERID/DBID".
                           // CONTID, e.g., POOLContainer(DataHeader), allows us to distinguish data and metadata headers,
                           // WORKERID allows us to distinguish AthenaMP workers,
                           // and DBID allows us to distinguish streams.
                           dataHeaderID = std::format("{}/{}/{}", token->contID(), oss2.str(), token->dbID().toString());
                        } else if (dataHeaderSeen) {
                           dataHeaderSeen = false;
                           // next object after DataHeader - may be a DataHeaderForm
                           // in any case we need to call the DH converter to update the DHForm Ref
                           if (className == "DataHeaderForm_p6") {
                              // Tell DataHeaderCnv that it should use a new DHForm
                              GenericAddress address(POOL_StorageType, ClassID_traits<DataHeader>::ID(),
                                                     tokenStr, dataHeaderID);
                              if (!DHcnv->updateRepRefs(&address, static_cast<DataObject*>(obj)).isSuccess()) {
                                 ATH_MSG_ERROR("Failed updateRepRefs for obj = " << tokenStr);
                                 return abortSharedWrClients(num);
                              }
                           } else {
                              // Tell DataHeaderCnv that it should use the old DHForm
                              GenericAddress address(0, 0, "", dataHeaderID);
                              if (!DHcnv->updateRepRefs(&address, nullptr).isSuccess()) {
                                 ATH_MSG_ERROR("Failed updateRepRefs for DataHeader");
                                 return abortSharedWrClients(num);
                              }
                           }
                        }
                        if (className != "Token" && className != "DataHeaderForm_p6" && !classDesc.IsFundamental()) {
                           commitCache.insert(std::pair<void*, RootType>(obj, classDesc));
                        }
                     }
                  }
               }
               // Send Token back to Client
               sc = m_outputStreamingTool->lockObject(tokenStr.c_str(), num);
               while (sc.isRecoverable()) {
                  sc = m_outputStreamingTool->lockObject(tokenStr.c_str(), num);
               }
               if (!sc.isSuccess()) {
                  ATH_MSG_ERROR("Failed to lock Data for " << tokenStr);
                  return abortSharedWrClients(-1);
               }
            }
            sc = m_outputStreamingTool->clearObject(&placementStr, num);
            while (sc.isRecoverable()) {
               sc = m_outputStreamingTool->clearObject(&placementStr, num);
            }
            if (sc.isFailure()) {
               // no more clients, break the loop and exit
               num = -1;
            }
         }
         if (dataHeaderSeen) {
            // DataHeader was the last object, need to tell the converter there is no DHForm coming
            GenericAddress address(0, 0, "", std::move(dataHeaderID));
            if (!DHcnv->updateRepRefs(&address, nullptr).isSuccess()) {
               ATH_MSG_ERROR("Failed updateRepRefs for DataHeader");
               return abortSharedWrClients(-1);
            }
         }
         placementStr = nullptr;
      } else if (sc.isSuccess() && placementStr != nullptr && strncmp(placementStr, "stop", 4) == 0) {
         return(StatusCode::RECOVERABLE);
      } else if (sc.isRecoverable() || num == -1) {
         return(StatusCode::RECOVERABLE);
      }
      if (sc.isFailure() || fileName.empty()) {
         ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
         std::ostringstream oss1;
         oss1 << std::dec << m_metadataClient;
         std::string memName = "SHM[NUM=" + oss1.str() + "]";
         FileIncident beginInputIncident(name(), "BeginInputMemFile", memName);
         incSvc->fireIncident(beginInputIncident);
         FileIncident endInputIncident(name(), "EndInputMemFile", memName);
         incSvc->fireIncident(endInputIncident);
         if (sc.isFailure()) {
            ATH_MSG_INFO("All SharedWriter clients stopped - exiting");
         } else {
            ATH_MSG_INFO("Failed to get Data for client: " << num);
         }
         return(StatusCode::FAILURE);
      }
   }
   if (m_parallelCompression && !fileName.empty()) {
      ATH_MSG_DEBUG("commitOutput SKIPPED for metadata-only server: " << outputConnectionSpec);
      return(StatusCode::SUCCESS);
   }
   if (outputConnection.empty()) {
      outputConnection = std::move(fileName);
   } else {
      outputConnection = outputConnectionSpec;
      if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient() && m_parallelCompression) {
         outputConnection += m_streamPortString.value();
      }
   }
   StatusCode status = AthenaPoolCnvSvc::commitOutput(outputConnection, doCommit);
   for (std::map<void*, RootType>::iterator iter = commitCache.begin(), last = commitCache.end(); iter != last; ++iter) {
      iter->second.Destruct(iter->first);
   }
   return(status);
}

//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::disconnectOutput(const std::string& outputConnectionSpec) {
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient()
	   && (!m_parallelCompression || outputConnectionSpec.find("[PoolContainerPrefix=" + m_metadataContainerProp.value() + "]") != std::string::npos)) {
      return(StatusCode::SUCCESS);
   }
   if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient()) {
      if (m_streamServerActive) {
         m_streamServerActive = false;
         ATH_MSG_DEBUG("disconnectOutput SKIPPED to expire server.");
         return(StatusCode::SUCCESS);
      } else {
         m_streamServerActive = false;
      }
      ATH_MSG_DEBUG("disconnectOutput not SKIPPED for server.");
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient() && m_parallelCompression) {
      outputConnection += m_streamPortString.value();
   }
   return AthenaPoolCnvSvc::disconnectOutput(outputConnectionSpec + m_streamPortString.value());
}

//______________________________________________________________________________
Token* AthenaPoolSharedIOCnvSvc::registerForWrite(Placement* placement, const void* obj, const RootType& classDesc) {
   if (m_makeStreamingToolClient.value() > 0 && !m_outputStreamingTool.empty() && !m_outputStreamingTool->isServer() && !m_outputStreamingTool->isClient()) {
      if (!makeClient(m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolSharedIOCnvSvc a Share Client");
         return(nullptr);
      }
   }
   Token* token = nullptr;
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient()
	   && (!m_parallelCompression || placement->containerName().compare(0, m_metadataContainerProp.value().size(), m_metadataContainerProp.value()) == 0)) {
      // Lock object
      std::string placementStr = placement->toString();
      placementStr += "[PNAME=";
      placementStr += classDesc.Name();
      placementStr += ']';
      ATH_MSG_VERBOSE("Requesting write object for: " << placementStr);
      StatusCode sc = m_outputStreamingTool->lockObject(placementStr.c_str());
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool->lockObject(placementStr.c_str());
      }
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Failed to lock Data for " << placementStr);
         return(nullptr);
      }
      // Serialize object via ROOT
      const void* buffer = nullptr;
      std::size_t nbytes = 0;
      bool own = true;
      if (classDesc.Name() == "Token") {
         nbytes = strlen(static_cast<const char*>(obj)) + 1;
         buffer = obj;
         own = false;
      } else if (classDesc.IsFundamental()) {
         nbytes = classDesc.SizeOf();
         buffer = obj;
         own = false;
      } else {
         buffer = m_serializeSvc->serialize(obj, classDesc, nbytes);
      }
      // Share object
      sc = m_outputStreamingTool->putObject(buffer, nbytes);
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool->putObject(buffer, nbytes);
      }
      if (own) { delete [] static_cast<const char*>(buffer); }
      buffer = nullptr;
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << placementStr);
         m_outputStreamingTool->putObject(nullptr, 0).ignore();
         return(nullptr);
      }
      AuxDiscoverySvc auxDiscover;
      if (!auxDiscover.sendStore(m_serializeSvc.get(), m_outputStreamingTool.get(), obj, pool::DbReflex::guid(classDesc), placement->containerName()).isSuccess()) {
         ATH_MSG_ERROR("Could not share dynamic aux store for: " << placementStr);
         m_outputStreamingTool->putObject(nullptr, 0).ignore();
         return(nullptr);
      }
      if (!m_outputStreamingTool->putObject(nullptr, 0).isSuccess()) {
         ATH_MSG_ERROR("Failed to put Data for " << placementStr);
         return(nullptr);
      }
      // Get Token back from Server
      const char* tokenStr = nullptr;
      int num = -1;
      sc = m_outputStreamingTool->clearObject(&tokenStr, num);
      while (sc.isRecoverable()) {
         //usleep(100);
         sc = m_outputStreamingTool->clearObject(&tokenStr, num);
      }
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Failed to get Token");
         return(nullptr);
      }
      if (!strcmp(tokenStr, "ABORT")) {
         ATH_MSG_ERROR("Writer requested ABORT");
         // tell the server we are leaving
         m_outputStreamingTool->stop().ignore();
         return nullptr;
      }
      Token* tempToken = new Token();
      tempToken->fromString(tokenStr); tokenStr = nullptr;
      tempToken->setClassID(pool::DbReflex::guid(classDesc));
      token = tempToken; tempToken = nullptr;
// Client Write Request
   } else {
      if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient() && !m_outputStreamingTool->isServer()) {
         ATH_MSG_DEBUG("registerForWrite SKIPPED for uninitialized server, Placement = " << placement->toString());
         Token* tempToken = new Token();
         tempToken->setClassID(pool::DbReflex::guid(classDesc));
         token = tempToken; tempToken = nullptr;
      } else if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isClient() && !m_streamServerActive) {
         ATH_MSG_DEBUG("Requested write object for: " << placement->toString());
         token = getPoolSvc()->registerForWrite(placement, obj, classDesc);
      } else {
         if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isClient() && m_parallelCompression) {
            placement->setFileName(placement->fileName() + m_streamPortString.value());
         }
	 token = AthenaPoolCnvSvc::registerForWrite(placement, obj, classDesc);
      }
   }
   return(token);
}
//______________________________________________________________________________
void AthenaPoolSharedIOCnvSvc::setObjPtr(void*& obj, const Token* token) {
   if (m_makeStreamingToolClient.value() > 0 && !m_inputStreamingTool.empty() && !m_inputStreamingTool->isServer() && !m_inputStreamingTool->isClient()) {
      if (!makeClient(-m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolSharedIOCnvSvc a Share Client");
      }
   }
   if (!m_outputStreamingTool.empty() && m_outputStreamingTool->isServer()) {
      if (token->dbID() == Guid::null()) {
         int num = token->oid().first;
         // Get object from SHM
         void* buffer = nullptr;
         std::size_t nbytes = 0;
         StatusCode sc = m_outputStreamingTool->getObject(&buffer, nbytes, num);
         while (sc.isRecoverable()) {
            //usleep(100);
            sc = m_outputStreamingTool->getObject(&buffer, nbytes, num);
         }
         if (!sc.isSuccess()) {
            ATH_MSG_ERROR("Failed to get Data for " << token->toString());
            obj = nullptr;
         } else {
            ATH_MSG_DEBUG("Server deserializing " << token->toString());
            if (token->classID() != Guid::null()) {
               // Deserialize object
               RootType cltype(pool::DbReflex::forGuid(token->classID()));
               obj = m_serializeSvc->deserialize(buffer, nbytes, cltype); buffer = nullptr;
            } else {
               // Deserialize object
               std::string className = token->auxString();
               className = className.substr(className.find("[PNAME="));
               className = className.substr(7, className.find(']') - 7);
               RootType cltype(RootType::ByNameNoQuiet(className));
               obj = m_serializeSvc->deserialize(buffer, nbytes, cltype); buffer = nullptr;
            }
            AuxDiscoverySvc auxDiscover;
            if (!auxDiscover.receiveStore(m_serializeSvc.get(), m_outputStreamingTool.get(), obj, num).isSuccess()) {
               ATH_MSG_ERROR("Failed to get Dynamic Aux Store for " << token->toString());
               obj = nullptr;
            }
         }
      }
   }
   if (!m_inputStreamingTool.empty() && m_inputStreamingTool->isClient() && (m_streamingTechnology.value() < 0 || token->technology() == m_streamingTechnology.value())) {
      ATH_MSG_VERBOSE("Requesting remote object for: " << token->toString());
      if (!m_inputStreamingTool->lockObject(token->toString().c_str()).isSuccess()) {
         ATH_MSG_ERROR("Failed to lock Data for " << token->toString());
         obj = nullptr;
      } else {
         void* buffer = nullptr;
         std::size_t nbytes = 0;
         StatusCode sc = StatusCode::FAILURE;
         // StopWatch listens from here until the end of this current scope
         {
            PMonUtils::BasicStopWatch stopWatch("gObj_ALL", this->m_chronoMap);
            sc = m_inputStreamingTool->getObject(&buffer, nbytes);
            while (sc.isRecoverable()) {
               // sleep
               sc = m_inputStreamingTool->getObject(&buffer, nbytes);
            }
         }
         if (!sc.isSuccess()) {
            ATH_MSG_ERROR("Failed to get Data for " << token->toString());
            obj = nullptr;
         } else {
            obj = m_serializeSvc->deserialize(buffer, nbytes, token->classID()); buffer = nullptr;
            AuxDiscoverySvc auxDiscover;
            if (!auxDiscover.receiveStore(m_serializeSvc.get(), m_inputStreamingTool.get(), obj).isSuccess()) {
               ATH_MSG_ERROR("Failed to get Dynamic Aux Store for " << token->toString());
               obj = nullptr;
            }
         }
      }
   } else if (token->dbID() != Guid::null()) {
      AthenaPoolCnvSvc::setObjPtr(obj, token);
   }
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string* par,
		const unsigned long* ip,
		IOpaqueAddress*& refpAddress) {
   if (m_makeStreamingToolClient.value() > 0 && !m_inputStreamingTool.empty() && !m_inputStreamingTool->isServer() && !m_inputStreamingTool->isClient()) {
      if (!makeClient(-m_makeStreamingToolClient.value()).isSuccess()) {
         ATH_MSG_ERROR("Could not make AthenaPoolSharedIOCnvSvc a Share Client");
         return(StatusCode::FAILURE);
      }
   }
   if (!m_inputStreamingTool.empty() && m_inputStreamingTool->isClient()) {
      Token addressToken;
      addressToken.setDb(par[0].substr(4));
      addressToken.setCont(par[1]);
      addressToken.setOid(Token::OID_t(ip[0], ip[1]));
      if (!m_inputStreamingTool->lockObject(addressToken.toString().c_str()).isSuccess()) {
         ATH_MSG_WARNING("Failed to lock Address Token: " << addressToken.toString());
         return(StatusCode::FAILURE);
      }
      void* buffer = nullptr;
      std::size_t nbytes = 0;
      StatusCode sc = m_inputStreamingTool->getObject(&buffer, nbytes);
      while (sc.isRecoverable()) {
         // sleep
         sc = m_inputStreamingTool->getObject(&buffer, nbytes);
      }
      if (!sc.isSuccess()) {
         ATH_MSG_WARNING("Failed to get Address Token: " << addressToken.toString());
         return(StatusCode::FAILURE);
      }
      auto token = std::make_unique<Token>();
      token->fromString(static_cast<const char*>(buffer)); buffer = nullptr;
      if (token->classID() == Guid::null()) {
         token.reset();
      }
      m_inputStreamingTool->getObject(&buffer, nbytes).ignore();
      if (token) {
         refpAddress = new TokenAddress(POOL_StorageType, clid, "", par[1], IPoolSvc::kInputStream, std::move(token));
         return(StatusCode::SUCCESS);
      }
      else {
         return(StatusCode::RECOVERABLE);
      }
   } else {
      return AthenaPoolCnvSvc::createAddress(svcType, clid, par, ip, refpAddress);
   }
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string& refAddress,
		IOpaqueAddress*& refpAddress) {
   return AthenaPoolCnvSvc::createAddress(svcType, clid, refAddress, refpAddress);
}
//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::makeServer(int num) {
   if (num < 0) {
      num = -num;
      m_streamServerActive = true;
      num = num % 1024;
      if (!m_outputStreamingTool.empty() && !m_outputStreamingTool->isServer()) {
         ATH_MSG_DEBUG("makeServer: " << m_outputStreamingTool << " = " << num);
         ATH_MSG_DEBUG("makeServer: Calling shared memory tool with port suffix " << m_streamPortString);
         const std::string streamPortSuffix = m_streamPortString.value();
         if (m_outputStreamingTool->makeServer(num, streamPortSuffix).isFailure()) {
            ATH_MSG_ERROR("makeServer: " << m_outputStreamingTool << " failed");
            return(StatusCode::FAILURE);
         }
         // Disable PersistencySvc per output file mode, for SharedWriter Server
         m_persSvcPerOutput.setValue(false);
         return(StatusCode::SUCCESS);
      }
      return(StatusCode::RECOVERABLE);
   }
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::RECOVERABLE);
   }
   ATH_MSG_DEBUG("makeServer: " << m_inputStreamingTool << " = " << num);
   return(m_inputStreamingTool->makeServer(num, ""));
}
//________________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::makeClient(int num) {
   if (!m_outputStreamingTool.empty()) {
      ATH_MSG_DEBUG("makeClient: " << m_outputStreamingTool << " = " << num);
      std::string streamPortSuffix;
      if (m_outputStreamingTool->makeClient(num, streamPortSuffix).isFailure()) {
         ATH_MSG_ERROR("makeClient: " << m_outputStreamingTool << " failed");
         return(StatusCode::FAILURE);
      } else if (m_streamPortString.value().find("localhost:0") != std::string::npos) {
         // We don't seem to use a dedicated port per stream so doing this for the first client is probably OK
         ATH_MSG_DEBUG("makeClient: Setting conversion service port suffix to " << streamPortSuffix);
         m_streamPortString.setValue(streamPortSuffix);
      }
   }
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::SUCCESS);
   }
   ATH_MSG_DEBUG("makeClient: " << m_inputStreamingTool << " = " << num);
   std::string dummyStr;
   return(m_inputStreamingTool->makeClient(num, dummyStr));
}
//________________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::readData() {
   if (m_inputStreamingTool.empty()) {
      return(StatusCode::FAILURE);
   }
   const char* tokenStr = nullptr;
   int num = -1;
   StatusCode sc = m_inputStreamingTool->clearObject(&tokenStr, num);
   if (sc.isSuccess() && tokenStr != nullptr && strlen(tokenStr) > 0 && num > 0) {
      ATH_MSG_DEBUG("readData: " << tokenStr << ", for client: " << num);
   } else {
      return(sc);
   }
   // Read object instance via POOL/ROOT
   void* instance = nullptr;
   Token token;
   token.fromString(tokenStr); tokenStr = nullptr;
   if (token.classID() != Guid::null()) {
      std::string objName = "ALL";
      if (useDetailChronoStat()) {
         objName = token.classID().toString();
      }
      // StopWatch listens from here until the end of this current scope
      PMonUtils::BasicStopWatch stopWatch("cObj_" + objName, this->m_chronoMap);
      this->setObjPtr(instance, &token);
      // Serialize object via ROOT
      RootType cltype(pool::DbReflex::forGuid(token.classID()));
      void* buffer = nullptr;
      std::size_t nbytes = 0;
      buffer = m_serializeSvc->serialize(instance, cltype, nbytes);
      sc = m_inputStreamingTool->putObject(buffer, nbytes, num);
      while (sc.isRecoverable()) {
         sc = m_inputStreamingTool->putObject(buffer, nbytes, num);
      }
      delete [] static_cast<char*>(buffer); buffer = nullptr;
      if (!sc.isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << token.toString());
         return(StatusCode::FAILURE);
      }
      AuxDiscoverySvc auxDiscover;
      if (!auxDiscover.sendStore(m_serializeSvc.get(), m_inputStreamingTool.get(), instance, token.classID(), token.contID(), num).isSuccess()) {
         ATH_MSG_ERROR("Could not share dynamic aux store for: " << token.toString());
         return(StatusCode::FAILURE);
      }
      cltype.Destruct(instance); instance = nullptr;
      if (!m_inputStreamingTool->putObject(nullptr, 0, num).isSuccess()) {
         ATH_MSG_ERROR("Could not share object for: " << token.toString());
         return(StatusCode::FAILURE);
      }
   } else if (token.dbID() != Guid::null()) {
      std::string returnToken;
      const Token* metadataToken = getPoolSvc()->getToken("FID:" + token.dbID().toString(), token.contID(), token.oid().first);
      if (metadataToken != nullptr) {
         returnToken = metadataToken->toString();
      } else {
         returnToken = token.toString();
      }
      delete metadataToken; metadataToken = nullptr;
      // Share token
      sc = m_inputStreamingTool->putObject(returnToken.c_str(), returnToken.size() + 1, num);
      if (!sc.isSuccess() || !m_inputStreamingTool->putObject(nullptr, 0, num).isSuccess()) {
         ATH_MSG_ERROR("Could not share token for: " << token.toString());
         return(StatusCode::FAILURE);
      }
   } else {
      return(StatusCode::RECOVERABLE);
   }
   return(StatusCode::SUCCESS);
}

//________________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::commitCatalog() {
   pool::IFileCatalog* catalog ATLAS_THREAD_SAFE =  // This is on the SharedWriter, after mother process finishes events
	   const_cast<pool::IFileCatalog*>(getPoolSvc()->catalog());
   catalog->commit();
   catalog->start();
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
StatusCode AthenaPoolSharedIOCnvSvc::abortSharedWrClients(int client_n)
{
   ATH_MSG_ERROR("Sending ABORT to clients");
   // the master process will kill this process once workers abort
   // but it could be a time-limited loop
   StatusCode sc = StatusCode::SUCCESS;
   while (sc.isSuccess()) {
      if (client_n >= 0) {
         sc = m_outputStreamingTool->lockObject("ABORT", client_n);
      }
      const char* dummy;
      sc = m_outputStreamingTool->clearObject(&dummy, client_n);
      while (sc.isRecoverable()) {
         sc = m_outputStreamingTool->clearObject(&dummy, client_n);
      }
   }
   return StatusCode::FAILURE;
}

//______________________________________________________________________________
void AthenaPoolSharedIOCnvSvc::handle(const Incident& incident) {
   if (incident.type() == "StoreCleared" && m_outputStreamingTool->isClient() && !m_parallelCompression) {
      m_outputStreamingTool->lockObject("release").ignore();
   }
}
//______________________________________________________________________________
AthenaPoolSharedIOCnvSvc::AthenaPoolSharedIOCnvSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator) {
}
