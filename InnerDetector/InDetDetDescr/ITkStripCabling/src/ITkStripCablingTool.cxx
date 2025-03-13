/*
/  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ITkStripCablingTool.cxx
 * Implementation file for ITkStrip cabling tool
 * Based on code by Susumu Oda
 * @author Shaun Roe
 * @author Edson Carquin
 * @author Daniel Torres
 * @date 06 March, 2025
 **/
 
//this package
#include "ITkStripCablingTool.h"

//Athena
#include "InDetIdentifier/SCT_ID.h"
#include "Identifier/Identifier.h"
#include "StoreGate/ReadCondHandle.h"

//Gaudi includes
#include "GaudiKernel/StatusCode.h"

//constants in file scope
static const std::string coracool("CORACOOL");
static const std::string coolVectorPayload("COOLVECTOR");
static const std::string defaultSource(coracool);
static const std::string file("ITkStrip_Sept08Cabling_svc.dat");
//invalid identifiers to return in case of error
static const ITkStripOnlineId invalidId;
static const IdentifierHash invalidHash;

// Constructor
ITkStripCablingTool::ITkStripCablingTool(const std::string& type, const std::string& name, const IInterface* parent) :
  base_class(type, name, parent)
{}

//
StatusCode
ITkStripCablingTool::initialize() {
  ATH_MSG_DEBUG("Initialize ITkStrip cabling");
  const std::string cablingDataSource = m_cablingDataSource.value();
  m_usingDatabase=(cablingDataSource == coracool) or (cablingDataSource == coolVectorPayload) or (cablingDataSource == file);
  ATH_CHECK(detStore()->retrieve(m_idHelper, "SCT_ID"));
  ATH_CHECK(m_data.initialize());
  return StatusCode::SUCCESS;
}

ITkStripOnlineId 
ITkStripCablingTool::getOnlineIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const {
  const ITkStripCablingData* data{getData(ctx)};
  if (data==nullptr) {
    ATH_MSG_FATAL("Filling the cabling FAILED");
    return invalidId;
  }
  
  return data->getOnlineIdFromHash(hash);
}

ITkStripOnlineId
ITkStripCablingTool::getOnlineIdFromHash(const IdentifierHash& hash) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getOnlineIdFromHash(hash, ctx);
}

std::uint32_t
ITkStripCablingTool::getRobIdFromHash(const IdentifierHash& hash, const EventContext& ctx) const {
  return getOnlineIdFromHash(hash, ctx).rod();
}

std::uint32_t
ITkStripCablingTool::getRobIdFromHash(const IdentifierHash& hash) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  return getRobIdFromHash(hash, ctx);
}

void
ITkStripCablingTool::getAllRods(std::vector<std::uint32_t>& usersVector, const EventContext& ctx) const {
  const ITkStripCablingData* data{getData(ctx)};
  if (data==nullptr) {
    ATH_MSG_FATAL("Filling the cabling FAILED");
    return;
  }

  data->getRods(usersVector);
}

void
ITkStripCablingTool::getAllRods(std::vector<std::uint32_t>& usersVector) const {
  const EventContext& ctx{Gaudi::Hive::currentContext()};
  getAllRods(usersVector, ctx);
}

const ITkStripCablingData*
ITkStripCablingTool::getData(const EventContext& ctx) const {
  SG::ReadCondHandle<ITkStripCablingData> condData{m_data, ctx};
  ATH_MSG_DEBUG("After getting ITkStripCablindData");
  return condData.retrieve();
}

