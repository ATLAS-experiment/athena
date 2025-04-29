/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef IPunchThroughG4Classifier_H
#define IPunchThroughG4Classifier_H

// C++
#include <vector>

// Gaudi
#include "GaudiKernel/IAlgTool.h"

// forward declarations
class G4FastTrack;

/**
  @class IPunchThroughG4Classifier

   Interface for a tool which takes simulstate and particle
   and predicts whether it should result in a muon segment.

  @author thomas.michael.carter@cern.ch, firdaus.soberi@cern.ch
*/

class IPunchThroughG4Classifier : virtual public IAlgTool
{
 public:
    /** AlgTool interface method, handles constructor/destructor */
    DeclareInterfaceID(IPunchThroughG4Classifier, 1, 0);

   /** calculates probability of punch through from G4FastTrack and the energies (simE as simulated total energy and simEfrac as simulated layer energies) */
   virtual double computePunchThroughProbability(const G4FastTrack& fastTrack, const double simE, const std::vector<double> & simEfrac) const = 0;
};

#endif // IPunchThroughG4Classifier_H
