/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
// qgTagger.h
// Class to implement the constituents based q/g tagger;
// the class retrives the tagger score attached to the jets
// in the derivations, and it applies a 2D (pT, eta) WP.
// author: Antonio Giannini <antonio.giannini@cern.ch>
/////////////////////////////////////////////////////////////

#ifndef BOOSTEDJETSTAGGERS_QGTAGGER_H_
#define BOOSTEDJETSTAGGERS_QGTAGGER_H_

#include "BoostedJetTaggers/JSSTaggerBase.h"
#include "JetAnalysisInterfaces/IVarTool.h"

namespace BJT{

  class qgTagger :
    public JSSTaggerBase {
      ASG_TOOL_CLASS0(qgTagger)
    
      public:

        /// Constructor
        qgTagger( const std::string& name );

        /// Run once at the start of the job to setup everything
        virtual StatusCode initialize() override;

        /// Decorate jet container with tagging info
        virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

        /// Dummy single jet tag function
        virtual StatusCode tag(const xAOD::Jet& jet) const override;

      private:

        /// input file
        std::string m_InputFileName;
        std::string m_HistoName;

        /// helper histogram tool
        ToolHandle<JetHelper::IVarTool> m_histTool2D {
          this, "HistoReader2D", "HistoInput2D", "Histogram reader as a JetHelper::IVarTool"
        };

        /// WriteDecorHandle keys
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decValidKinRangeKey{this, "ValidKinRangeName", "ValidKinRange", "SG key for ValidKinRange"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decPassScoreKey{this, "PassScoreName", "PassScore", "SG key for PassScore"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decCutScoreKey{this, "CutScoreName", "Cut_Score", "SG key for Cut_Score"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decAcceptKey{this, "acceptName", "accept", "SG key for accept"};

    };

}
#endif
