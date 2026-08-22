/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/** @file AthenaPoolCnvSvc.cxx
 *  @brief This file contains the implementation for the AthenaPoolCnvSvc class.
 *  @author Peter van Gemmeren <gemmeren@anl.gov>
 **/

#include "AthenaPoolCnvSvc.h"

#include "GaudiKernel/AttribStringParser.h"
#include "GaudiKernel/ClassID.h"
#include "GaudiKernel/FileIncident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IIoComponentMgr.h"
#include "GaudiKernel/IOpaqueAddress.h"

#include "PersistentDataModel/Placement.h"
#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/TokenAddress.h"
#include "PersistentDataModel/DataHeader.h"

#include "StorageSvc/DbType.h"
#include "StorageSvc/APRDefaults.h"

#include <algorithm>
#include <charconv>
#include <format>
#include <iomanip>
#include <sstream>
#include "CxxUtils/HexString.h"
//______________________________________________________________________________
// Initialize the service.
StatusCode AthenaPoolCnvSvc::initialize() {
   // Retrieve PoolSvc
   ATH_CHECK(m_poolSvc.retrieve());
   // Retrieve ClassIDSvc
   ATH_CHECK(m_clidSvc.retrieve());
   // Register this service for 'I/O' events
   ServiceHandle<IIoComponentMgr> iomgr("IoComponentMgr", name());
   ATH_CHECK(iomgr.retrieve());
   if (!iomgr->io_register(this).isSuccess()) {
      ATH_MSG_FATAL("Could not register myself with the IoComponentMgr !");
      return(StatusCode::FAILURE);
   }
   // Global POOL container naming scheme
   if (auto scheme = APRDefaults::WriteConfig::parseNamingScheme(m_containerNamingSchemeProp.value())) {
      APRDefaults::WriteConfig::setNamingScheme(*scheme);
   } else {
      ATH_MSG_ERROR(std::format("Invalid PoolContainerNamingScheme: {}, see APRDefaults.h for the full list.", m_containerNamingSchemeProp.value()));
      return StatusCode::FAILURE;
   }
   // Extracting INPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
   extractPoolAttributes(m_inputPoolAttr, &m_inputAttr, &m_inputAttr, &m_inputAttr);
   // Extracting the INPUT POOL ItechnologySpecificAttributes which are to be printed for each event
   extractPoolAttributes(m_inputPoolAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent);
   // Setup incident for ProcessEventAttributes to process attributes on each event
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
   long int pri = 1000;
   // Set to be listener for ProcessEventAttributes
   incSvc->addListener(this, "ProcessEventAttributes", pri);
   if (!processPoolAttributes(m_inputAttr, "", IPoolSvc::kInputStream, false, true, true).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed setting POOL domain attributes.");
   }

   // Load these dictionaries now, so we don't need to try to do so
   // while multiple threads are running.
   TClass::GetClass ("TLeafI");
   TClass::GetClass ("TLeafL");
   TClass::GetClass ("TLeafD");
   TClass::GetClass ("TLeafF");

   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::io_reinit() {
   ATH_MSG_DEBUG("I/O reinitialization...");
   m_processedContextIds.clear();
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
void AthenaPoolCnvSvc::flushDataHeaderForms(const std::string& streamName) {
   // Write remaining DataHeaderForms for a given streamName, "*"" means all
   auto DHCnvListener = dynamic_cast<IIncidentListener*>( converter( ClassID_traits<DataHeader>::ID() ) );
   FileIncident incident(name(), "WriteDataHeaderForms", streamName);
   if( DHCnvListener ) DHCnvListener->handle(incident);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::stop() {
   ATH_MSG_VERBOSE("stop()");
   // In case of direct writing without an OutputStream, this should be a good time to flush DHForms
   flushDataHeaderForms();
   return StatusCode::SUCCESS;
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::finalize() {
   ATH_MSG_VERBOSE("Finalizing...");
   // Some algorithms write in finalize(), flush DHForms if any are left
   flushDataHeaderForms();
   // Release ClassIDSvc
   if (!m_clidSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release ClassIDSvc.");
   }
   // Release PoolSvc
   if (!m_poolSvc.release().isSuccess()) {
      ATH_MSG_WARNING("Cannot release PoolSvc.");
   }
   // Print Performance Statistics
   // The pattern AthenaPoolCnvSvc.*PerfStats is ignored in AtlasTest/TestTools/share/post.sh
   const std::string msgPrefix{"PerfStats "};
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));
   ATH_MSG_INFO(msgPrefix << "Timing Measurements for AthenaPoolCnvSvc");
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));
   for(const auto& [key, value] : m_chronoMap) {
      ATH_MSG_INFO(msgPrefix << "| " << std::left << std::setw(15) << key << " | "
                        << std::right << std::setw(15) << std::fixed << std::setprecision(0) << value << " ms |");
   }
   ATH_MSG_INFO(msgPrefix << std::string(40, '-'));

   m_cnvs.clear();
   m_cnvs.shrink_to_fit();
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::io_finalize() {
   ATH_MSG_DEBUG("I/O finalization...");
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createObj(IOpaqueAddress* pAddress, DataObject*& refpObject) {
   assert(pAddress);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pAddress->clID(), objName).isFailure()) {
         objName = std::to_string(pAddress->clID());
      }
      objName += '#';
      objName += *(pAddress->par() + 1);
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cObj_" + objName, m_chronoMap);
   if (!m_persSvcPerInputType.value().empty()) { // Use separate PersistencySvc for each input data type
      TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(pAddress);
      if (tokAddr != nullptr && tokAddr->getToken() != nullptr && (tokAddr->getToken()->contID().starts_with(m_persSvcPerInputType.value() + "(") || tokAddr->getToken()->contID().starts_with(m_persSvcPerInputType.value() + "_"))) {
         const unsigned int maxContext = m_poolSvc->getInputContextMapSize();
         const unsigned int auxContext = m_poolSvc->getInputContext(tokAddr->getToken()->classID().toString() + tokAddr->getToken()->dbID().toString(), 1);
         if (m_poolSvc->getInputContextMapSize() > maxContext) {
            if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, auxContext, false, true, false).isSuccess()) {
               ATH_MSG_DEBUG("setInputAttribute failed setting POOL database/container attributes.");
            }
         }
         tokAddr->getToken()->setAuxString(CxxUtils::HexString<"[CTXT={}]">(auxContext));
      }
   }
   // Forward to base class createObj
   StatusCode status = ::AthCnvSvc::createObj(pAddress, refpObject);
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createRep(DataObject* pObject, IOpaqueAddress*& refpAddress) {
   assert(pObject);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pObject->clID(), objName).isFailure()) {
         objName = std::to_string(pObject->clID());
      }
      objName += '#';
      objName += pObject->registry()->name();
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cRep_" + objName, m_chronoMap);
   StatusCode status = StatusCode::FAILURE;
   if (pObject->clID() == 1) {
      // No transient object was found use cnv to write default persistent object
      SG::DataProxy* proxy = dynamic_cast<SG::DataProxy*>(pObject->registry());
      if (proxy != nullptr) {
         IConverter* cnv = converter(proxy->clID());
         status = cnv->createRep(pObject, refpAddress);
      }
   } else {
      // Forward to base class createRep
      try {
         status = ::AthCnvSvc::createRep(pObject, refpAddress);
      } catch(std::runtime_error& e) {
         ATH_MSG_FATAL(e.what());
      }
   }
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::fillRepRefs(IOpaqueAddress* pAddress, DataObject* pObject) {
   assert(pObject);
   std::string objName = "ALL";
   if (m_useDetailChronoStat.value()) {
      if (m_clidSvc->getTypeNameOfID(pObject->clID(), objName).isFailure()) {
         objName = std::to_string(pObject->clID());
      }
      objName += '#';
      objName += pObject->registry()->name();
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("fRep_" + objName, m_chronoMap);
   StatusCode status = StatusCode::FAILURE;
   if (pObject->clID() == 1) {
      // No transient object was found use cnv to write default persistent object
      SG::DataProxy* proxy = dynamic_cast<SG::DataProxy*>(pObject->registry());
      if (proxy != nullptr) {
         IConverter* cnv = converter(proxy->clID());
         status = cnv->fillRepRefs(pAddress, pObject);
      }
   } else {
      // Forward to base class fillRepRefs
      try {
         status = ::AthCnvSvc::fillRepRefs(pAddress, pObject);
      } catch(std::runtime_error& e) {
         ATH_MSG_FATAL(e.what());
      }
   }
   return(status);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::connectOutput(const std::string& outputConnectionSpec,
		const std::string& openMode) {
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   unsigned int contextId = outputContextId(outputConnection);
   Io::IoFlag mode = openMode == "APPEND" ? Io::APPEND : Io::WRITE;
   try {
      if (!m_poolSvc->connect(mode, contextId).isSuccess()) {
         ATH_MSG_ERROR("connectOutput FAILED to open an " << openMode << " transaction.");
         return(StatusCode::FAILURE);
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("connectOutput - caught exception: " << e.what());
      return(StatusCode::FAILURE);
   }
   std::unique_lock<std::mutex> lock(m_mutex);
   if (m_processedContextIds.insert(contextId).second) {
      // Extracting OUTPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
      extractPoolAttributes(m_poolAttr, &m_containerAttr, &m_databaseAttr, &m_domainAttr);
   }
   if (!processPoolAttributes(m_domainAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("connectOutput failed process POOL domain attributes.");
   }
   if (!processPoolAttributes(m_databaseAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("connectOutput failed process POOL database attributes.");
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::connectOutput(const std::string& outputConnectionSpec) {
// This is called before DataObjects are being converted.
   return(connectOutput(outputConnectionSpec, "UPDATE"));
}

//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::commitOutput(const std::string& outputConnectionSpec, bool doCommit) {
   // This is called after all DataObjects are converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("commitOutput", m_chronoMap);
   std::unique_lock<std::mutex> lock(m_mutex);
   unsigned int contextId = outputContextId(outputConnection);
   if (!processPoolAttributes(m_domainAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL domain attributes.");
   }
   if (!processPoolAttributes(m_databaseAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL database attributes.");
   }
   if (!processPoolAttributes(m_containerAttr, outputConnection, contextId).isSuccess()) {
      ATH_MSG_DEBUG("commitOutput failed process POOL container attributes.");
   }

   // lock.unlock();  //MN: first need to make commitCache slot-specific
   try {
      if (doCommit) {
         if (!m_poolSvc->commit(contextId).isSuccess()) {
            ATH_MSG_ERROR("commitOutput FAILED to commit OutputStream.");
            return(StatusCode::FAILURE);
         }
      } else {
         if (!m_poolSvc->commitAndHold(contextId).isSuccess()) {
            ATH_MSG_ERROR("commitOutput FAILED to commitAndHold OutputStream.");
            return(StatusCode::FAILURE);
         }
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("commitOutput - caught exception: " << e.what());
      return(StatusCode::FAILURE);
   }
   if (!this->cleanUp(outputConnection).isSuccess()) {
      ATH_MSG_ERROR("commitOutput FAILED to cleanup converters.");
      return(StatusCode::FAILURE);
   }

   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::disconnectOutput(const std::string& outputConnectionSpec) {
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   unsigned int contextId = outputContextId(outputConnection);
   StatusCode sc = m_poolSvc->disconnect(contextId);
   return sc;
}

//______________________________________________________________________________
unsigned int AthenaPoolCnvSvc::outputContextId(const std::string& outputConnection) {
   return m_persSvcPerOutput?
      m_poolSvc->getOutputContext(outputConnection) : (unsigned int)IPoolSvc::kOutputStream;
}

//______________________________________________________________________________
IPoolSvc* AthenaPoolCnvSvc::getPoolSvc() {
   return(&*m_poolSvc);
}
//______________________________________________________________________________
Token* AthenaPoolCnvSvc::registerForWrite(Placement* placement, const void* obj, const RootType& classDesc) {
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cRepR_ALL", m_chronoMap);
   Token* token = nullptr;
   if (m_persSvcPerOutput) { // Use separate PersistencySvc for each output stream/file
      placement->setAuxString(CxxUtils::HexString<"[CTXT={}]">(m_poolSvc->getOutputContext(placement->fileName())));
   }
   if(placement->technology() == 0) { // No technology specified, use the default
      placement->setTechnology(pool::DbType::getType(m_defaultContainerType).type());
   }
   token = m_poolSvc->registerForWrite(placement, obj, classDesc);
   return(token);
}
//______________________________________________________________________________
void AthenaPoolCnvSvc::setObjPtr(void*& obj, const Token* token) {
   ATH_MSG_VERBOSE("Requesting object for: " << token->toString());
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cObjR_ALL", m_chronoMap);
   if (token->dbID() != Guid::null()) {
      ATH_MSG_VERBOSE("Requesting object for: " << token->toString());
      m_poolSvc->setObjPtr(obj, token);
   }
}
//______________________________________________________________________________
bool AthenaPoolCnvSvc::useDetailChronoStat() const {
   return(m_useDetailChronoStat.value());
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string* par,
		const unsigned long* ip,
		IOpaqueAddress*& refpAddress) {
   if( svcType != repSvcType() ) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << repSvcType());
      return(StatusCode::FAILURE);
   }
   std::unique_ptr<Token> token;
   Token *t = m_poolSvc->getToken(par[0], par[1], ip[0]);
   if( t ) {
      token = std::make_unique<Token>(t);
      t->release();
   }
   if (token == nullptr) {
      return(StatusCode::RECOVERABLE);
   }
   refpAddress = new TokenAddress(repSvcType(), clid, "", par[1], IPoolSvc::kInputStream, std::move(token));
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string& refAddress,
		IOpaqueAddress*& refpAddress) {
   if (svcType != repSvcType()) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << repSvcType());
      return(StatusCode::FAILURE);
   }
   refpAddress = new GenericAddress(repSvcType(), clid, refAddress);
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::convertAddress(const IOpaqueAddress* pAddress,
		std::string& refAddress) {
   assert(pAddress);
   const TokenAddress* tokAddr = dynamic_cast<const TokenAddress*>(pAddress);
   if (tokAddr != nullptr && tokAddr->getToken() != nullptr) {
      refAddress = tokAddr->getToken()->toString();
   } else {
      refAddress = *pAddress->par();
   }
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::registerCleanUp(IAthenaPoolCleanUp* cnv) {
   m_cnvs.push_back(cnv);
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::cleanUp(const std::string& connection) {
   bool retError = false;
   std::size_t cpos = connection.find(':');
   std::size_t bpos = connection.find('[');
   if (cpos == std::string::npos) {
      cpos = 0;
   } else {
      cpos++;
   }
   if (bpos != std::string::npos) bpos = bpos - cpos;
   const std::string conn = connection.substr(cpos, bpos);
   ATH_MSG_VERBOSE("Cleanup for Connection='"<< conn <<"'");
   for (auto converter : m_cnvs) {
      if (!converter->cleanUp(conn).isSuccess()) {
         ATH_MSG_WARNING("AthenaPoolConverter cleanUp failed.");
         retError = true;
      }
   }
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::setInputAttributes(const std::string& fileName) {
   // Set attributes for input file
   m_lastInputFileName = fileName; // Save file name for printing attributes per event
   if (!m_persSvcPerInputType.empty()) {
// Loop over all extra event input contexts
      const auto& extraInputContextMap = m_poolSvc->getInputContextMap();
      for (const auto& [label, id]: extraInputContextMap) {
         if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, id, false, true, false).isSuccess()) {
            ATH_MSG_DEBUG("setInputAttribute failed setting POOL database/container attributes.");
         }
      }
   }
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, false, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed setting POOL database/container attributes.");
   }
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed getting POOL database/container attributes.");
   }
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
void AthenaPoolCnvSvc::handle(const Incident& incident) {
   if (incident.type() == "ProcessEventAttributes") {
      m_inputAttrPerEvent.push_back({"SET_ACTIVE_ENTRY", incident.source(), m_lastInputFileName, ""});
      if (!processPoolAttributes(m_inputAttrPerEvent, m_lastInputFileName, IPoolSvc::kInputStream).isSuccess()) {
         ATH_MSG_DEBUG("handle ProcessEventAttributes failed process POOL database attributes.");
      }
   }
}
//______________________________________________________________________________
AthenaPoolCnvSvc::AthenaPoolCnvSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator, pool::POOL_StorageType.type()) {
}
//__________________________________________________________________________
void AthenaPoolCnvSvc::extractPoolAttributes(const StringArrayProperty& property,
		std::vector<std::vector<std::string> >* contAttr,
		std::vector<std::vector<std::string> >* dbAttr,
		std::vector<std::vector<std::string> >* domAttr) const {
   std::vector<std::string> opt;
   std::string attributeName, containerName, databaseName, valueString;
   for (const auto& propertyValue : property.value()) {
      opt.clear();
      attributeName.clear();
      containerName.clear();
      databaseName.clear();
      valueString.clear();
      using Gaudi::Utils::AttribStringParser;
      for (const AttribStringParser::Attrib& attrib : AttribStringParser (propertyValue)) {
         if (attrib.tag == "DatabaseName") {
            databaseName = attrib.value;
         } else if (attrib.tag == "ContainerName") {
            if (databaseName.empty()) {
               databaseName = "*";
            }
            containerName = attrib.value;
         } else {
            attributeName = attrib.tag;
            valueString = attrib.value;
         }
      }
      if (!attributeName.empty() && !valueString.empty()) {
         opt.push_back(attributeName);
         opt.push_back(valueString);
         if (!databaseName.empty()) {
            opt.push_back(databaseName);
            if (!containerName.empty()) {
               opt.push_back(containerName);
               if (containerName.compare(0, 6, "TTree=") == 0) {
                  dbAttr->push_back(opt);
               } else {
                  contAttr->push_back(opt);
               }
            } else {
               opt.push_back("");
               dbAttr->push_back(opt);
            }
         } else if (domAttr != 0) {
            domAttr->push_back(opt);
         } else {
            opt.push_back("*");
            opt.push_back("");
            dbAttr->push_back(opt);
         }
      }
   }
}
//__________________________________________________________________________
StatusCode AthenaPoolCnvSvc::processPoolAttributes(std::vector<std::vector<std::string> >& attr,
		const std::string& fileName,
		unsigned long contextId,
		bool doGet,
		bool doSet,
		bool doClear) const {
   bool retError = false;
   for (auto& attrEntry : attr) {
      if (attrEntry.size() == 2) {
         const std::string& opt = attrEntry[0];
         std::string data = attrEntry[1];
         if (data == "int" || data == "DbLonglong" || data == "double" || data == "string") {
            if (doGet) {
               if (!m_poolSvc->getAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), contextId).isSuccess()) {
                  ATH_MSG_DEBUG("getAttribute failed for domain attr " << opt);
                  retError = true;
               }
            }
         } else if (doSet) {
            if (m_poolSvc->setAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), contextId).isSuccess()) {
               ATH_MSG_DEBUG("setAttribute " << opt << " to " << data);
               if (doClear) {
                  attrEntry.clear();
               }
            } else {
               ATH_MSG_DEBUG("setAttribute failed for domain attr " << opt << " to " << data);
               retError = true;
            }
         }
      }
      if (attrEntry.size() == 4) {
         const std::string& opt = attrEntry[0];
         std::string data = attrEntry[1];
         const std::string& file = attrEntry[2];
         const std::string& cont = attrEntry[3];
         if (!fileName.empty() && (0 == fileName.compare(0, fileName.find('?'), file)
	         || (file[0] == '*' && file.find("," + fileName + ",") == std::string::npos))) {
            if (data == "int" || data == "DbLonglong" || data == "double" || data == "string") {
               if (doGet) {
                  if (!m_poolSvc->getAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), fileName, cont, contextId).isSuccess()) {
                     ATH_MSG_DEBUG("getAttribute failed for database/container attr " << opt);
                     retError = true;
                  }
               }
            } else if (doSet) {
               if (m_poolSvc->setAttribute(opt, data, pool::DbType(pool::ROOTTREE_StorageType).type(), fileName, cont, contextId).isSuccess()) {
                  ATH_MSG_DEBUG("setAttribute " << opt << " to " << data << " for db: " << fileName << " and cont: " << cont);
                  if (doClear) {
                     if (file[0] == '*' && !m_persSvcPerOutput) {
                        attrEntry[2] += "," + fileName + ",";
                     } else {
                        attrEntry.clear();
                     }
                  }
               } else {
                  ATH_MSG_DEBUG("setAttribute failed for " << opt << " to " << data << " for db: " << fileName << " and cont: " << cont);
                  retError = true;
               }
            }
         }
      }
   }
   std::erase_if(attr, [](const auto& entry) { return entry.empty(); });
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}
