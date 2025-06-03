/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef ISF_SIMULATIONSELECTORS_BASESIMULATIONSELECTOR_H
#define ISF_SIMULATIONSELECTORS_BASESIMULATIONSELECTOR_H 1

// ISF includes
#include "ISF_Interfaces/ISimulationSelector.h"
#include "ISF_Interfaces/ISimulationSvc.h"
#include "ISF_Interfaces/SimulationFlavor.h"


// Gaudi & Athena basics
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaBaseComps/AthAlgTool.h"

#include "CxxUtils/checker_macros.h"


namespace ISF {

class ISFParticle;

/**
 @class BaseSimulationSelector

 @author Elmar.Ritsch -at- cern.ch , Peter.Sherwood -at- cern.ch
 */

class BaseSimulationSelector : public extends<AthAlgTool, ISimulationSelector> {
  public:
    /** Constructor with parameters */
    BaseSimulationSelector( const std::string& t, const std::string& n, const IInterface* p );

    /** virtual destructor */
    virtual ~BaseSimulationSelector();

    /** Gaudi sysInitialize() method */
    virtual StatusCode sysInitialize() override;

    /** return a handle on the simulator */
    virtual ServiceHandle<ISimulationSvc>* simulator ATLAS_NOT_THREAD_SAFE () override;

    /** return if is this a static or dynamic SimulationSelector
        (is used by fully dynamic particle routers) */
    virtual bool isDynamic() override;

    /** return the simulation service ID */
    virtual SimSvcID simSvcID ATLAS_NOT_THREAD_SAFE () override;

    /** return the simulation flavor */
    virtual ISF::SimulationFlavor simFlavor() const override;

    /** initialize Selector */
    virtual void initializeSelector() override;

    /** called at the beginning of each athena event
        (can be used for eg. resetting dynamic selectors) */
    virtual void beginEvent ATLAS_NOT_THREAD_SAFE () override;

    /** called at the end of each athena event
        (can be used for eg. resetting dynamic selectors) */
    virtual void endEvent ATLAS_NOT_THREAD_SAFE () override;

    /** update internal event representation */
    virtual void update ATLAS_NOT_THREAD_SAFE (const ISFParticle& ) override;

    /** make the routing decision */
    virtual bool selfSelect(const ISFParticle& particle) const override;

  private:
    ServiceHandle<ISimulationSvc>       m_simulator{this, "Simulator", "", "simulation service assigned to a single advisor"};
    Gaudi::Property<bool>               m_isDynamic{this, "IsDynamic", false, "this selector is either dynamic or static"};
    Gaudi::Property<bool>               m_invertCuts{this, "InvertCuts", false, "invert the result given by passesCuts(..) method"};
    Gaudi::CheckedProperty<unsigned short> m_simFlavorProp{this, "SimulationFlavor", 0, "the simulation flavour that this selector will select"};
    void SimulationFlavorHandler(Gaudi::Details::PropertyBase&);
    ISF::SimulationFlavor               m_simflavor{ISF::UndefinedSim};  //!< simulation flavor
};


} // end of namespace


#endif // ISF_SIMULATIONSELECTORS_BASESIMULATIONSELECTOR_H
