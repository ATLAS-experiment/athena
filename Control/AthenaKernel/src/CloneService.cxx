/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "AthenaKernel/CloneService.h"
#include "AthenaKernel/errorcheck.h"

#include <cassert>
#include <vector>

#include "GaudiKernel/ISvcLocator.h"
#include "GaudiKernel/ISvcManager.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/Service.h"
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/System.h"


namespace CloneService {
  /** given a pointer to a parent svc sets a reference to a cloned child */
  StatusCode clone(const IService* iParent, const std::string& childName,
		   Service*& child) 
  {
    //check the parent pointer is valid
    const Service* parent = dynamic_cast<const Service*>(iParent);
    if ( !parent ) {
      throw GaudiException("parent " + iParent->name() + " is not a service!",
                           "CloneService", StatusCode::FAILURE);
      return StatusCode::FAILURE;
    }

    MsgStream mlog( parent->msgSvc(), "CloneService" );
    ISvcLocator* svcLoc = parent->serviceLocator();

    //Create the child unless it is already there. We cannot use the createIf`
    //feature of ISvcLocator because that tries to initialize the service immediately.
    if ( svcLoc->existsService(childName) ) {
      mlog << MSG::DEBUG
           << "Found service " << childName
           << endmsg;
    } else {
      // can we access the Service Manager interface?
      SmartIF<ISvcManager> svcMgr(svcLoc);
      CHECK_WITH_CONTEXT( svcMgr.isValid(), "CloneService" );

      // Create the service
      const std::string& parentType = System::typeinfoName(typeid(*parent));
      Gaudi::Utils::TypeNameString tn(childName, parentType);

      child = dynamic_cast<Service*>(svcMgr->createService(tn).get());
      if ( child ) {
        mlog << MSG::DEBUG
             << "Created service " << childName << " of type " << parentType
             << endmsg;
      } else {
        mlog << MSG::ERROR
             << "Failed to create " << childName << " of type " << parentType
             << endmsg;
        return StatusCode::FAILURE;
      }
    }

    //now copy parent's properties into child
    for (Gaudi::Details::PropertyBase* prop : parent->getProperties()) {
      if ( child->setProperty(*prop).isFailure() ) {
        mlog << MSG::ERROR
             << "Failed to set child property " << prop->name() << endmsg;
        return StatusCode::FAILURE;
      }
    }

    //finally put the service in the same state as the parent
    //FIXME should we handle the case in which the parent is RUNNING?
    if (Gaudi::StateMachine::INITIALIZED == parent->FSMState() &&
        Gaudi::StateMachine::CONFIGURED == child->FSMState())
      return child->sysInitialize(); 
    else {
      mlog << MSG::DEBUG 
           << "Did not initialize " <<  childName
           << endmsg;
      return StatusCode::SUCCESS;
    }
  }
}
