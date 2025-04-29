///////////////////////// -*- C++ -*- /////////////////////////////

/*
  Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration
*/

// SGInputLoader.cxx 
// Implementation file for class SGInputLoader
/////////////////////////////////////////////////////////////////// 

#include "SGInputLoader.h"

// FrameWork includes
#include "Gaudi/Property.h"
#include "AthenaKernel/errorcheck.h"
#include "StoreGate/VarHandleKey.h"
#include "AthenaKernel/StoreID.h"

//---------------------------------------------------------------------------------


namespace
{
  struct DataObjIDSorter {
    bool operator()( const DataObjID* a, const DataObjID* b ) { return a->fullKey() < b->fullKey(); }
  };

  // Sort a DataObjIDColl in a well-defined, reproducible manner.
  // Used for making debugging dumps.
  std::vector<const DataObjID*> sortedDataObjIDColl( const DataObjIDColl& coll )
  {
    std::vector<const DataObjID*> v;
    v.reserve( coll.size() );
    for ( const DataObjID& id : coll ) v.push_back( &id );
    std::sort( v.begin(), v.end(), DataObjIDSorter() );
    return v;
  }
}

//---------------------------------------------------------------------------------

SGInputLoader::SGInputLoader( const std::string& name, 
			  ISvcLocator* pSvcLocator ) : 
  ::AthAlgorithm( name, pSvcLocator )
{
  //
  // Property declaration
  // 
  declareProperty( "Load", m_load, "create Output data dependencies for these objects")
    ->declareUpdateHandler(&SGInputLoader::loader, this);

}

//---------------------------------------------------------------------------------

SGInputLoader::~SGInputLoader()
{}

//---------------------------------------------------------------------------------

StatusCode 
SGInputLoader::initialize()
{
  ATH_MSG_VERBOSE ("Initializing " << name() << "...");

  StatusCode sc(StatusCode::SUCCESS);

  if (m_load.size() > 0) {
    std::ostringstream str;
    str << "Will explicitly preload the following DataObjects:";
    for (auto &e : m_load) {
      str << "\n    + " << e;
      if (e.key().empty()) {
        sc = StatusCode::FAILURE;
        str << "   ERROR: empty key is not allowed!";
      }
    }
    ATH_MSG_INFO(str.str());
  }

  return sc;
}

//---------------------------------------------------------------------------------

StatusCode 
SGInputLoader::finalize()
{
  ATH_MSG_INFO ("Finalizing " << name() << "...");

  return StatusCode::SUCCESS;
}

//---------------------------------------------------------------------------------

StatusCode 
SGInputLoader::execute()
{  
  StatusCode sc(StatusCode::SUCCESS);

  ATH_MSG_DEBUG ("Executing " << name() << "...");

  // add objects automatically added by the Scheduler
  if (m_first) {
    if (m_loadProxies.value()) {
      for (DataObjID obj : outputDataObjs() ) {
        // Strip any decoration name.
        std::string::size_type ppos = obj.key().find ('.');
        if (ppos < obj.key().size()-1) {
          //obj.updateKey (obj.key().substr (0, ppos));
          // commented out line above, to keep decoration on key so can distinguish undeclared decorations from undeclared objects
          // undeclared objects will be an error, undeclared decorations will just be a warning
          // TODO: restore the above modification (and make all transient proxies an error in loadObj, not just non-decorations) when all decorations are declared
        }
        m_load.emplace (std::move(obj));
      }
    }

    // check if objects are not in EventStore
    DataObjIDColl toLoad;
    for (const DataObjID* obj : sortedDataObjIDColl (m_load)) {
      // don't load anything that is in the ExtraOutputs list, which is used for objects created e.g. by the eventloopmgr
      if( extraOutputDeps().count(*obj) ) {
        ATH_MSG_DEBUG(obj->key() << " is in ExtraOutputs and will not be loaded");
        continue;
      } else if(std::string::size_type ppos = obj->key().find ('.'); ppos < obj->key().size()-1) {
        // see if the object that the decoration is on is declared as extra output. Will assume the extra output
        // will also provide such a decoration
        DataObjID objcopy(*obj);
        objcopy.updateKey(obj->key().substr (0, ppos));
        if( extraOutputDeps().count(objcopy) ) {
          ATH_MSG_DEBUG(obj->key() << "'s object/container is in ExtraOutputs and will not be loaded");
          continue;
        }
      }
      SG::VarHandleKey vhk(obj->clid(),obj->key(),Gaudi::DataHandle::Writer);
      if (StoreID::findStoreID(vhk.storeHandle().name()) == StoreID::EVENT_STORE) {
        toLoad.emplace(*obj);
      }
      else if (StoreID::findStoreID(vhk.storeHandle().name()) == StoreID::CONDITION_STORE) {
        ATH_MSG_ERROR("Unresolved conditions dependency: "
                        << *obj);
        return StatusCode::FAILURE;
      }
      else {
        ATH_MSG_DEBUG("Will not auto-load proxy for non-EventStore object: "
                        << *obj);
      }
    }
    m_load = toLoad;
    
    m_first = false;
  }

  bool b = loadObjs( m_load );

  if (m_dump.value()) {
    ATH_MSG_DEBUG(evtStore()->dump()); 
  }

  if (m_failEvt.value() && !b) {
    ATH_MSG_ERROR("autoload of objects failed. aborting event processing");
    sc = StatusCode::FAILURE;
  }

  return sc;
}

//---------------------------------------------------------------------------------

void
SGInputLoader::loader(Gaudi::Details::PropertyBase& p ) {

  ATH_MSG_DEBUG("Adding to outputs: " <<  p.toString());

  DataObjIDColl toLoad;

  for (auto obj : m_load) {
    // add an explicit storename as needed
    SG::VarHandleKey vhk(obj.clid(),obj.key(),Gaudi::DataHandle::Writer);
    obj.updateKey( vhk.objKey() );
    toLoad.emplace(obj);
    if(!outputDataObjs().count(obj)) { addDependency(obj,Gaudi::DataHandle::Writer); }
  }
  m_load = toLoad;

}

//---------------------------------------------------------------------------------

bool
SGInputLoader::loadObjs(const DataObjIDColl& objs) const {

  bool ok = true;

  for (auto &obj: objs) {

    std::string::size_type ppos = obj.key().substr(0,obj.key().size()-1).find('.');

    // use the parsing built into the VarHandleKey to get the correct
    // StoreGate key
    SG::VarHandleKey vhk(obj.clid(),obj.key().substr(0,ppos),Gaudi::DataHandle::Reader);

    ATH_MSG_DEBUG("trying to load " << obj << "   sgkey: " << vhk.key() );

    SG::DataProxy* dp = evtStore()->proxy(obj.clid(), vhk.key());
    if (dp != 0) {
      ATH_MSG_DEBUG(" found proxy for " << obj);
      if (dp->provider() == 0 && extraOutputDeps().find(obj)==extraOutputDeps().end()) {
        if(ppos==std::string::npos) {
          ATH_MSG_ERROR("   obj " << obj << " has no provider, and is only Transient - indicative of a missing output declaration" );
          ok =false;
        } else { // just warning for now for potentially undeclared decorations, instead of error, because too many cases to fix
          ATH_MSG_WARNING("   decoration " << obj << " has no provider, and is only Transient - indicative of a missing output declaration" );
        }
      }
    } else {
      ok = false;
      if (m_failEvt.value()) {
        ATH_MSG_ERROR("unable to find proxy for " << obj);
      } else {
        ATH_MSG_WARNING("unable to find proxy for " << obj);
      }
    }
  }

  return ok;

}
