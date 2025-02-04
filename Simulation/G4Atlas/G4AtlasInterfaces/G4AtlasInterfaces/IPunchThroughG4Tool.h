/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPunchThroughG4Tool_H
#define IPunchThroughG4Tool_H

// C++
#include <vector>
#include <map>
#include <string>

// Gaudi
#include "GaudiKernel/IAlgTool.h"

//G4
#include "G4TrackVector.hh"

// forward declarations
class G4Track;
class G4FastTrack;
class G4FastStep;
class G4ParticleTable;

namespace CLHEP {
  class HepRandomEngine;
}

/**
  @class IPunchThroughG4Tool

   Interface for a tool to simulate punchtrough particles outgoing Calo-MS boundaries.
   
  @author thomas.michael.carter@cern.ch, firdaus.soberi@cern.ch
*/
class IPunchThroughG4Tool : virtual public IAlgTool
{
 public:
     /** AlgTool interface method, handles constructor/destructor */
    DeclareInterfaceID(IPunchThroughG4Tool, 1, 0);

    /** Creates new vector of G4Track out of a given G4Track */
    virtual std::vector<std::map<std::string, double>> computePunchThroughParticles(const G4FastTrack& fastTrack, CLHEP::HepRandomEngine* rndmEngine, double punchThroughProbability, double punchThroughClassifierRand) = 0;
    virtual void createAllSecondaryTracks(G4ParticleTable &ptable, G4FastStep& fastStep, const G4Track& g4PrimaryTrack, std::vector<std::map<std::string, double>> &secKinematicsMapVect, G4TrackVector& secTrackCont, std::vector<double> caloMSVars) = 0;

    /** Other methods */
    virtual std::vector<double> getCaloMSVars() = 0;
};

#endif // IPunchThroughG4Tool_H