/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "HiveAlgV.h"
#include <vector>

HiveAlgV::HiveAlgV( const std::string& name, 
		    ISvcLocator* pSvcLocator ) : 
  ::HiveAlgBase( name, pSvcLocator )
{}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
HiveAlgV::~HiveAlgV() = default;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgV::initialize() {
  ATH_MSG_DEBUG("initialize " << name());

  ATH_CHECK( m_rhv.initialize() );
  ATH_CHECK( m_whv.initialize() );

  ATH_MSG_INFO(m_rhv.keys() << " : " << m_rhv.size());
  ATH_MSG_INFO(m_whv.keys() << " : " << m_whv.size());

  // initialize base class
  return HiveAlgBase::initialize ();
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode HiveAlgV::execute(const EventContext& ctx) const {

  ATH_MSG_DEBUG("execute " << name());
 
  sleep(ctx);

  if (m_writeFirst) {
    ATH_CHECK(write(ctx));
    ATH_CHECK(read(ctx));
  } else {
    ATH_CHECK(read(ctx));
    ATH_CHECK(write(ctx));
  }

  return StatusCode::SUCCESS;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode
HiveAlgV::read(const EventContext& ctx) const {
  StatusCode sc { StatusCode::SUCCESS };
  std::vector< SG::ReadHandle<HiveDataObj> > rhv = m_rhv.makeHandles(ctx);
  for (auto &hnd : rhv) {
    if (!hnd.isValid()) {
      ATH_MSG_ERROR ("Could not retrieve HiveDataObj with key " << hnd.key());
      sc = StatusCode::FAILURE;
    } else {
      ATH_MSG_INFO("  read: " << hnd.key() << " = " << hnd->val() );
    }
  }
  return sc;
}

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
StatusCode
HiveAlgV::write(const EventContext& ctx) const {
  std::vector< SG::WriteHandle<HiveDataObj> > whv = m_whv.makeHandles(ctx);
  for (auto &hnd : whv) {
    ATH_CHECK(hnd.record(std::make_unique<HiveDataObj>( 10101 )));
    ATH_MSG_INFO("  write: " << hnd.key() << " = " << hnd->val() );
  }
  return StatusCode::SUCCESS;
}

