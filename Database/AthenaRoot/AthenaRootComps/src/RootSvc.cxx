///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// RootSvc.cxx
// Implementation file for class Athena::RootSvc
// Author: Peter van Gemmeren <gemmeren@anl.gov>
///////////////////////////////////////////////////////////////////

// AthenaRootComps includes
#include "RootSvc.h"
#include "RootConnection.h"

// fwk includes
#include "AthenaKernel/IDictLoaderSvc.h"

#include "PersistentDataModel/Token.h"
#include "PersistentDataModel/Placement.h"

namespace Athena {

RootSvc::RootSvc(const std::string& name, ISvcLocator* pSvcLocator) :
	base_class(name, pSvcLocator),
	m_gCatalogMgr(Gaudi::svcLocator()->service<Gaudi::IFileCatalogMgr>( "Gaudi::MultiFileCatalog" )),
	m_gCatalog(m_gCatalogMgr),
	m_conns(),
	m_wconn(0),
	m_dictSvc("AthDictLoaderSvc", name) {
}

RootSvc::~RootSvc() {
  for (ConnMap_t::iterator itr = m_conns.begin(), iend = m_conns.end(); itr != iend; ++itr) {
    delete itr->second; itr->second = 0;
  }
  m_conns.clear();
}

StatusCode RootSvc::initialize() {
  ATH_MSG_INFO("Initializing " << name());
  if (!::AthService::initialize().isSuccess()) {
    ATH_MSG_FATAL("Cannot initialize ConversionSvc base class.");
    return StatusCode::FAILURE;
  }
  try {
    m_gCatalogMgr->addCatalog("xmlcatalog_file:RootFileCatalog.xml");
    m_gCatalogMgr->setWriteCatalog(m_gCatalogMgr->findCatalog("file:RootFileCatalog.xml", true));
    m_gCatalog->init();
  } catch (std::exception& e) {
    ATH_MSG_FATAL ("Set up Catalog - caught exception: " << e.what());
    return StatusCode::FAILURE;
  }
  ATH_CHECK(m_dictSvc.retrieve());
  return StatusCode::SUCCESS;
}

StatusCode RootSvc::finalize() {
  for (ConnMap_t::const_iterator itr = m_conns.begin(), iend = m_conns.end(); itr != iend; ++itr) {
    if (!itr->second->disconnect().isSuccess()) {
      ATH_MSG_WARNING("Cannot disconnect file = " << itr->first.toString());
    }
  }
  m_gCatalog->commit();
  return ::AthService::finalize();
}

/// Load the type (dictionary) from Root.
RootType RootSvc::getType(const std::type_info& type) const {
  return m_dictSvc->load_type(type);
}

/// Read object from Root.
void* RootSvc::readObject(const Token& /*token*/, void*& /*pObj*/) {
  return 0;
}

/// Write object of a given class to Root.
const Token* RootSvc::writeObject(const Placement& placement, const RootType& type, const void* pObj) {
  ATH_MSG_VERBOSE("RootSvc::writeObject pObj = " << pObj);
  if (m_wconn == 0) {
    ATH_MSG_ERROR("Cannot write without RootConnection for placement " << placement.containerName());
    return 0;
  }
  if (!m_wconn->setContainer(placement.containerName(), type.Name()).isSuccess()) {
    ATH_MSG_ERROR("Cannot set container [" << placement.containerName() << "]");
    return 0;
  }
  unsigned long ientry = 0;
  if (!m_wconn->write(pObj, ientry).isSuccess()) {
    ATH_MSG_ERROR("Cannot write Object to placement [" << placement.containerName() << "]");
    return 0;
  }
  return new Token();
}

/// Create an object of a given `RootType`.
void* RootSvc::createObject(const RootType& type) const {
  void* pObj = type.Construct();
  return pObj;
}

/// Destruct a given object of type `RootType`.
void RootSvc::destructObject(const RootType& /*type*/, void* /*pObj*/) const {
}

/// Open the file `fname` with open mode `mode`
StatusCode RootSvc::open(const std::string& fname, const std::string& /*mode*/) {
// Catalog to get fid...
  Guid fid = Guid::null();
  std::string fidString;
  fidString = m_gCatalog->lookupPFN(fname);
  if( fidString.empty() ) {
     fidString = m_gCatalog->createFID();
     m_gCatalog->registerPFN(fidString, fname, "ROOT_All");
  }
  fid.fromString(fidString);
  Athena::RootConnection* conn = 0;
  ConnMap_t::const_iterator fitr = m_conns.find(fid);
  if (fitr == m_conns.end()) {
     conn = new Athena::RootConnection(this, fname);
     m_conns.insert(std::make_pair(fid, conn));
  } else {
     conn = fitr->second;
  }
  if (conn == 0) {
     ATH_MSG_ERROR("Cannot get RootConnection for file " << fid.toString());
     return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Connect the file `fname` to the service.
StatusCode RootSvc::connect(const std::string& fname) {
  ATH_MSG_VERBOSE("connect(" << fname << ")...");
  Athena::RootConnection* conn = this->connection(fname);
  if (conn == 0) {
    ATH_MSG_ERROR("No open RootConnection for file " << fname);
    return StatusCode::FAILURE;
  }
  if (!conn->connectWrite("recreate").isSuccess()) {
    ATH_MSG_ERROR("Cannot connect to file " << fname);
    return StatusCode::FAILURE;
  }
  m_wconn = conn;
  return StatusCode::SUCCESS;
}

StatusCode RootSvc::commitOutput() {
  ATH_MSG_VERBOSE("RootSvc::commitOutput");
  if (m_wconn == 0) {
    ATH_MSG_ERROR("Cannot commit without RootConnection.");
    return StatusCode::FAILURE;
  }
  if (!m_wconn->commit().isSuccess()) {
    ATH_MSG_ERROR("Cannot commit RootConnection.");
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS;
}

/// Disconnect the file `fname` from the service.
StatusCode RootSvc::disconnect(const std::string& fname) {
  ATH_MSG_VERBOSE("disconnect(" << fname << ")...");
  Athena::RootConnection* conn = this->connection(fname);
  if (conn == 0) {
    ATH_MSG_ERROR("No open RootConnection for file " << fname);
    return StatusCode::FAILURE;
  }
  if (!conn->disconnect().isSuccess()) {
    ATH_MSG_ERROR("Cannot disconnect to file " << fname);
    return StatusCode::FAILURE;
  }
  if (m_wconn == conn) {
    m_wconn = 0;
  }
  return StatusCode::SUCCESS;
}

/// Get the RootConnection associated with file `fname`
/// @returns NULL if no such file is known to this service
Athena::RootConnection* RootSvc::connection(const std::string& fname) {
// Catalog to get fid...
  Guid fid = Guid::null();
  std::string fidString;
  fidString = m_gCatalog->lookupPFN(fname);
  if( fidString.empty() ) {
     fidString = m_gCatalog->createFID();
     m_gCatalog->registerPFN(fidString, fname, "ROOT_All");
  }
  fid.fromString(fidString);
  Athena::RootConnection* conn = 0;
  ConnMap_t::const_iterator fitr = m_conns.find(fid);
  if (fitr != m_conns.end()) {
    conn = fitr->second;
  }
  return conn;
}

} //> namespace Athena
