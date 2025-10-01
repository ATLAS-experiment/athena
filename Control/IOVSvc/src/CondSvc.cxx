/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "CondSvc.h"
#include "AthenaKernel/CondCont.h"
#include "GaudiKernel/EventIDBase.h"
#include "AthenaKernel/StoreID.h"
#include "AthenaKernel/BaseInfo.h"


//---------------------------------------------------------------------------

CondSvc::CondSvc( const std::string& name, ISvcLocator* svcLoc ):
  base_class(name,svcLoc),
  m_sgs("StoreGateSvc/ConditionStore", name)
{
}

//---------------------------------------------------------------------------

StatusCode
CondSvc::validRanges( std::vector< EventIDRange >& ranges, const DataObjID& id ) const
{
  // Retrieve all conditions data and search
  SG::ConstIterator< CondContBase > cib, cie;
  StatusCode sc = m_sgs->retrieve( cib, cie );
  if ( sc.isSuccess() ) {
    while ( cib != cie ) {
      if ( cib->id() == id ) {
        ranges = cib->ranges();
      }
      ++cib;
    }
  }

  return sc;
}

//---------------------------------------------------------------------------

StatusCode
CondSvc::initialize() {

  ATH_CHECK( m_sgs.retrieve() );

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

void
CondSvc::dump(std::ostream& ost) const {

  std::scoped_lock lock(m_lock);

  ost << "CondSvc::dump()";

  ost << "\ndumping id->alg map\n";
  for (const auto& [id, alg] : m_idMap) {
    ost << "\n + " << id << " : " << alg->name();
  }

  ost << "\n\ndumping ConditionStore:\n\n";

  SG::ConstIterator<CondContBase> cib,cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while (cib != cie) {
      ost << " + ";
      cib->list(ost);
      ++cib;
    }
  }

  ost << "\n";
    
}

//---------------------------------------------------------------------------

StatusCode
CondSvc::start()
{
  // Call this now, in case there is no CondInputLoader.
  ATH_CHECK( setupDone() );
  return StatusCode::SUCCESS;
}


StatusCode
CondSvc::stop() {

  ATH_MSG_DEBUG( "CondSvc::stop()" );

  if (msgLvl(MSG::DEBUG)) {
    std::ostringstream ost;
    dump(ost);
    
    ATH_MSG_DEBUG( ost.str() );
  }

  return StatusCode::SUCCESS;

}

//---------------------------------------------------------------------------

StatusCode 
CondSvc::regHandle(IAlgorithm* alg, const Gaudi::DataHandle& dh) {

  std::scoped_lock lock(m_lock);
  return regHandle_i(alg, dh);

}

//---------------------------------------------------------------------------

// separate implementation to avoid the use of a recursive mutex
StatusCode 
CondSvc::regHandle_i(IAlgorithm* alg, const Gaudi::DataHandle& dh) {

  ATH_MSG_DEBUG( "regHandle: alg: " << alg->name() << "  id: "
                 << dh.fullKey() );

  if (dh.mode() != Gaudi::DataHandle::Writer) {
    ATH_MSG_DEBUG(dh.fullKey() << " is a ReadHandle. No need to register.");
    return StatusCode::SUCCESS;
  }

  const auto [itr, success] = m_idMap.try_emplace(dh.fullKey(), alg);
  if (!success) {
    const IAlgorithm *ia = itr->second;
    if (ia->name() != alg->name()) {
      ATH_MSG_ERROR("WriteCondHandle " << dh.fullKey()
                    << " is already registered against a different Algorithm "
                    << ia->name()
                    << ". This is not allowed.");
      return StatusCode::FAILURE;
    }
  }

  m_condAlgs.emplace(alg);
  m_condIDs.emplace( dh.fullKey() );

  StatusCode sc{StatusCode::SUCCESS};

  const CLID clid = dh.fullKey().clid();
  const SG::BaseInfoBase* bib = SG::BaseInfoBase::find( clid );
  if ( bib ) {
    for (CLID clid2 : bib->get_bases()) {
      if (clid2 != clid) {
        SG::VarHandleKey vhk(clid2,dh.objKey(),Gaudi::DataHandle::Writer,
                             StoreID::storeName(StoreID::CONDITION_STORE));
        sc &= regHandle_i(alg, vhk);
      }
    }
  }

  return sc;
}


//---------------------------------------------------------------------------

bool
CondSvc::isValidID(const EventContext& ctx, const DataObjID& id) const {
  // Don't take out the lock here.
  // In many-thread jobs, a lock here becomes heavily contended.
  // The only potential conflict is with setupDone(),
  // which should only be called during START.

  auto it = m_condConts.find (id);
  if (it != m_condConts.end()) {
    const bool valid = it->second->valid (ctx.eventID());
    ATH_MSG_VERBOSE("CondSvc::isValidID:  now: " << ctx.eventID() << "  id : "
                    << id << (valid ? ": T" : ": F") );
    return valid;
  }
  else {
    ATH_MSG_ERROR( "Cannot find CondCont " << id );
  }

  return false;

}

//---------------------------------------------------------------------------

/**
 * Create DataObjID -> CondCont map for usage in isValidID.
 *
 * As some conditions are looked up via their base class, we need to register
 * each CondCont key against all its possible bases.
 */
StatusCode CondSvc::setupDone()
{
  std::scoped_lock lock(m_lock);

  // CondHandleKeys always carry the store prefix. Prepend it to the raw key name.
  const std::string& storePrefix = StoreID::storeName(StoreID::CONDITION_STORE) + "+";

  SG::ConstIterator<CondContBase> cib, cie;
  if (m_sgs->retrieve(cib,cie).isSuccess()) {
    while(cib != cie) {
      // CLID of CondCont payload
      CLID clid = cib->id().clid();
      const SG::BaseInfoBase* bib = SG::BaseInfoBase::find(clid);
      if ( !bib ) {
        // If no bases, register type
        m_condConts.try_emplace( DataObjID(clid, storePrefix + cib.key()), &*cib );
      }
      else {
        // Otherwise register all bases (which includes itself)
        for (CLID clid2 : bib->get_bases()) {
          m_condConts.try_emplace( DataObjID(clid2, storePrefix + cib.key()), &*cib );
        }
      }
      ++cib;
    }
  }

  return StatusCode::SUCCESS;
}
