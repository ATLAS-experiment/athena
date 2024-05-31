/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_SERVICES_PARTICLEBROKERDYNAMICONREADIN_H
#define ISF_SERVICES_PARTICLEBROKERDYNAMICONREADIN_H 1

// STL includes
#include <string>
#include <set>

// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"
#include "GaudiKernel/ITHistSvc.h"

// ISF includes
#include "ISF_Interfaces/IParticleBroker.h"
#include "ISF_Interfaces/IParticleOrderingTool.h"
#include "ISF_Interfaces/IEntryLayerTool.h"
#include "ISF_Interfaces/IGeoIDSvc.h"
#include "ISF_Interfaces/SimulationFlavor.h"
#include "ISF_Event/EntryLayer.h"
#include "ISF_Event/ISFParticleContainer.h"
#include "ISF_Event/ISFParticleOrderedQueue.h"

// AtlasDetDescr
#include "AtlasDetDescr/AtlasRegion.h"

// forward declarations
namespace PMonUtils {
  class CustomBenchmark;
}
// ROOT forward declarations
class TTree;

namespace ISF {

  class ISimulationSelector;

  typedef std::vector<ISimulationSelector*>      SimSelectorArray;
  typedef std::set<ISimulationSelector*>         SimSelectorSet;

  /** @class ParticleBrokerDynamicOnReadIn

      ISF particle broker responsible to flag particles with their corresponding
      simulator. On detector boundaries, simulators hand back all particles
      that pass them via the push() method.

      @author Andreas.Salzburger -at- cern.ch , Elmar.Ritsch -at- cern.ch
     */
  class ATLAS_NOT_THREAD_SAFE ParticleBrokerDynamicOnReadIn : public extends<AthService, IParticleBroker> {
    public:

      //** Constructor with parameters */
      ParticleBrokerDynamicOnReadIn( const std::string& name, ISvcLocator* pSvcLocator );

      /** Destructor */
      virtual ~ParticleBrokerDynamicOnReadIn();

      /** Athena algorithm's interface methods */
      StatusCode  initialize();

      /** Initialize the particle broker */
      StatusCode initializeEvent(ISFParticleContainer&& simParticles);

      /** Finalize the event in the broker service */
      virtual StatusCode finalizeEvent();

      /** add a new particle to the stack and inherit ISF properties from the given
          ancestor ISF particel (usually the particle which ISF handed over to the
          simulator) */
      virtual void push( ISFParticle *particle, const ISFParticle *ancestor);

      /** Get vectors of ISF particles from the broker */
      virtual const ISFParticleVector& popVector(size_t maxVectorSize);

      /** Get the current stack size */
      virtual size_t numParticles() const;

      /** Return the particle stack (not implemented) */
      //const ParticleContainer& particleStack() const;

      /* Dump the stack to the screen */
      StatusCode dump() const;

      /** Register an array of SimulationSelectors */
      StatusCode registerSimSelector(SimSelectorToolArray &simSelectorTools, AtlasDetDescr::AtlasRegion geoID);

    private:
      /** Default constructor */
      ParticleBrokerDynamicOnReadIn();

      /** setup the ROOT TTrees for THistSvc in validation mode */
      StatusCode registerPosValTree( const char *treeName,
                                     const char *treeDescr,
                                     TTree      *&tree );

      /** fill the TTree with the (x,y,z) coordinates of the given ISFParticle */
      void fillPosValTree( TTree *tree, const ISFParticle &particle);

      /** update all unfrozen simSelectors with the given particle */
      void updateAllSelectors(const ISFParticle &particle);

      /** go through the chain of SimulationSelectors and let the first one which selects the particle
          decide which simulation the particle will be assigned to */
      void selectAndStore( ISFParticle* p);

      /** go through the chain of SimulationSelectors and return the SimulatoID of the first
          SimulationSelector that selects the particle */
      ISF::SimSvcID identifySimID( const ISF::ISFParticle* p);

      /** AthenaTool responsible for writing Calo/Muon Entry/Exit Layer collection */
      PublicToolHandle<IEntryLayerTool> m_entryLayerTool{this, "EntryLayerTool", "iGeant4::EntryLayerTool/ISF_EntryLayerTool"};

      /** AthenaTool responsible for proritizing the particles and determine their simulation order */
      PublicToolHandle<IParticleOrderingTool> m_orderingTool{this, "ParticleOrderingTool", ""};

      /** the geo identifier service used to route the particle into the right
          SimulationSelector chain */
      ServiceHandle<IGeoIDSvc> m_geoIDSvc{this, "GeoIDSvc", ""};
      IGeoIDSvc *m_geoIDSvcQuick{};         //!< minimize Gaudi overhead

      /** always use GeoIDSvc to determine GeoID of given particles */
      BooleanProperty m_forceGeoIDSvc{this, "AlwaysUseGeoIDSvc", false};

      /** always use GeoIDSvc to check correctness of GeoIDs already assigned to particles */
      BooleanProperty m_validateGeoID{this, "ValidateGeoIDs", false};

      /** the particle container storing all particles which need to be simulated */
      ISFParticleOrderedQueue                   m_particles;

      /** the vector of particles returned for simulation (via popVector() ) */
      ISFParticleVector                         m_popParticles;

      /** the simulation selectors per geoID (the actual routing chain) */
      SimSelectorArray                          m_simSelector[AtlasDetDescr::fNumAtlasRegions]; //!< selectors per geoID
      SimSelectorSet                            m_simSelectorSet; //!< used to remove multiple uses of the same selector

      /** Screen output refinement */
      std::string                               m_screenOutputPrefix{"isf >> "};
      std::string                               m_screenEmptyPrefix{""};

       /** Benchmarking */
      BooleanProperty m_doSelectorCPUMon{this, "SimSelectorCPUMonitoring", false}; //!< whether we use PMonUtils or not
      PMonUtils::CustomBenchmark               *m_benchPDGCode{};
      PMonUtils::CustomBenchmark               *m_benchGeoID{};

      /** validation mode: create ROOT Tree with additional information */
      BooleanProperty m_validationOutput{this, "ValidationOutput", false, "If turned on, write out a ROOT tree."}; //!< turn validation mode on/off
      ServiceHandle<ITHistSvc> m_thistSvc{this, "THistService", "THistSvc"};         //!< the histogram service
      StringProperty m_validationStream{this, "ValidationStreamName", "ParticleBroker"}; //!< validation THist stream name
      /** the actual validation histograms and ROOT trees */
      TTree                                    *m_t_pushPosition{};
      TTree                                    *m_t_entryLayerPos[ISF::fNumAtlasEntryLayers];
      /** memory containing the entries for the ROOT tree */
      int m_val_pdg{0};
      float  m_val_p{0.f};
      float  m_val_x{0.f};
      float m_val_y{0.f};
      float m_val_z{0.f};
  };

  /** Get the current stack size */
  inline size_t ParticleBrokerDynamicOnReadIn::numParticles() const { return m_particles.size(); }

  /** Return the particle stack (not implemented) */
  /* inline const ParticleContainer& ParticleBrokerDynamicOnReadIn::particleStack() const
  {
    // @TODO:
    //ATH_MSG_WARNING("TODO: the ParticleBrokerDynamicOnReadIn::particleStack() method only returns the active part of the partilce stack");
    return m_activeStack;
  } */

}

#endif //> !ISF_SERVICES_PARTICLEBROKERDYNAMICONREADIN_H
