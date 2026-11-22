/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BOOSTEDJETTAGGERS_JSSTAGGERSELECTIONTOOL_H
#define BOOSTEDJETTAGGERS_JSSTAGGERSELECTIONTOOL_H

#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "AsgTools/ToolHandle.h"
#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgDataHandles/WriteDecorHandleKey.h"
#include "AthContainers/ConstAccessor.h"

#include "PATCore/IAsgSelectionTool.h"
#include "JetInterface/IJetDecorator.h"
#include "JetAnalysisInterfaces/IVarTool.h"
#include "xAODJet/JetContainer.h"

namespace CP {

  /// Selection tool for JSS taggers: a jet is tagged if it is inside the
  /// kinematic range and its score is above a cut. The cut value is provided
  /// by any JetHelper::IVarTool (e.g. HistoInput2D or FuncInput1D).
  class JSSTaggerSelectionTool : public asg::AsgTool,
                                 virtual public IAsgSelectionTool,
                                 virtual public IJetDecorator {
    ASG_TOOL_CLASS2(JSSTaggerSelectionTool, IAsgSelectionTool, IJetDecorator)

  public:
    using asg::AsgTool::AsgTool;
    virtual ~JSSTaggerSelectionTool() override = default;

    virtual StatusCode initialize() override;

    /// IAsgSelectionTool
    virtual const asg::AcceptInfo& getAcceptInfo() const override;
    virtual asg::AcceptData accept(const xAOD::IParticle* jet) const override;

    /// IJetDecorator
    virtual StatusCode decorate(const xAOD::JetContainer& jets) const override;

  private:
    /// Kinematic range check
    bool isInRange(const xAOD::IParticle& jet) const;
    /// Score cut, given the jet score
    bool passScore(const xAOD::IParticle& jet, float score) const;

    /// Jet container and decorations
    SG::ReadHandleKey<xAOD::JetContainer> m_jetContainer {this, "JetContainer", "", "Jet container to use"};
    SG::ReadDecorHandleKey<xAOD::JetContainer> m_scoreNameKey {this, "ScoreName", m_jetContainer, "", "Tagger score decoration"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_passTaggerKey {this, "TaggedName", m_jetContainer, "", "Pass tagger (kinematics + score)"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_passValidKinRangeKey {this, "ValidKinRangeName", m_jetContainer, "", "Pass kinematic range"};
    SG::WriteDecorHandleKey<xAOD::JetContainer> m_passScoreKey {this, "PassScoreName", m_jetContainer, "", "Pass score cut"};

    /// Kinematic selection (pt in MeV)
    Gaudi::Property<std::string> m_jetEtaName {this, "JetEtaName", "eta", "Name of the jet eta to use"};
    Gaudi::Property<float> m_jetPtMin {this, "JetPtMin", 0., "Minimum jet pt"};
    Gaudi::Property<float> m_jetPtMax {this, "JetPtMax", 0., "Maximum jet pt"};
    Gaudi::Property<float> m_jetEtaMax {this, "JetEtaMax", 0., "Maximum jet |eta|"};

    /// Score cut value as a function of the jet (histogram, functional form, ...)
    ToolHandle<JetHelper::IVarTool> m_cutTool {this, "CutReader", "", "Score cut as a JetHelper::IVarTool"};

    asg::AcceptInfo m_info;
    SG::ConstAccessor<float> m_etaAcc {"eta"};
  };

} // namespace CP

#endif
