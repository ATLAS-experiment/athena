/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/////////////////////////////////////////////////////////////
// ScaleFactors.h
// Class to implement the use of the tagger scale factors;
// the class retrives the tagger scale factors from the 
// recommendation files and attach them to the jets.
// author: Antonio Giannini <antonio.giannini@cern.ch>
/////////////////////////////////////////////////////////////

#ifndef BOOSTEDJETSTAGGERS_SCALEFACTORS_H_
#define BOOSTEDJETSTAGGERS_SCALEFACTORS_H_

#include "BoostedJetTaggers/JSSTaggerBase.h"
#include "JetAnalysisInterfaces/IVarTool.h"

namespace BJT{

  class ScaleFactors :
    public asg::AsgTool, virtual public IJetDecorator {
      ASG_TOOL_CLASS1(ScaleFactors, IJetDecorator)

      public:

        /// Constructor
        ScaleFactors(const std::string& name);

        /// Run once at the start of the job to setup everything
        virtual StatusCode initialize() override;

        /// Decorate jet container with tagging info
        virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

      private:

        /// input parameters
        Gaudi::Property<std::string> m_truthLabelName{this, "truthLabelName", "", "truth label"};
        Gaudi::Property<float> m_jetPtMin{this, "jetPtMin", 200., "minimum jet pT cut"};
        Gaudi::Property<float> m_jetPtMax{this, "jetPtMax", 2500., "maximum jet pT cut"};
        Gaudi::Property<float> m_jetEtaMax{this, "jetEtaMax", 2., "maximum jet eta cut"};

        /// helper histogram tool
        /// ToDo: once W and top available should make this more generic and configurable
        ToolHandle<JetHelper::IVarTool> m_histTool2D_quark_eff {
          this, "HistoReader2D_quark_eff", "HistoInput2D quark eff", "Histogram reader as a JetHelper::IVarTool"
        };
        ToolHandle<JetHelper::IVarTool> m_histTool2D_quark_ineff {
          this, "HistoReader2D_quark_ineff", "HistoInput2D quark ineff", "Histogram reader as a JetHelper::IVarTool"
        };

        ToolHandle<JetHelper::IVarTool> m_histTool2D_gluon_eff {
          this, "HistoReader2D_gluon_eff", "HistoInput2D gluon eff", "Histogram reader as a JetHelper::IVarTool"
        };
        ToolHandle<JetHelper::IVarTool> m_histTool2D_gluon_ineff {
          this, "HistoReader2D_gluon_ineff", "HistoInput2D gluon ineff", "Histogram reader as a JetHelper::IVarTool"
        };

        /// ReadDecorHandle keys
        SG::ReadHandleKey<xAOD::JetContainer> m_jetsKey {this, "JetContainer", "", "jet container to use"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_accTaggedKey{this, "TaggedName", m_jetsKey, "", "SG key for Tagger WP decision"};

        /// WriteDecorHandle keys
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decEfficiencyKey{this, "Efficiency", m_jetsKey, "", "SG key for Scale Factor Efficiency correction"};
        SG::WriteDecorHandleKey<xAOD::JetContainer> m_decInefficiencyKey{this, "Inefficiency", m_jetsKey, "", "SG key for Scale Factor Inefficiency correction"};

    };

}
#endif
