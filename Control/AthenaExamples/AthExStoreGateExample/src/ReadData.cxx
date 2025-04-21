/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#undef NDEBUG
#include "ReadData.h"

#include <list>
#include <map>
#include <vector>
#include "AthExStoreGateExample/MyDataObj.h"
#include "MyContObj.h"
#include "MapStringFloat.h"
#include "StoreGateExample_ClassDEF.h" /*the CLIDs for the containers*/
#include "StoreGate/SGIterator.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/DefaultKey.h"
#include "AthLinks/ElementLink.h"


/////////////////////////////////////////////////////////////////////////////

StatusCode ReadData::initialize() {

  ATH_MSG_INFO ("in initialize()");

  // Print out the location of the data objects
  ATH_MSG_INFO ("Data Object producer: " << m_DataProducer.value());

  //locate the StoreGateSvc and initialize our local ptr
  ATH_CHECK (evtStore().retrieve());

  ATH_CHECK( m_dobj3.assign (m_DataProducer) );
  ATH_CHECK( m_dobj3.initialize() );
  
  ATH_CHECK( m_eventInfo.initialize() );

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode ReadData::execute() {

  ATH_MSG_INFO ("in execute()");

  /////////////////////////////////////////////////////////////////////
  // Get a specific MyDataObj by providing its key
  // (in this case the name of the algo which recorded it)

  ATH_MSG_INFO ("Get the MyDataObj recorded by " << m_DataProducer.value());

  //this time we set a *non-const* pointer. *If* we get back a valid
  //pointer we'll be able to modify the content of the underlying transient obj
  MyDataObj* dobj3 = nullptr;
  //Most objects recorded to the SG may not be retrieved
  //using a non-const handle, unless the original producer of the object 
  //allowed for later modifications invoking StoreGateSvc::record with 
  //the last "allowMods" optional argument = true 

  //  We retrieve our MyDataObj, using m_DataProducer as our key
  ATH_CHECK (evtStore()->retrieve(dobj3, m_DataProducer));
  dobj3->val(4);   // should be able to do this

  // Also check retrieving it as a const object via ReadHandle.
  assert (m_dobj3->val() == 4);

  //not every type can be used as a key.
  //These requirements are checked at compile time in the record method
  //using StoreGate/constraints/KeyConcept.h
  //uncomment to see how the key concept is checked by the compiler
  //  p_SGevent->retrieve(dobj3, 1.0);
  //concept checking does not work using egcs 1.1 (gcc 2.91)
  //(but you still get a compiler error message...)

  /////////////////////////////////////////////////////////////////////
  // iii) Get all recorded instances of MyDataObj, print out their contents

  // (a SG::ConstIterator is an iterator class that we also use to defer
  //  and control the access to the persistent object)
  SG::ConstIterator<MyDataObj> iMyD;
  SG::ConstIterator<MyDataObj> eMyD;

  ATH_CHECK (evtStore()->retrieve(iMyD, eMyD));

  // while(iMyD != eMyD) {
  //   ATH_MSG_INFO ((*iMyD).val());
  //   ++iMyD;
  // }
  ATH_MSG_WARNING ("FIXME loop of ConstIterator bombs");



  ////////////////////////////////////////////////////////////////////
  // Get the default listof MyContObj, print out its contents

  //the CLID of list<MyContObj> must be defined using the CLASSDEF
  //macros. See StoreGateExample_ClassDEF.h for a few examples
  //If no CLID is defined StoreGate assumes the object is (convertible to a)
  //DataObject. If this is not the case an error message is issued:
  //uncomment below to see how your compiler catches an undefined CLID
  //ERROR  p_SGevent->retrieve(errorH);

  DataVector<MyContObj>::const_iterator it;

  //now let's try again with a const pointer
  const DataVector<MyContObj>* list = nullptr;

  ATH_CHECK (evtStore()->retrieve(list));
  ATH_MSG_INFO ("Retrieved DataVector of MyContObj using a const pointer");

  for (const MyContObj* obj : *list) {
    float time = obj->time();
    int ID     = obj->id();
    
    ATH_MSG_INFO ("Time: " << time << "  ID: " << ID);
  }

  /////////////////////////////////////////////////////////////////////
  // Get the std::vector, print out its contents

  const std::vector<float>* pVec = nullptr;

  ATH_CHECK (evtStore()->retrieve(pVec));
  for (unsigned int it=0; it<pVec->size(); it++) {
    ATH_MSG_INFO ("pVec [" << it << "] = " << (*pVec)[it]);
  }

  // test if an object is not in the store
  if (evtStore()->contains<MyDataObj>("FunnyNonExistingKey")) {
    ATH_MSG_ERROR 
      ("event store claims it contains MyDataObj with FunnyNonExistingKey");
    return( StatusCode::FAILURE);
  } else {
    ATH_MSG_INFO
      ("event store does not contain MyDataObj with FunnyNonExistingKey");
  }	

  
  
  /////////////////////////////////////////////////////////////////////
  // Part 2: retrieving DataLinks

  // Get the list of links, print out its contents
  
  typedef ElementLink<std::vector<float> > VecElemLink;
  const std::list<VecElemLink>* pList = nullptr;

  ATH_CHECK (evtStore()->retrieve(pList));
  ATH_MSG_INFO ("Retrieved list of links");

  std::list<VecElemLink>::const_iterator itL= pList->begin();
  std::list<VecElemLink>::const_iterator endL= pList->end();

  while(itL != endL) {
    ATH_MSG_INFO ("ListVecLinks::linked element " << **itL);
    ++itL;
  }

  // Get the vector of links, print out its contents

  typedef ElementLink<MapStringFloat> MapElemLink;
  const std::vector<MapElemLink>* vectorHandle = nullptr;

  ATH_CHECK (evtStore()->retrieve(vectorHandle));
  ATH_MSG_INFO ("Retrieved vector of links");

  std::vector<MapElemLink>::const_iterator itV= vectorHandle->begin();
  std::vector<MapElemLink>::const_iterator endV= vectorHandle->end();
  while(itV != endV) {
    ATH_MSG_INFO 
      ("VectorMapLinks::linked element: key " << itV->index()
       << " - value " << (**itV) 
       << " - stored as " << *itV);
    ++itV;
  }

  //try to "read back" a link
  //    istrstream istr("due 0");
  //    MapElemLink toBeRead;
  //    istr >> toBeRead;
  //    if (!toBeRead.isValid()) {
  //      log << MSG::ERROR 
  //  	<< "Could not read back MapElement" 
  //  	<< endmsg;
  //      return( StatusCode::FAILURE);
  //    } else {
  //      log << MSG::INFO <<  "MapElement read back: key " << toBeRead->first
  //  	<< "  value " << toBeRead->second <<endmsg;
  //    }

  /////////////////////////////////////////////////////////////////////
  // Part 3: symbolic links and derived types
  // Get all objects as its base class 

  SG::ConstIterator<BaseClass> it2;
  SG::ConstIterator<BaseClass> iEnd2;

  ATH_CHECK (evtStore()->retrieve(it2, iEnd2));

  // not much we can do with an empty base class,
  // so print the path of its persistable companion (the DataBucket)
  ATH_MSG_INFO (" retrieve as BaseClass, print its address: ");
  while(it2 != iEnd2) {
    ATH_MSG_INFO (&*it2);
    ++it2;
  }
  
  /////////////////////////////////////////////////////////////////////
  // Part 4: Get the event header, print out event and run number

  SG::ReadHandle<xAOD::EventInfo> eventInfo(m_eventInfo);
  if(eventInfo.isValid()) {
    int event = eventInfo->eventNumber();
    int run = eventInfo->runNumber();
    ATH_MSG_INFO (" EventInfo : " 
		  << " event: " << event 
		  << " run: " << run);
  }
  else
  {
    ATH_MSG_ERROR (" Unable to retrieve EventInfo from StoreGate ");
    return StatusCode::SUCCESS;
  }

  return StatusCode::SUCCESS;
}

