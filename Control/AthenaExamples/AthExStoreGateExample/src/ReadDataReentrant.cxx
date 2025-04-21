/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file ReadDataReentrant.h
 * @author scott snyder <snyder@bnl.gov>
 * @date Jan, 2016
 * @brief Testing reentrant algorithms.
 */


#undef NDEBUG
#include "ReadDataReentrant.h"

#include "AthExStoreGateExample/MyDataObj.h"
#include "AthContainers/DataVector.h"
#include "AthenaKernel/DefaultKey.h"
#include "AthenaKernel/errorcheck.h"
#include "AthLinks/ElementLink.h"
#include "StoreGate/SGIterator.h"
#include "StoreGate/ReadHandle.h"
#include "StoreGateExample_ClassDEF.h" /*the CLIDs for the containers*/

#include <vector>


/////////////////////////////////////////////////////////////////////////////

StatusCode ReadDataReentrant::initialize()
{
  errorcheck::ReportMessage::hideErrorLocus();

  ATH_MSG_INFO ("in initialize()");

  ATH_CHECK( m_cobjKey.initialize() );
  ATH_CHECK( m_vFloatKey.initialize() );
  ATH_CHECK( m_pLinkListKey.initialize() );
  ATH_CHECK( m_linkVectorKey.initialize() );
  ATH_CHECK( m_testObjectKey.initialize() );
  ATH_CHECK( m_eventInfo.initialize() );
  ATH_CHECK( m_dobjKeyArray.initialize() );
  ATH_CHECK( m_nonexistingKey.initialize() );

  return StatusCode::SUCCESS;
}

// * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * 

StatusCode ReadDataReentrant::execute (const EventContext& ctx) const
{

  ATH_MSG_INFO ("in execute()");

  /////////////////////////////////////////////////////////////////////
  // Part 1: retrieving individual objects from SG
  //

  SG::ReadHandle<TestDataObject> testobj (m_testObjectKey, ctx);
  if (testobj->val() != 10) std::abort();

  // Reading the array of handles.
  std::vector<SG::ReadHandle<MyDataObj> > vh = m_dobjKeyArray.makeHandles (ctx);
  for (size_t i = 0; i < vh.size(); i++) {
    assert (vh[i]->val() == static_cast<int> (100+i));
  }


  ////////////////////////////////////////////////////////////////////
  // Get the default listof MyContObj, print out its contents

  //the CLID of list<MyContObj> must be defined using the CLASSDEF
  //macros. See StoreGateExample_ClassDEF.h for a few examples
  //If no CLID is defined StoreGate assumes the object is (convertible to a)
  //DataObject. If this is not the case an error message is issued:
  //uncomment below to see how your compiler catches an undefined CLID
  //ERROR  p_SGevent->retrieve(errorH);

  SG::ReadHandle<DataVector<MyContObj> > list (m_cobjKey, ctx);
  for (const MyContObj* obj : *list) {
    float time = obj->time();
    int ID     = obj->id();
    
    ATH_MSG_INFO ("Time: " << time << "  ID: " << ID);
  }

  /////////////////////////////////////////////////////////////////////
  // Get the std::vector, print out its contents

  SG::ReadHandle<std::vector<float> > pVec (m_vFloatKey, ctx);
  for (unsigned int it=0; it<pVec->size(); it++) {
    ATH_MSG_INFO ("pVec [" << it << "] = " << (*pVec)[it]);
  }

  /////////////////////////////////////////////////////////////////////
  // test if an object is not in the store
  SG::ReadHandle<MyDataObj> nonexisting (m_nonexistingKey, ctx);
  if (nonexisting.isValid()) {
    ATH_MSG_ERROR ("event store claims it contains MyDataObj with " << m_nonexistingKey);
    return StatusCode::FAILURE;
  } else {
    ATH_MSG_INFO ("event store does not contain MyDataObj with " << m_nonexistingKey);
  }	

  /////////////////////////////////////////////////////////////////////
  // Part 2: retrieving DataLinks

  // Get the list of links, print out its contents

  typedef ElementLink<std::vector<float> > VecElemLink;
  auto pList = SG::makeHandle (m_pLinkListKey, ctx);
  for (const VecElemLink& l : *pList) {
    ATH_MSG_INFO ("ListVecLinks::linked element " << *l);
  }

  // Get the vector of links, print out its contents
  typedef ElementLink<MapStringFloat> MapElemLink;
  SG::ReadHandle<std::vector<MapElemLink> > vectorHandle (m_linkVectorKey, ctx);
  for (const MapElemLink& l : *vectorHandle) {
    ATH_MSG_INFO 
      ("VectorMapLinks::linked element: key " << l.index()
       << " - value " << (*l) 
       << " - stored as " << l);
  }

  return StatusCode::SUCCESS;
}
