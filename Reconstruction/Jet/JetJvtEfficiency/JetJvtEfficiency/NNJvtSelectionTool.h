/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETJVTEFFICIENCY_NNJVTSELECTIONTOOL_H
#define JETJVTEFFICIENCY_NNJVTSELECTIONTOOL_H

#include "AsgTools/PropertyWrapper.h"
#include "JetJvtEfficiency/JvtSelectionToolBase.h"
#include "JetMomentTools/NNJvtBinning.h"


namespace CP {
    class NNJvtSelectionTool : public JvtSelectionToolBase {
        ASG_TOOL_CLASS(NNJvtSelectionTool, IAsgSelectionTool)
    public:
        using JvtSelectionToolBase::JvtSelectionToolBase;
        virtual ~NNJvtSelectionTool() override = default;

        virtual StatusCode initialize() override;

    private:
        // Used to correctly initialize the ReadDecorHandle
        Gaudi::Property<std::string> m_jetContainer{
                this, "JetContainer", "",
                "The name of the jet container, used to correctly initialize the read handles"};
        Gaudi::Property<std::string> m_wp{
                this, "WorkingPoint", "FixedEffPt", "The working point to use"};
        Gaudi::Property<std::string> m_configDir{
                this, "ConfigDir", "JetPileupTag/NNJvt/2022-03-22",
                "The directory containing the NN config files"};
        Gaudi::Property<std::string> m_configFile{
                this, "ConfigFile", "", "The NNJvt config file. Overrides the WorkingPoint property"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_jvtMoment{
                this, "JvtMomentName", "NNJvt", "The name of the Jvt moment to use"};

        JetPileupTag::NNJvtCutMap m_cutMap;
        virtual bool select(const xAOD::IParticle *jet) const override;
        
        // TODO: TEMPORARY
        // Backup accessors to allow using these tools in the JetJvtEfficiency object which does not
        // know its parent jet container name
        SG::ConstAccessor<float> m_jvtAcc { m_jvtMoment.key() };
    };
} // namespace CP

#endif //> !JETJVTEFFICIENCY_NNJVTSELECTIONTOOL_H
