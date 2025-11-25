/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef G4FASTSIMULATION_PUNCHTHROUGHSIMWRAPPER_H
#define G4FASTSIMULATION_PUNCHTHROUGHSIMWRAPPER_H

// Base classes
#include "AthenaBaseComps/AthAlgTool.h"
#include "G4AtlasInterfaces/IPunchThroughSimWrapper.h"

// Geant4 Punchthrough G4 Tool
#include "G4AtlasInterfaces/IPunchThroughG4Tool.h"
#include "G4AtlasInterfaces/IPunchThroughG4Classifier.h"

//Geant4
#include "G4ParticleTable.hh"
#include "G4FastStep.hh"
#include "G4FastTrack.hh"

/** @class PunchThroughSimWrapper PunchThroughSimWrapper.h
 *
 *  @brief Class to wrap PunchThrough simulation inside FastCaloSim; Runs both
 *   PunchThroughG4Classifier and PunchThroughG4Tool methods
 *
 * @author  Elmar Ritsch <Elmar.Ritsch@cern.ch>
 * @maintainer/updater Thomas Carter <thomas.michael.carter@cern.ch>
 * @maintainer/updater Firdaus Soberi <firdaus.soberi@cern.ch>
 */

class PunchThroughSimWrapper : public extends<AthAlgTool, IPunchThroughSimWrapper>
{
  public:
    /** Constructor */
    PunchThroughSimWrapper(const std::string&,const std::string&,const IInterface*);

    /** Destructor */
    virtual ~PunchThroughSimWrapper() = default;

    /** AlgTool initialize method */
    virtual StatusCode initialize();
    /** AlgTool finalize method */
    virtual StatusCode finalize();

    // public function exposed to interface
    virtual void DoPunchThroughSim(G4ParticleTable &ptable, CLHEP::HepRandomEngine* rng, const double simE, std::vector<double> simEfrac, const G4FastTrack& fastTrack, G4FastStep& fastStep);

 private:
    /*---------------------------------------------------------------------
     *  Private member functions
     *---------------------------------------------------------------------*/
    PublicToolHandle<IPunchThroughG4Tool>       m_PunchThroughG4Tool{this, "PunchThroughG4Tool", "PunchThroughG4Tool", ""};
    PublicToolHandle<IPunchThroughG4Classifier> m_PunchThroughG4Classifier{this, "PunchThroughG4Classifier", "PunchThroughG4Classifier", ""};
};

#endif
