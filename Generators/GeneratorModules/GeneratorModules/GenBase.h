/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GENERATORMODULES_GENBASE_H
#define GENERATORMODULES_GENBASE_H

#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/IPartPropSvc.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "AthenaKernel/errorcheck.h"
#include "CxxUtils/checker_macros.h"
#include "StoreGate/ReadHandleKey.h"
#include "GeneratorObjects/McEventCollection.h"

#include "AtlasHepMC/GenEvent.h"
#include "AtlasHepMC/GenVertex.h"
#include "TruthUtils/MagicNumbers.h"

#include "HepPDT/ParticleData.hh"
#include "HepPDT/ParticleDataTable.hh"

#include <string>
#include <vector>
#include <map>


/// @class GenBase
/// @brief Base class for common behaviour of MC truth algorithms.
///
/// This class is the base class used to specify the behavior of all
/// MC truth algorithms and is meant to capture the common behavior
/// of these modules. GenBase inherits the AthAlgorithm interface.
/// Event generator interfaces should inherit from the derived GenModule
/// interface, which adds more generation-specific functionality.
///
/// The common behavior currently consists of:
///   - Standard access to the event collection and its first member
///   - Providing access to the HepPDT ParticleProperties Table
///   - Including standard HepMC and Athena framework headers and CMT uses
///
/// The following virtual methods should be overloaded in the child class:
///
/// @author A. Buckley: Creating GenBase from GenModule, Nov 2013.
///
class GenBase : public AthAlgorithm {
public:

  /// @name Construction/destruction
  //@{

  /// Constructor
  GenBase(const std::string& name, ISvcLocator* pSvcLocator);

  /// Virtual destructor
  virtual ~GenBase() { }

  //@}


  /// @name Event loop algorithm methods
  //@{
  virtual StatusCode initialize() override;
  virtual StatusCode execute(const EventContext&) override { return StatusCode::SUCCESS; }
  //@}


  /// @name Event collection accessors (const and non-const)
  //@{

  /// Access the current signal event (first in the McEventCollection)
  ///
  /// @note This function will make a new McEventCollection
  /// if there is not already a valid one _and_ MakeMcEvent=True.
  HepMC::GenEvent* event ATLAS_NOT_CONST_THREAD_SAFE () {
    if (events()->empty())
      ATH_MSG_ERROR("McEventCollection is empty during first event access");
    return *(events()->begin());
  }

  /// Access the current signal event (const)
  const HepMC::GenEvent* event_const( const EventContext& ctx ) const {
    const McEventCollection* coll = events_const(ctx);
    if (coll->empty())
      ATH_MSG_ERROR("Const McEventCollection is empty during first event access");
    return *(coll->begin());
  }

  /// @brief Access the current event's McEventCollection
  ///
  /// @note This function will make a new McEventCollection
  /// if there is not already a valid one _and_ MakeMcEvent=True.
  McEventCollection* events ATLAS_NOT_CONST_THREAD_SAFE ();

  /// Access the current event's McEventCollection (const)
  const McEventCollection* events_const( const EventContext& ctx ) const {
    SG::ReadHandle<McEventCollection> ret = SG::makeHandle(m_mcevents_const, ctx);
    if (!ret.isValid())
      ATH_MSG_ERROR("No McEventCollection found in StoreGate with key " << m_mcevents_const.key());
    return ret.cptr();
  }

  //@}


  /// @name Particle data accessors
  //@{

  /// Access the particle property service
  const ServiceHandle<IPartPropSvc> partPropSvc() const {
    return m_ppSvc;
  }

  /// Get a particle data table
  const HepPDT::ParticleDataTable& particleTable() const {
    return *(m_ppSvc->PDT());
  }

  /// Shorter alias to get a particle data table
  const HepPDT::ParticleDataTable& pdt() const { return particleTable(); }

  /// Access an element in the particle data table
  const HepPDT::ParticleData* particleData(int pid) const {
    return pdt().particle(HepPDT::ParticleID(std::abs(pid)));
  }
  //@}

protected:

  /// @name Properties
  //@{
  /// StoreGate key for the MC event collection (defaults to GEN_EVENT)
  std::string m_mcEventKey{};
  /// Flag to determine if a new MC event collection should be made if it doesn't exist
  BooleanProperty m_mkMcEvent{this, "MakeMcEvent", false, "Create a new MC event collection if it doesn't exist"};
  //@}

private:

  /// Handle on the particle property service
  ServiceHandle<IPartPropSvc> m_ppSvc{this, "PartPropSvc", "PartPropSvc"};

  /// Const handle to the MC event collection
  SG::ReadHandleKey<McEventCollection> m_mcevents_const{ this, "McEventKey", "GEN_EVENT", "StoreGate key of the MC event collection" };

};


#endif
