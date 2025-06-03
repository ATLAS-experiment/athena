/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_BASESIMULATIONSVC_H
#define ISF_BASESIMULATIONSVC_H 1

// STL includes
#include <string>

// FrameWork includes
#include "AthenaKernel/IOVSvcDefs.h"
#include "GaudiKernel/ServiceHandle.h"
#include <GaudiKernel/StatusCode.h>
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/IChronoStatSvc.h"
#include "AthenaBaseComps/AthService.h"
#include "StoreGate/StoreGateSvc.h"

#include "GeneratorObjects/McEventCollection.h"

// ISF includes
#include "ISF_Interfaces/ISimulationSvc.h"
#include "ISF_Event/ISFParticle.h"
#include "ISF_Event/SimSvcID.h"

namespace ISF {

  class IParticleBroker;
  class ITruthSvc;

  /** @class BaseSimulationSvc

      Concrete base class for all simulation services.

      It facilitates the use of event store and detector store, provides record and retrieve
      methods and initializes the ChronoStatSvc.

      @author Michael.Duehrssen -at- cern.ch, Andreas.Salzburger -at- cern.ch, Elmar.Ritsch -at- cern.ch
  */
  class BaseSimulationSvc : public extends<AthService, ISimulationSvc> {
  public:

    //** Constructor with parameters */
    BaseSimulationSvc( const std::string& name, ISvcLocator* pSvcLocator):
      base_class(name,pSvcLocator) {};

    /** Destructor */
    virtual ~BaseSimulationSvc() {};

    /** Gaudi sysInitialize() methods */
    virtual StatusCode sysInitialize() override
    {
      if ( AthService::sysInitialize().isFailure() ) {
        ATH_MSG_FATAL( m_screenOutputPrefix << " Cannot initialize AthService! Abort.");
        return StatusCode::FAILURE;
      }
      if ( m_chrono.retrieve().isFailure()){
        ATH_MSG_FATAL( m_screenOutputPrefix << " Cannot retrieve ChronoStatSvc! Abort.");
        return StatusCode::FAILURE;
      }

      return StatusCode::SUCCESS;
    }

    /** Return the simulation service descriptor */
    const std::string& simSvcDescriptor() override { return m_simDescr.value(); }

    /** Setup Event chain - in case of a begin-of event action is needed */
    virtual StatusCode setupEvent() override
    { return StatusCode::SUCCESS; }

    /** Release Event chain - in case of an end-of event action is needed */
   virtual  StatusCode releaseEvent() override
    { return StatusCode::SUCCESS; }

    /** Inform the SimulationSvc about the ParticleBroker svc */
    virtual StatusCode setParticleBroker( IParticleBroker *broker) override {
      m_particleBroker = broker;
      return StatusCode::SUCCESS;
    }

    /** Simulation call for vectors of particles */
    virtual StatusCode simulateVector(const ISFParticleVector& particles, McEventCollection* mcEventCollection, McEventCollection *) override {
      // this implementation is a wrapper in case the simulator does
      // implement particle-vector input
      StatusCode sc = StatusCode::SUCCESS;
      // simulate each particle individually
      for (ISF::ISFParticle* part : particles) {
        ATH_MSG_VERBOSE( m_screenOutputPrefix <<  "Starting simulation of particle: " << part );
        if ( this->simulate(*part, mcEventCollection).isFailure()) {
          ATH_MSG_WARNING("Simulation of particle failed!" << endmsg <<
                          "   -> simulator: " << this->simSvcDescriptor() <<
                          "   -> particle : " << *part );
          sc = StatusCode::FAILURE;
        }
      }
      return sc;
    }

    /** Simulation call for individual particles */
    virtual StatusCode simulate(ISFParticle& isp, McEventCollection* mcEventCollection) override;

    /** wrapper call to start chrono with given tag */
    const ChronoEntity* chronoStart(const IChronoSvc::ChronoTag& tag ) {
      if (m_chrono) return m_chrono->chronoStart( tag);
      return nullptr;
    }

    /** wrapper call to stop chrono with given tag */
    const ChronoEntity* chronoStop(const IChronoSvc::ChronoTag& tag ) {
      if (m_chrono) return m_chrono->chronoStop( tag);
      return nullptr;
    }

    /** @brief The standard @c StoreGateSvc (event store)
     * Returns (kind of) a pointer to the @c StoreGateSvc
     */
    const ServiceHandle<StoreGateSvc>& evtStore() const {return m_evtStore;};
    ServiceHandle<StoreGateSvc>& evtStore() {return m_evtStore;};

    /** @brief The standard @c StoreGateSvc/DetectorStore
     * Returns (kind of) a pointer to the @c StoreGateSvc
     */
    const ServiceHandle<StoreGateSvc>& detStore() const {return m_detStore;};
    ServiceHandle<StoreGateSvc>& detStore() {return m_detStore;};

    /** templated Tool retrieval - gives unique handling & look and feel */
    template <class T> StatusCode retrieveTool(ToolHandle<T>& thandle){
      if (!thandle.empty() && thandle.retrieve().isFailure()){
        ATH_MSG_FATAL( m_screenOutputPrefix << "Cannot retrieve " << thandle << ". Abort.");
        return StatusCode::FAILURE;
      } else
        ATH_MSG_DEBUG(m_screenOutputPrefix << "Successfully retrieved " << thandle);
      return StatusCode::SUCCESS;
    }

    /** templated Tool retrieval - gives unique handling & look and feel */
    template <class T>
    StatusCode retrieveTools(ToolHandleArray<T>& thandleArray){
      if (!thandleArray.empty() && thandleArray.retrieve().isFailure()){
        ATH_MSG_FATAL( m_screenOutputPrefix << "Cannot retrieve " << thandleArray << ". Abort.");
        return StatusCode::FAILURE;
      } else
        ATH_MSG_DEBUG(m_screenOutputPrefix << "Successfully retrieved " << thandleArray);
      return StatusCode::SUCCESS;
    }

    /** templated record collection method, will create a new one if not existing */
    template<class T>
    StatusCode recordCollection( T*& coll, const std::string& collName) const{
      // create if necessary
      coll = coll ? coll : new T;
      // record
      if (evtStore()->record( coll, collName).isFailure()){
        ATH_MSG_FATAL( m_screenOutputPrefix << "Cannot record collection " <<  collName << ". Abort." );
        return StatusCode::FAILURE;
      } else
        ATH_MSG_DEBUG(m_screenOutputPrefix << "Successfully recorded collection " << collName);
      return StatusCode::SUCCESS;
    }

    /** templated retrieve collection method, boolean steers that force break */
    template<class T>
    StatusCode retrieveCollection(T*& coll, const std::string& collName, bool forceBreak=true) const {
      // check for existence in the soft case
      if (!forceBreak && !evtStore()->contains<T>(collName)) {
        coll = 0;
        ATH_MSG_DEBUG(m_screenOutputPrefix << "Collection does not exists (not required). Ignore.");
        return StatusCode::SUCCESS;
      }
      if ( evtStore()->retrieve(coll, collName).isFailure()){
        ATH_MSG_FATAL( m_screenOutputPrefix << "Cannot retireve collection " <<  collName << ". Abort." );
        return StatusCode::FAILURE;
      } else
        ATH_MSG_DEBUG(m_screenOutputPrefix << "Successfully retrieved collection " << collName);
      return StatusCode::SUCCESS;
    }

  private:
    /** Default constructor */
    BaseSimulationSvc();

    /// Handle to StoreGate (event store by default)
    ServiceHandle<StoreGateSvc> m_evtStore{this, "EvtStore", "StoreGateSvc/StoreGateSvc",
      "Handle to a StoreGateSvc instance: it will be used to retrieve data during the course of the job"};

    /// Handle to StoreGate (detector store by default)
    ServiceHandle<StoreGateSvc> m_detStore{this, "DetStore", "StoreGateSvc/DetectorStore",
      "Handle to a StoreGateSvc/DetectorStore instance: it will be used to retrieve data during the course of the job"};


  protected:
    /** The simulator service descriptor */
    Gaudi::Property<std::string> m_simDescr{this, "Identifier", {},
      "A unique string to identify the simulator."};

    /** Screen output refinement */
    Gaudi::Property<std::string> m_screenOutputPrefix{this, "ScreenOutputPrefix", "isf >> ",
      "Prefix for log output"};

    /** The timing service for general usage */
    ServiceHandle<IChronoStatSvc>   m_chrono{this, "ChronoStatService", "ChronoStatSvc"};

    /** The particle service used to push particles into the simulation */
    IParticleBroker*  m_particleBroker{};

  };


  /** Simulation Call --- hand over to the particleProcessor if it exists */
  inline StatusCode BaseSimulationSvc::simulate(ISFParticle& /*isp*/, McEventCollection*)
  {
    return StatusCode::SUCCESS;
  }
}


#endif //> !ISF_BASESIMULATIONSVC_H
