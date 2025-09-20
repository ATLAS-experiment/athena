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

#include "StorageSvc/DbReflex.h"
#include "RootUtils/APRDefaults.h"

#include <algorithm>
#include <iomanip>
#include <sstream>

//______________________________________________________________________________
// Initialize the service.
StatusCode AthenaPoolCnvSvc::initialize() {
   // Initialize DataModelCompatSvc
   ServiceHandle<IService> dmcsvc("DataModelCompatSvc", this->name());
   ATH_CHECK(dmcsvc.retrieve());
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
   // Extracting MaxFileSizes for global default and map by Database name.
   for (std::vector<std::string>::const_iterator iter = m_maxFileSizes.value().begin(),
	   last = m_maxFileSizes.value().end(); iter != last; ++iter) {
      if (auto p = iter->find('='); p != std::string::npos) {
         long long maxFileSize = atoll(iter->data() + (p + 1));
         std::string databaseName = iter->substr(0, iter->find_first_of(" 	="));
         m_databaseMaxFileSize.insert(std::make_pair(databaseName, maxFileSize));
      } else {
         m_domainMaxFileSize = atoll(iter->c_str());
      }
   }
   ATH_MSG_DEBUG("Setting StorageType to " << m_storageTechProp.value());
   m_dbType = pool::DbType::getType( m_storageTechProp.value() );
   if( m_dbType == TEST_StorageType ) {
      ATH_MSG_FATAL("Unknown StorageType rquested: " << m_storageTechProp.value());
      return StatusCode::FAILURE;
   }
   if( m_containerPrefixProp.value() == "Default" ) {
      // select default storage element name accoring to storage tech
      if( m_dbType.exactMatch(pool::ROOTRNTUPLE_StorageType) ) m_containerPrefixProp.setValue( APRDefaults::RNTupleNames::EventData );
      else m_containerPrefixProp.setValue( APRDefaults::TTreeNames::EventData );
   }

   // Extracting INPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
   extractPoolAttributes(m_inputPoolAttr, &m_inputAttr, &m_inputAttr, &m_inputAttr);
   // Extracting the INPUT POOL ItechnologySpecificAttributes which are to be printed for each event
   extractPoolAttributes(m_inputPoolAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent, &m_inputAttrPerEvent);
   // Setup incident for EndEvent to print out attributes each event
   ServiceHandle<IIncidentSvc> incSvc("IncidentSvc", name());
   long int pri = 1000;
   if (!m_inputPoolAttrPerEvent.value().empty()) {
      // Set to be listener for EndEvent
      incSvc->addListener(this, "EndEvent", pri);
      ATH_MSG_DEBUG("Subscribed to EndEvent for printing out input file attributes.");
   }
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
   m_contextAttr.clear();
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
         std::ostringstream oss;
         oss << std::dec << pAddress->clID();
         objName = oss.str();
      }
      objName += '#';
      objName += *(pAddress->par() + 1);
   }
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("cObj_" + objName, m_chronoMap);
   if (!m_persSvcPerInputType.empty()) { // Use separate PersistencySvc for each input data type
      TokenAddress* tokAddr = dynamic_cast<TokenAddress*>(pAddress);
      if (tokAddr != nullptr && tokAddr->getToken() != nullptr && (tokAddr->getToken()->contID().starts_with(m_persSvcPerInputType.value() + "(") || tokAddr->getToken()->contID().starts_with(m_persSvcPerInputType.value() + "_"))) {
         const unsigned int maxContext = m_poolSvc->getInputContextMap().size();
         const unsigned int auxContext = m_poolSvc->getInputContext(tokAddr->getToken()->classID().toString() + tokAddr->getToken()->dbID().toString(), 1);
         char text[32];
         ::sprintf(text, "[CTXT=%08X]", auxContext);
         if (m_poolSvc->getInputContextMap().size() > maxContext) {
            if (m_poolSvc->setAttribute("TREE_CACHE", "0", pool::DbType(pool::ROOTTREE_StorageType).type(), "FID:" + tokAddr->getToken()->dbID().toString(), m_persSvcPerInputType.value(), auxContext).isSuccess()) {
               ATH_MSG_DEBUG("setInputAttribute failed to switch off TTreeCache for id = " << auxContext << ".");
            }
         }
         tokAddr->getToken()->setAuxString(text);
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
         std::ostringstream oss;
         oss << std::dec << pObject->clID();
         objName = oss.str();
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
         std::ostringstream oss;
         oss << std::dec << pObject->clID();
         objName = oss.str();
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
		const std::string& /*openMode*/) {
   return(connectOutput(outputConnectionSpec));
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::connectOutput(const std::string& outputConnectionSpec) {
// This is called before DataObjects are being converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   // Extract the technology
   int tech = m_dbType.type();
   if (!decodeOutputSpec(outputConnection, tech).isSuccess()) {
      ATH_MSG_ERROR("connectOutput FAILED extract file name and technology.");
      return(StatusCode::FAILURE);
   }
   unsigned int contextId = outputContextId(outputConnection);
   try {
      if (!m_poolSvc->connect(pool::ITransaction::UPDATE, contextId).isSuccess()) {
         ATH_MSG_ERROR("connectOutput FAILED to open an UPDATE transaction.");
         return(StatusCode::FAILURE);
      }
   } catch (std::exception& e) {
      ATH_MSG_ERROR("connectOutput - caught exception: " << e.what());
      return(StatusCode::FAILURE);
   }

   std::unique_lock<std::mutex> lock(m_mutex);
   if (std::find(m_contextAttr.begin(), m_contextAttr.end(), contextId) == m_contextAttr.end()) {
      std::size_t merge = outputConnection.find("?pmerge="); // Used to remove trailing TMemFile
      int flush = m_numberEventsPerWrite.value();
      m_contextAttr.push_back(contextId);
      // Setting default 'TREE_MAX_SIZE' for ROOT to 1024 GB to avoid file chains.
      std::vector<std::string> maxFileSize;
      maxFileSize.push_back("TREE_MAX_SIZE");
      maxFileSize.push_back("1099511627776L");
      m_domainAttr.emplace_back(std::move(maxFileSize));
      // Extracting OUTPUT POOL ItechnologySpecificAttributes for Domain, Database and Container.
      extractPoolAttributes(m_poolAttr, &m_containerAttr, &m_databaseAttr, &m_domainAttr);
      //FIXME
      for (std::vector<std::vector<std::string> >::iterator iter = m_databaseAttr.begin(), last = m_databaseAttr.end();
                      iter != last; ++iter) {
         const std::string& opt = (*iter)[0];
         std::string& data = (*iter)[1];
         const std::string& file = (*iter)[2];
         const std::string& cont = (*iter)[3];
         std::size_t equal = cont.find('='); // Used to remove leading "TTree="
         if (equal == std::string::npos) equal = 0;
         else equal++;
         std::size_t colon = m_containerPrefixProp.value().find(':');
         if (colon == std::string::npos) colon = 0; // Used to remove leading technology
         else colon++;
         const auto& strProp = m_containerPrefixProp.value();
         if (merge != std::string::npos && opt == "TREE_AUTO_FLUSH" && 0 == outputConnection.compare(0, merge, file) &&cont.compare(equal, std::string::npos, strProp, colon) == 0 && data != "int" && data != "DbLonglong" && data != "double" && data != "string") {
            flush = atoi(data.c_str());
            if (flush < 0 && m_numberEventsPerWrite.value() > 0) {
               flush = m_numberEventsPerWrite.value();
               std::ostringstream eventAutoFlush;
               eventAutoFlush << flush;
               data = eventAutoFlush.str();
            } else if (flush > 0 && flush < m_numberEventsPerWrite.value()) {
               flush = flush * (int((m_numberEventsPerWrite.value()) / flush - 0.5) + 1);
            }
         }
      }
      if (merge != std::string::npos) {
         ATH_MSG_INFO("connectOutput setting auto write for: " << outputConnection << " to " << flush << " events");
         m_fileFlushSetting[outputConnection.substr(0, merge)] = flush;
      }
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
StatusCode AthenaPoolCnvSvc::commitOutput(const std::string& outputConnectionSpec, bool doCommit) {
   // This is called after all DataObjects are converted.
   std::string outputConnection = outputConnectionSpec.substr(0, outputConnectionSpec.find('['));
   // StopWatch listens from here until the end of this current scope
   PMonUtils::BasicStopWatch stopWatch("commitOutput", m_chronoMap);
   std::unique_lock<std::mutex> lock(m_mutex);
   // Extract the technology
   int tech = m_dbType.type();
   if (!decodeOutputSpec(outputConnection, tech).isSuccess()) {
      ATH_MSG_ERROR("connectOutput FAILED extract file name and technology.");
      return(StatusCode::FAILURE);
   }
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
   std::size_t merge = outputConnection.find("?pmerge="); // Used to remove trailing TMemFile
   const std::string baseOutputConnection = outputConnection.substr(0, merge);
   m_fileCommitCounter[baseOutputConnection]++;
   if (merge != std::string::npos && m_fileFlushSetting[baseOutputConnection] > 0 && m_fileCommitCounter[baseOutputConnection] % m_fileFlushSetting[baseOutputConnection] == 0) {
      doCommit = true;
      ATH_MSG_DEBUG("commitOutput sending data.");
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
   if (!this->cleanUp(baseOutputConnection).isSuccess()) {
      ATH_MSG_ERROR("commitOutput FAILED to cleanup converters.");
      return(StatusCode::FAILURE);
   }
   // Check FileSize
   long long int currentFileSize = m_poolSvc->getFileSize(outputConnection, m_dbType.type(), contextId);
   if (m_databaseMaxFileSize.find(outputConnection) != m_databaseMaxFileSize.end()) {
      if (currentFileSize > m_databaseMaxFileSize[outputConnection]) {
         ATH_MSG_WARNING("FileSize > " << m_databaseMaxFileSize[outputConnection] << " for " << outputConnection);
         return(StatusCode::RECOVERABLE);
      }
   } else if (currentFileSize > m_domainMaxFileSize) {
      ATH_MSG_WARNING("FileSize > " << m_domainMaxFileSize <<  " for " << outputConnection);
      return(StatusCode::RECOVERABLE);
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
            char text[32];
            ::sprintf(text, "[CTXT=%08X]", m_poolSvc->getOutputContext(placement->fileName()));
            placement->setAuxString(text);
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
   if (svcType != POOL_StorageType) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << POOL_StorageType);
      return(StatusCode::FAILURE);
   }
   std::unique_ptr<Token> token;
   if (par[0].compare(0, 3, "SHM") == 0) {
      token = std::make_unique<Token>();
      token->setOid(Token::OID_t(ip[0], ip[1]));
      token->setAuxString("[PNAME=" + par[2] + "]");
      RootType classDesc = RootType::ByNameNoQuiet(par[2]);
      token->setClassID(pool::DbReflex::guid(classDesc));
   } else {
      token.reset(m_poolSvc->getToken(par[0], par[1], ip[0]));
   }
   if (token == nullptr) {
      return(StatusCode::RECOVERABLE);
   }
   refpAddress = new TokenAddress(POOL_StorageType, clid, "", par[1], IPoolSvc::kInputStream, std::move(token));
   return(StatusCode::SUCCESS);
}
//______________________________________________________________________________
StatusCode AthenaPoolCnvSvc::createAddress(long svcType,
		const CLID& clid,
		const std::string& refAddress,
		IOpaqueAddress*& refpAddress) {
   if (svcType != POOL_StorageType) {
      ATH_MSG_ERROR("createAddress: svcType != POOL_StorageType " << svcType << " " << POOL_StorageType);
      return(StatusCode::FAILURE);
   }
   refpAddress = new GenericAddress(POOL_StorageType, clid, refAddress);
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
//__________________________________________________________________________
StatusCode AthenaPoolCnvSvc::decodeOutputSpec(std::string& fileSpec, int& outputTech) const {
  if (fileSpec.starts_with ( "oracle") || fileSpec.starts_with ( "mysql")) {
      outputTech = pool::POOL_RDBMS_StorageType.type();
   } else if (fileSpec.starts_with ( "ROOTKEY:")) {
      outputTech = pool::ROOTKEY_StorageType.type();
      fileSpec.erase(0, 8);
   } else if (fileSpec.starts_with ( "ROOTTREE:")) {
      outputTech = pool::ROOTTREE_StorageType.type();
      fileSpec.erase(0, 9);
   } else if (fileSpec.starts_with ( "ROOTTREEINDEX:")) {
      outputTech = pool::ROOTTREEINDEX_StorageType.type();
      fileSpec.erase(0, 14);
   } else if (fileSpec.starts_with ( "ROOTRNTUPLE:")) {
      outputTech = pool::ROOTRNTUPLE_StorageType.type();
      fileSpec.erase(0, 12);
   } else if (outputTech == 0) {
      outputTech = m_dbType.type();
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
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, false, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed setting POOL database/container attributes.");
   }
   if (!processPoolAttributes(m_inputAttr, m_lastInputFileName, IPoolSvc::kInputStream, true, false).isSuccess()) {
      ATH_MSG_DEBUG("setInputAttribute failed getting POOL database/container attributes.");
   }
   if (!m_persSvcPerInputType.empty()) {
      // Loop over all extra event input contexts and switch off TTreeCache
      const auto& extraInputContextMap = m_poolSvc->getInputContextMap();
      for (const auto& [label, id]: extraInputContextMap) {
         if (m_poolSvc->setAttribute("TREE_CACHE", "0", pool::DbType(pool::ROOTTREE_StorageType).type(), m_lastInputFileName, m_persSvcPerInputType.value(), id).isSuccess()) {
            ATH_MSG_DEBUG("setInputAttribute failed to switch off TTreeCache for = " << label << ".");
         }
      }
   }
   return(StatusCode::SUCCESS);
}

//______________________________________________________________________________
void AthenaPoolCnvSvc::handle(const Incident& incident) {
   if (incident.type() == "EndEvent") {
      if (!processPoolAttributes(m_inputAttrPerEvent, m_lastInputFileName, IPoolSvc::kInputStream).isSuccess()) {
         ATH_MSG_DEBUG("handle EndEvent failed process POOL database attributes.");
      }
   }
}
//______________________________________________________________________________
AthenaPoolCnvSvc::AthenaPoolCnvSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator, POOL_StorageType) {
}
//__________________________________________________________________________
void AthenaPoolCnvSvc::extractPoolAttributes(const StringArrayProperty& property,
		std::vector<std::vector<std::string> >* contAttr,
		std::vector<std::vector<std::string> >* dbAttr,
		std::vector<std::vector<std::string> >* domAttr) const {
   std::vector<std::string> opt;
   std::string attributeName, containerName, databaseName, valueString;
   for (std::vector<std::string>::const_iterator iter = property.value().begin(),
           last = property.value().end(); iter != last; ++iter) {
      opt.clear();
      attributeName.clear();
      containerName.clear();
      databaseName.clear();
      valueString.clear();
      using Gaudi::Utils::AttribStringParser;
      for (const AttribStringParser::Attrib& attrib : AttribStringParser (*iter)) {
         const std::string tag = attrib.tag;
         const std::string val = attrib.value;
         if (tag == "DatabaseName") {
            databaseName = val;
         } else if (tag == "ContainerName") {
            if (databaseName.empty()) {
               databaseName = "*";
            }
            containerName = std::move(val);
         } else {
            attributeName = std::move(tag);
            valueString = std::move(val);
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
   for (std::vector<std::vector<std::string> >::iterator iter = attr.begin(), last = attr.end();
		   iter != last; ++iter) {
      if (iter->size() == 2) {
         const std::string& opt = (*iter)[0];
         std::string data = (*iter)[1];
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
                  iter->clear();
               }
            } else {
               ATH_MSG_DEBUG("setAttribute failed for domain attr " << opt << " to " << data);
               retError = true;
            }
         }
      }
      if (iter->size() == 4) {
         const std::string& opt = (*iter)[0];
         std::string data = (*iter)[1];
         const std::string& file = (*iter)[2];
         const std::string& cont = (*iter)[3];
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
                        (*iter)[2] += "," + fileName + ",";
                     } else {
                        iter->clear();
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
   for (std::vector<std::vector<std::string> >::iterator iter = attr.begin(); iter != attr.end(); ) {
      if (iter->empty()) {
         iter = attr.erase(iter);
      } else {
         ++iter;
      }
   }
   return(retError ? StatusCode::FAILURE : StatusCode::SUCCESS);
}
