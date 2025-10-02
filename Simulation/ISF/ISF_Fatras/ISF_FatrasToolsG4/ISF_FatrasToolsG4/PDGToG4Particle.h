/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef FATRASG4TOOLS_PDGTOG4PARTICLE_H
#define FATRASG4TOOLS_PDGTOG4PARTICLE_H

// Gaudi/Athena
#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/MsgStream.h"

// STL
#include <map>
#include <vector>
#include <string>

class G4ParticleDefinition;

namespace iFatras
{

  /** @class PDGToG4Particle

      AlgTool to convert a pdgCode into a particle definition used by the G4 decayer

      @author Joerg.Mechnich -at- cern.ch, Andreas.Salzburger -at- cern.ch
  */

  class PDGToG4Particle : public AthAlgTool
  {
  public:
    /** Constructor from base class. */
    using AthAlgTool::AthAlgTool;

    /** AlgTool initailize method.*/
    virtual StatusCode initialize() override;

    /**
       Returns the G4ParticleDefinition of particle with PDG ID pdgCode,
       0 otherwise.
    */
    G4ParticleDefinition* getParticleDefinition( int pdgCode) const;

    /**
       returns a vector of pdgid / particlename pairs containing all particles
    */
    std::vector<std::pair<int,std::string> > listOfParticles() const;

    /** prints list of particles to stdout */
    void printListOfParticles( bool withDecayTableOnly=false) const;

    typedef std::map<int,G4ParticleDefinition*> PDGG4ParticleMap;

  private:
    /*---------------------------------------------------------------------
     *  Private members
     *---------------------------------------------------------------------*/
    /** fills default particles in map */
    std::map<int,G4ParticleDefinition*> predefinedParticles();

    /** map from pdg codes to defined Geant4 particles */
    PDGG4ParticleMap m_pdgG4ParticleMap;

    /*---------------------------------------------------------------------
     *  Properties
     *---------------------------------------------------------------------*/
    Gaudi::Property<std::vector<int>> m_useParticles{this, "UseParticles", {},
      "List of particles which should be available for conversion"};

    Gaudi::Property<bool> m_printList{this, "PrintList", false,
      "Print list of loaded particles in initialize()"};

  };
}

#endif // FATRASG4TOOLS_PDGTOG4PARTICLE_H
