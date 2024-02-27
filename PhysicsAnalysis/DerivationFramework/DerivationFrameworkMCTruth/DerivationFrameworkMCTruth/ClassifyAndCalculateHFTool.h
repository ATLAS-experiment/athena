/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

//////////////////////////////////////////////////////////////////////////
//                                                                      //
// ClassifyAndCalculateHFAugmentation.h                                 //
// Header file for class ClassifyAndCalculateHFTool                     //
// Author: Adrian Berrocal Guardia <adrian.berrocal.guardia@cern.ch>    //
//                                                                      //
// Algorithm to calculate a variable called HFClassification which      //
// classifies ttbar+jets events according to the number of additional   //
// HF jets.                                                             //
//                                                                      //
////////////////////////////////////////////////////////////////////////// 

#ifndef DERIVATIONFRAMEWORK_CLASSIFYANDCALCULATEHFTOOL_H
#define DERIVATIONFRAMEWORK_CLASSIFYANDCALCULATEHFTOOL_H

// Basic C++ headers:

#include <string>

// Athena tools headers.

#include "AthenaBaseComps/AthAlgTool.h"
#include "DerivationFrameworkMCTruth/HadronOriginClassifier.h"

#include "xAODJet/Jet.h"
#include "xAODJet/JetContainer.h"
#include "xAODBase/IParticle.h"
#include "xAODTruth/TruthParticle.h"
#include "xAODTruth/TruthParticleContainer.h"

namespace DerivationFramework {

  static const InterfaceID IID_ClassifyAndCalculateHFTool("ClassifyAndCalculateHFTool", 1, 0);
  
  // Declare the class that computes the HF classifier.

  class ClassifyAndCalculateHFTool: public AthAlgTool {

    /*
    -------------------------------------------------------------------------------------------------------------------------------------
    --------------------------------------------------- Public Variables and Functions --------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------
    */
  
    public: 

      // Declare the constructor and destructor functions.

      ClassifyAndCalculateHFTool(const std::string& t, const std::string& n, const IInterface* p);
      virtual ~ClassifyAndCalculateHFTool();

      // Declare the initialize and finalize function for the class which are called before and after the loop over events respectively.

      virtual StatusCode initialize() override;
      virtual StatusCode finalize() override;
      
      static const InterfaceID& interfaceID() { return IID_ClassifyAndCalculateHFTool; }

      // Declare a set of functions to change cuts on the particles:
      //  -jetPtCut:                Save a given float value as a cut on the pt of the jets.
      //  -jetEtaCut:               Save a given float value as a cut on the eta of the jets.
      //  -leadingHadronPtCut:      Save a given float value as a cut on the pt of the leading hadron.
      //  -leadingHadronPtRatioCut: Save a given float value as a cut on the ratio between the pt of the leading hadron and the pt of its associated jet.

      inline void jetPtCut(float a){m_jetPtCut=a;}
      inline void jetEtaCut(float a){m_jetEtaCut=a;}
      inline void leadingHadronPtCut(float a){m_leadingHadronPtCut=a;}
      inline void leadingHadronPtRatioCut(float a){m_leadingHadronPtRatioCut=a;}

      // Declare the following set of functions to compute the classifier:
      //  -flagJets:                Computes the necessary variables for the classifier using information from jets and add the information in three vectors.
      //  -computeHFClassification: Compute the classifier.
      //  -getSimpleClassification: Compute a simpler classifier. 
      //  -isBHadron:               Determine if an hadron is a B-type.
      //  -isCHadron:               Determine if an hadron is a C-type.

      void flagJets(const xAOD::JetContainer* jets,
                    const std::map<const xAOD::Jet*, std::vector<xAOD::TruthParticleContainer::const_iterator>>& particleMatch,
                    const std::map<const xAOD::TruthParticle*, DerivationFramework::HadronOriginClassifier::HF_id>&  hadronMap,
                    const std::string& hfDecorationName) const;
      int computeHFClassification(const xAOD::JetContainer* jets, const std::string& hfDecorationName) const;
      int getSimpleClassification(int hfclassif) const;
      bool isBHadron(int pdgId) const;
      bool isCHadron(int pdgId) const;

    /*
    -------------------------------------------------------------------------------------------------------------------------------------
    -------------------------------------------------- Private Variables and Functions --------------------------------------------------
    -------------------------------------------------------------------------------------------------------------------------------------
    */
  
    private:

      // Declare a set of float variables to save the cuts on the particles:
      //  -m_jetPtCut:                Cut on the pt of the jets.
      //  -m_jetEtaCut:               Cut on the eta of the jets.
      //  -m_leadingHadronPtCut:      Cut on the pt of the leading hadron.
      //  -m_leadingHadronPtRatioCut: Cut on the ratio between the pt of the leading hadron and the pt of its associated jet.

      Gaudi::Property<float> m_jetPtCut{this, "jetPtCut", 15000., "Cut on the jets pt that are considered to compute the HF classification."};
      Gaudi::Property<float> m_jetEtaCut{this, "jetEtaCut", 2.5, "Cut on the jets eta that are considered to compute the HF classification."};
      Gaudi::Property<float> m_leadingHadronPtCut{this, "leadingHadronPtCut", 5000., "Cut on the hadrons that are considered to compute the HF classification."};
      Gaudi::Property<float> m_leadingHadronPtRatioCut{this, "leadingHadronPtRatioCut", -1., "Cut on the ratio between the pt of the leading hadron matched to a jet and the jet pt."};
      
  }; 
}

#endif // DERIVATIONFRAMEWORK_CLASSIFYANDCALCULATEHFTOOL_H
