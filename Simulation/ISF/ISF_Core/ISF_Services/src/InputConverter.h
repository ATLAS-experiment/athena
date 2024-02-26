/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_INPUTCONVERTER_H
#define ISF_INPUTCONVERTER_H 1

// STL includes
#include <string>
// ISF include
#include "ISF_Interfaces/IInputConverter.h"
// FrameWork includes
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthService.h"

#include "BarcodeEvent/Barcode.h"

namespace HepPDT {
  class ParticleDataTable;
}
#include "AtlasHepMC/GenEvent_fwd.h"
#include "AtlasHepMC/GenParticle_fwd.h"
#include "TruthUtils/MagicNumbers.h"

class IPartPropSvc;
class McEventCollection;
namespace ISFTesting {
  class InputConverter_test;
}
namespace ISF {
  class ISFParticle;
  class IGenParticleFilter;
}

class G4ParticleDefinition;
class G4PrimaryParticle;
class G4VSolid;

namespace ISF {

  /** @class InputConverter

      Convert simulation input collections to ISFParticles for subsequent ISF simulation.

      @author Elmar.Ritsch -at- cern.ch
     */
  class InputConverter final: public extends<AthService, IInputConverter> {

    // allow test to access private data
    friend ISFTesting::InputConverter_test;

  public:
    InputConverter(const std::string& name, ISvcLocator* svc);
    virtual ~InputConverter();

    /** Athena algtool Hooks */
    virtual StatusCode  initialize() override final;
    virtual StatusCode  finalize() override final;

    /** Convert selected particles from the given McEventCollection into ISFParticles
        and push them into the given ISFParticleContainer */
    virtual StatusCode convert(McEventCollection& inputGenEvents,
                               ISF::ISFParticleContainer& simParticles) const override final;

    /** */
    virtual StatusCode convertHepMCToG4Event(McEventCollection& inputGenEvents,
                                             G4Event*& outputG4Event, McEventCollection& shadowGenEvents) const override final;

    virtual StatusCode convertHepMCToG4EventLegacy(McEventCollection& inputGenEvents,
                                             G4Event*& outputG4Event) const override final;

    /** Converts vector of ISF::ISFParticles to G4Event */
    G4Event* ISF_to_G4Event(const std::vector<ISF::ISFParticle*>& isp, HepMC::GenEvent *genEvent, HepMC::GenEvent *shadowGenEvent=nullptr, bool useHepMC=false) const override final;

  private:

    const G4ParticleDefinition* getG4ParticleDefinition(int pdgcode) const;

#ifdef HEPMC3
    G4PrimaryParticle* getDaughterG4PrimaryParticle(const HepMC::ConstGenParticlePtr& gp) const;
    G4PrimaryParticle* getDaughterG4PrimaryParticle(const HepMC::GenParticlePtr& gp, bool makeLinkToTruth=true) const;
#else
    G4PrimaryParticle* getDaughterG4PrimaryParticle(HepMC::GenParticle& gp, bool makeLinkToTruth=true) const;
#endif

    G4PrimaryParticle* getG4PrimaryParticle(ISF::ISFParticle& isp, bool useHepMC, HepMC::GenEvent *shadowGenEvent) const;

    void addG4PrimaryVertex(G4Event* g4evt, ISF::ISFParticle& isp, bool useHepMC, HepMC::GenEvent *shadowGenEvent) const;

#ifdef HEPMC3
    void processPredefinedDecays(const HepMC::ConstGenParticlePtr& genpart, ISF::ISFParticle& isp, G4PrimaryParticle* g4particle) const;
#endif
    void processPredefinedDecays(const HepMC::GenParticlePtr& genpart, ISF::ISFParticle& isp, G4PrimaryParticle* g4particle, bool makeLinkToTruth=true) const;

    bool matchedGenParticles(const HepMC::ConstGenParticlePtr& p1,
                             const HepMC::ConstGenParticlePtr& p2) const;

    HepMC::GenParticlePtr findShadowParticle(const HepMC::ConstGenParticlePtr& genParticle, HepMC::GenEvent *shadowGenEvent) const;

    /** Tests whether the given ISFParticle is within the Geant4 world volume */
    bool isInsideG4WorldVolume(const ISF::ISFParticle& isp, const G4VSolid* worldSolid) const;

    /** get right GenParticle mass */
#ifdef HEPMC3
    double getParticleMass(const HepMC::ConstGenParticlePtr& p) const;
#else
    double getParticleMass(const HepMC::GenParticle& p) const;
#endif

    /** get all generator particles which pass filters */
    std::vector<HepMC::GenParticlePtr > getSelectedParticles(HepMC::GenEvent& evnt, bool legacyOrdering=false) const;

    /** check if the given particle passes all filters */
#ifdef HEPMC3
    bool passesFilters(const HepMC::ConstGenParticlePtr& p) const;
#else
    bool passesFilters(const HepMC::GenParticle& p) const;
#endif

    /** convert GenParticle to ISFParticle */
    ISF::ISFParticle* convertParticle(const HepMC::GenParticlePtr& genPartPtr) const;

    /** ParticlePropertyService and ParticleDataTable */
    ServiceHandle<IPartPropSvc>           m_particlePropSvc;          //!< particle properties svc to retrieve PDT
    const HepPDT::ParticleDataTable      *m_particleDataTable;        //!< PDT used to look up particle masses

    bool                                  m_useGeneratedParticleMass; //!< use GenParticle::generated_mass() in simulation

    ToolHandleArray<IGenParticleFilter>   m_genParticleFilters;       //!< HepMC::GenParticle filters

    bool                                  m_quasiStableParticlesIncluded; //<! will quasi-stable particles be included in the simulation
    BooleanProperty m_useShadowEvent{this, "UseShadowEvent", false, "New approach to selecting particles for simulation" };

  };

}


#endif //> !ISF_INPUTCONVERTER_H
