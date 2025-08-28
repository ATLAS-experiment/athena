/* -*- C++ -*- */

/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CONTROL_STOREGATE_SEGMEMSVC_H
#define CONTROL_STOREGATE_SEGMEMSVC_H 1

#include "GaudiKernel/Service.h"
#include "GaudiKernel/IIncidentListener.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthAllocators/Arena.h"

#include <string>


/** @class SegMemSvc
 * @brief Service to access an arena caching allocator
 *
 * @param  No user settable jobOptions
 *      
 * @author Charles Leggett
 **/

class SegMemSvc: public extends<Service, IIncidentListener> {

public:

  
  enum LIFETIME {
    EVENT = 0,
    JOB = 1,
    INCIDENT = 2
  };
  
  virtual StatusCode initialize() override;
  virtual void handle(const Incident&) override;

  ////////////////////////////////////////////////////////////////////////
  /// \name Client Interface: allocate and free memory
  //@{


  /// @brief Get pointer to an area in the arena, used for new with placement
  /// @param LIFETIME object lifetime in the arena: EVENT, JOB or INCIDENT
  /// @return T* to be used with new with placement
  template <typename T> 
  T* allocate(SegMemSvc::LIFETIME, bool cache=true);
  
  /// free memory managed by arena
  template <typename T>
  bool del(T*);

  //@}
  
  
  SegMemSvc( const std::string& name, ISvcLocator* svc ); 
  
  // Destructor.
  virtual ~SegMemSvc();
  
  
private:
  
  ServiceHandle<IIncidentSvc> p_incSvc;

  /// helper to for printout
  inline std::string cvt(SegMemSvc::LIFETIME life) {
    switch  (life) {
    case SegMemSvc::EVENT : return "EVENT";
    case SegMemSvc::JOB : return "JOB";
    case SegMemSvc::INCIDENT: return "INCIDENT";
    }
    return "UNKNOWN";
  }

  /// three arenas for different lifetimes
  SG::ArenaHeader m_ahead_job, m_ahead_evt, m_ahead_inc;
  SG::Arena       m_arena_job, m_arena_evt, m_arena_inc;


};


#include "StoreGate/SegMemSvc.icc"

#endif
