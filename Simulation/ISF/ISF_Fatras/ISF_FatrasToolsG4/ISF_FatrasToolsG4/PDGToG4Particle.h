/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
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

  static const InterfaceID IID_PDGToG4Particle("PDGToG4Particle", 1, 0);

  /** @class PDGToG4Particle

      AlgTool to convert a pdgCode into a particle definition used by the G4 decayer

      @author Joerg.Mechnich -at- cern.ch, Andreas.Salzburger -at- cern.ch
  */

  class PDGToG4Particle : public AthAlgTool
  {
  public:
    /** Default constructor */
    PDGToG4Particle( const std::string&,
                     const std::string&,
                     const IInterface*);

    virtual ~PDGToG4Particle() = default;

    /** AlgTool initailize method.*/
    StatusCode initialize();

    /** AlgTool interface methods */
    static const InterfaceID& interfaceID() { return IID_PDGToG4Particle; }

    /**
       Returns the G4ParticleDefinition of particle with PDG ID pdgCode,
       0 otherwise.
    */
    virtual G4ParticleDefinition* getParticleDefinition( int pdgCode) const;

    /**
       returns a vector of pdgid / particlename pairs containing all particles
    */
    virtual std::vector<std::pair<int,std::string> > listOfParticles() const;

    /** prints list of particles to stdout */
    virtual void printListOfParticles( bool withDecayTableOnly=false) const;

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
    /** List of particles which should be available for conversion */
    std::vector<int> m_useParticles;

    /** Print list of loaded particles in initialize() */
    bool m_printList;
  };
}

#endif // FATRASG4TOOLS_PDGTOG4PARTICLE_H
