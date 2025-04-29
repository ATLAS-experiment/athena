/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BOOSTEDJETSTAGGERS_SMOOTHEDTOPTAGGER_H_
#define BOOSTEDJETSTAGGERS_SMOOTHEDTOPTAGGER_H_

#include "BoostedJetTaggers/JSSTaggerBase.h"

class SmoothedTopTagger :
  public JSSTaggerBase {
    ASG_TOOL_CLASS0(SmoothedTopTagger)

    public:

      // Default - so root can load based on a name
      SmoothedTopTagger(const std::string& name);

      // Decorate jet with tagging information
      virtual StatusCode tag(const xAOD::Jet& jet) const override;

      // Run once at the start of the job to setup everything
      virtual StatusCode initialize() override;

    private:

      // number of variables used by cut-based tagger
      int m_numTaggerVars{};

      // cut functions from smooth fits
      std::vector<std::string> m_varCutExprs;
     
      // names of the variables used by this tool (ex: Mass, Tau32)
      std::vector<std::string> m_varCutNames;

      // functions implementing the cut values vs pt
      std::vector<std::unique_ptr<TF1>> m_varCutFuncs;

      // declaration of decorators for cut values
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_mcut{this, "mcutName", "Cut_m", "SG key for Cut_m"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_dec_sphericitycut{this, "sphericitycutName", "Cut_Sphericity", "SG key for Cut_Sphericity"};

      // declaration of decorators for cut information
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decPassSphericityKey{this, "PassSphericityName", "PassSphericity", "SG key for PassSphericity"};

      // vector of recognised cut names
      // add to this vector as necessary when additional taggers are implemented
      // variables are duplicated to allow cases where variable names are not provided with first
      // letter capitalised, but all decorations to jets will assume the capitalised version is being used
      // e.g. PassMass, PassSphericity, etc.
      std::vector<std::string> m_recognisedCuts = {"Mass", "mass", "Sphericity", "sphericity"};

  };

#endif
