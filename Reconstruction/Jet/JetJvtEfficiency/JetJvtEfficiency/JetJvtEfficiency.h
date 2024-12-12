/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETJVTEFFICIENCYSCALEFACTORS_H_
#define JETJVTEFFICIENCYSCALEFACTORS_H_

#include "JetAnalysisInterfaces/IJetJvtEfficiency.h"
#include "JetAnalysisInterfaces/IJvtEfficiencyTool.h"
#include "PATCore/IAsgSelectionTool.h"
#include "JetInterface/IJetDecorator.h"
#include "PATInterfaces/SystematicsTool.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/AnaToolHandle.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "JetMomentTools/JetVertexNNTagger.h"

#include "xAODEventInfo/EventInfo.h"

#include <TH2.h>
#include <string>
#include <memory>

namespace CP {

enum SystApplied {
  NONE,
  NNJVT_EFFICIENCY_DOWN,
  NNJVT_EFFICIENCY_UP,
  FJVT_EFFICIENCY_DOWN,
  FJVT_EFFICIENCY_UP,
};

class JetJvtEfficiency: public asg::AsgTool,
                        public CP::SystematicsTool,
                        virtual public CP::IJetJvtEfficiency,
                        virtual public IJetDecorator {
    ASG_TOOL_CLASS2( JetJvtEfficiency, CP::IJetJvtEfficiency, IJetDecorator)

public:
    JetJvtEfficiency( const std::string& name);

    virtual StatusCode initialize() override;

    StatusCode histInitialize();

    virtual CorrectionCode getEfficiencyScaleFactor(const xAOD::Jet& jet,float& sf) override;
    virtual CorrectionCode getInefficiencyScaleFactor(const xAOD::Jet& jet,float& sf) override;
    virtual CorrectionCode applyEfficiencyScaleFactor(const xAOD::Jet& jet) override;
    virtual CorrectionCode applyInefficiencyScaleFactor(const xAOD::Jet& jet) override;
    virtual CorrectionCode applyAllEfficiencyScaleFactor(const xAOD::IParticleContainer *jets,float& sf) override;
    virtual bool passesJvtCut(const xAOD::Jet& jet) const override;
    virtual bool isInRange(const xAOD::Jet& jet) const override;

    // Decorate jets with flag for passJVT decision
    virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

    virtual StatusCode recalculateScores(const xAOD::JetContainer& jets) const override;

    bool isAffectedBySystematic(const CP::SystematicVariation& var) const override {return CP::SystematicsTool::isAffectedBySystematic(var);}
    CP::SystematicSet affectingSystematics() const override {return CP::SystematicsTool::affectingSystematics();}
    CP::SystematicSet recommendedSystematics() const override {return CP::SystematicsTool::recommendedSystematics();}
    StatusCode applySystematicVariation(const CP::SystematicSet& set) override {return CP::SystematicsTool::applySystematicVariation(set);}
    StatusCode sysApplySystematicVariation(const CP::SystematicSet&) override;

    float getJvtThresh() const override {return m_jvtCut;}
    float getUserPtMax() const override {return m_maxPtForJvt;}
    StatusCode tagTruth(const xAOD::IParticleContainer *jets,const xAOD::IParticleContainer *truthJets) override;

private:

    JetJvtEfficiency();

    SystApplied m_appliedSystEnum;

    ToolHandle<JetPileupTag::JetVertexNNTagger>  m_NNJvtTool_handle;
    ToolHandle<IAsgSelectionTool> m_jvtSelTool;
    ToolHandle<IJvtEfficiencyTool> m_jvtEffTool;

    int m_tagger{};
    std::string m_wp;
    std::string m_file;
    JvtTagger m_taggingAlg{};
    std::unique_ptr<TH2> m_h_JvtHist;
    std::unique_ptr<TH2> m_h_EffHist;
    std::string m_passJvtDecName;
    std::string m_sf_decoration_name;
    std::string m_isHS_decoration_name;
    std::string m_truthJetContName;
    std::string m_jetEtaName;
    float m_maxPtForJvt{};
    bool m_doTruthRequirement{};
    std::string m_ORdec;
    bool m_useMuBinsSF{};
    bool m_useDummySFs{};
    std::string m_jetContainerName;
    std::string m_NNJvtParamFile;
    std::string m_NNJvtCutFile;

    // kept for backwards compatibility with legacy Jvt
    std::string m_jetJvtMomentName;
    float m_jvtCut{};
    float m_jvtCutBorder{};

    // configurable accessors/decorators
    std::unique_ptr<SG::AuxElement::ConstAccessor< float > > m_jetEtaAcc;
    std::unique_ptr<SG::AuxElement::ConstAccessor< char > > m_passORAcc;
    std::unique_ptr<SG::AuxElement::Decorator< float > > m_sfDec;
    std::unique_ptr<SG::AuxElement::Decorator< char > > m_isHSDec;
    std::unique_ptr<SG::AuxElement::ConstAccessor< char > > m_isHSAcc;

    SG::WriteDecorHandleKey<xAOD::JetContainer> m_passJvtKey{this, "PassJVTKey", "passJvt",
      "SG key for passJvt decoration (including jet container name)"};
    bool m_suppressOutputDependence{};
};

} /* namespace CP */

#endif /* JETJVTEFFICIENCYSCALEFACTORS_H_ */
