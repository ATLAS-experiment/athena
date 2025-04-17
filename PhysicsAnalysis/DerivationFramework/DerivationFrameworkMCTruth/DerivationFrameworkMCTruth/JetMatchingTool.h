/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// JetMatchingTool.h                                                    //
// Header file for class JetMatchingTool                                //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
// Algorithm to match each truth particle to the closest jet            //
//                                                                      //
////////////////////////////////////////////////////////////////////////// 

#ifndef JetMatchingTool_HH
#define JetMatchingTool_HH

// Athena tools headers.

#include "AthenaBaseComps/AthAlgTool.h"

#include "xAODJet/Jet.h"
#include "xAODBase/IParticle.h"
#include "xAODTruth/TruthParticle.h"

namespace DerivationFramework{
  
  // Declare the class that matches hadrons with jets.

  class JetMatchingTool: public AthAlgTool {

  /*
  -------------------------------------------------------------------------------------------------------------------------------------
  --------------------------------------------------- Public Variables and Functions --------------------------------------------------
  -------------------------------------------------------------------------------------------------------------------------------------
  */

  public:
    
    // Declare the constructor and the destructor functions.

    JetMatchingTool(const std::string& t, const std::string& n, const IInterface* p);
    virtual ~JetMatchingTool();

    // Declare the initialize and finalize function for the class which are called before and after the loop over events respectively.

    virtual StatusCode initialize() override;
    virtual StatusCode finalize() override;

    // Declare a set of functions to change cuts on the particles:
    //  -jetPtCut:  Save a given float value as a cut on the pt of the jets.
    //  -jetEtaCut: Save a given float value as a cut on the eta of the jets.
    //  -drCut:     Save a given float value as a cut on the dr between the hadron and the jet to match them.

    inline void jetPtCut(float a){m_jetPtCut=a;}
    inline void jetEtaCut(float a){m_jetEtaCut=a;}
    inline void drCut(float a){m_drCut=a;}

    // Declare the function that matches the hadrons with the jets.

    std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>> matchHadronsToJets(const xAOD::TruthParticleContainer* hadrons,const xAOD::JetContainer* jets) const; 

  /*
  -------------------------------------------------------------------------------------------------------------------------------------
  -------------------------------------------------- Private Variables and Functions --------------------------------------------------
  -------------------------------------------------------------------------------------------------------------------------------------
  */

  // Declare the private variables and functions which the user cannot change outside the class.

  private:

    // Declare a set of float variables to save the cuts on the particles:
    //  -m_jetPtCut:  Cut on the pt of the jets.
    //  -m_jetEtaCut: Cut on the eta of the jets.
    //  -m_drCut:     Cut on the deltaR to match the a jet with an hadron.

    Gaudi::Property<float> m_jetPtCut{this, "jetPtCut", 15000., "Cut on the jets pt that are considered for the hadron-jet matching."};
    Gaudi::Property<float> m_jetEtaCut{this, "jetEtaCut", 2.5, "Cut on the jets eta that are considered for the hadron-jet matching."};
    Gaudi::Property<float> m_drCut{this, "drCut", 0.4, "Cut on the delta R between an hadron and the closest jet to consider them matched."};
    
  };
}

#endif // JetMatchingTool_HH
