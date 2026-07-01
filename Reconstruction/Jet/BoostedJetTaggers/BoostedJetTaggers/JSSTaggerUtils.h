/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BOOSTEDJETSTAGGERS_JSSTAGGERUTILS_H_
#define BOOSTEDJETSTAGGERS_JSSTAGGERUTILS_H_

#include "BoostedJetTaggers/JSSTaggerBase.h"
#include "BoostedJetTaggers/IJSSTaggerUtils.h"
#include "BoostedJetTaggers/JSSMLTool.h"

#include "xAODPFlow/FlowElement.h"

class JSSTaggerUtils :
  public IJSSTaggerUtils, public JSSTaggerBase {
  ASG_TOOL_CLASS(JSSTaggerUtils, IJSSTaggerUtils)
  
    public:

      /// Constructor
      JSSTaggerUtils( const std::string& name );

      /// Run once at the start of the job to setup everything
      virtual StatusCode initialize() override;

      /// IJetSelectorTool interface
      virtual StatusCode tag( const xAOD::Jet& jet ) const override;

      // Const tagger
      TH2D MakeJetImage(TString TagImage, const xAOD::Jet* jet, std::vector<xAOD::JetConstituent> constituents) const override;
      StatusCode GetImageScore(const xAOD::JetContainer& jets) const override;
      StatusCode GetConstScore(const xAOD::JetContainer& jets) const override;
      StatusCode GetQGConstScore(const xAOD::JetContainer& jets) const override;
      StatusCode GetTopConstScore(const xAOD::JetContainer& jets) const override;
      StatusCode GetWConstScore(const xAOD::JetContainer& jets) const override;
      StatusCode GetPolarisationScore(const xAOD::JetContainer& jets) const override;

      // HighLevel tagger
      StatusCode GetHLScore(const xAOD::JetContainer& jets) const override;
      std::map<std::string, double> GetJSSVars(const xAOD::Jet& jet) const override;
      StatusCode ReadScaler() override;

      std::map<std::string, std::vector<double>> m_scaler;

    private:

      int m_nbins_eta{}, m_nbins_phi{};
      double m_min_eta{}, m_max_eta{}, m_min_phi{}, m_max_phi{};
      int m_ncolors{};
      bool m_dorscaling{};
      double m_rscaling_p0{}, m_rscaling_p1{};


      // Constituents tagger 
      bool m_UseConstTagger{};
      ToolHandle<AthONNX::IJSSMLTool> m_MLBosonTagger {"", this};
      
      // HighLevel tagger
      bool m_UseHLTagger{};
      ToolHandle<AthONNX::IJSSMLTool> m_MLBosonTagger_HL {"", this};

      /// WriteDecorHandle keys
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decNConstituentsKey{this, "nConstituentsName", "nConstituents", "SG key for constituents multiplicity"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decNTopoTowersKey{this, "nTopoTowersName", "nTopoTowers", "SG key for towers multiplicity"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreKey{this, "ConstScoreNameDec", "ConstScore", "SG key for ConstScore"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decHLScoreKey{this, "HLScoreName", "HLScore", "SG key for HLScore"};

      std::unique_ptr<SG::ConstAccessor<float>> m_constScoreAcc = nullptr;

      // write decor handles
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreQGKey{this, "ConstScoreQGNameDec", "ConstScore_qg", "SG key for qg node"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreWLKey{this, "ConstScoreWLNameDec", "ConstScore_wl", "SG key for WL node"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreWTKey{this, "ConstScoreWTNameDec", "ConstScore_wt", "SG key for WT node"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreZLKey{this, "ConstScoreZLNameDec", "ConstScore_zl", "SG key for ZL node"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreZTKey{this, "ConstScoreZTNameDec", "ConstScore_zt", "SG key for ZT node"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreCosTKey{this, "ConstScoreCosTNameDec", "ConstScore_cosT", "SG key for cosT regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScorePT1Key{this, "ConstScorePT1NameDec", "ConstScore_pT1", "SG key for pT1 regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreEta1Key{this, "ConstScoreEta1NameDec", "ConstScore_eta1", "SG key for eta1 regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScorePhi1Key{this, "ConstScorePhi1NameDec", "ConstScore_phi1", "SG key for phi1 regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScorePT2Key{this, "ConstScorePT2NameDec", "ConstScore_pT2", "SG key for pT2 regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScoreEta2Key{this, "ConstScoreEta2NameDec", "ConstScore_eta2", "SG key for eta2 regress"};
      SG::WriteDecorHandleKey<xAOD::JetContainer> m_decConstScorePhi2Key{this, "ConstScorePhi2NameDec", "ConstScore_phi2", "SG key for phi2 regress"};

  };

#endif
