/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_INTERFACES_IINPUTCONVERTER_H
#define ISF_INTERFACES_IINPUTCONVERTER_H 1

// Gaudi
#include "GaudiKernel/IInterface.h"

//GeneratorObjects
#include "GeneratorObjects/HepMcParticleLink.h"

// StoreGate
#include "StoreGate/ReadHandle.h"
#include "StoreGate/WriteHandle.h"

// Simulation includes
#include "ISF_Event/ISFParticleContainer.h"

// forward declarations
class McEventCollection;
class G4Event;
#include "AtlasHepMC/GenEvent_fwd.h"

namespace ISF {

  class ISFParticle;

  /**
     @class IInputConverter

     Interface to Athena service that converts an input McEventCollection
     into a container of ISFParticles.

     @author Elmar.Ritsch -at- cern.ch
  */

  class IInputConverter : virtual public IInterface {
  public:

    /** Virtual destructor */
    virtual ~IInputConverter(){}

    /** Tell Gaudi which InterfaceID we have */
    DeclareInterfaceID( ISF::IInputConverter, 1, 0 );

    /** Convert selected particles from the given McEventCollection into ISFParticles
        and push them into the given ISFParticleContainer */
    virtual StatusCode convert(McEventCollection& inputGenEvents,
                               ISFParticleContainer& simParticles) const = 0;

    /** Convert selected particles from the given McEventCollection into G4PrimaryParticles
        and push them into the given G4Event */
    virtual StatusCode convertHepMCToG4Event(
        McEventCollection& inputGenEvents, G4Event& outputG4Event,
        McEventCollection& shadowGenEvents) const = 0;

    /** Convert selected particles from the given McEventCollection into G4PrimaryParticles
        and push them into the given G4Event */
    virtual StatusCode convertHepMCToG4EventLegacy(
        McEventCollection& inputGenEvents, G4Event& outputG4Event) const = 0;

    /** Converts vector of ISF::ISFParticles to G4Event */
    virtual void ISF_to_G4Event(G4Event& event,
                                const std::vector<ISF::ISFParticle*>& isp,
                                HepMC::GenEvent* genEvent,
                                HepMC::GenEvent* shadowGenEvent = nullptr,
                                bool useHepMC = false) const = 0;
  };

} // end of ISF namespace

#endif // ISF_INTERFACES_IINPUTCONVERTER_H
