/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETJVTEFFICIENCY_FJVTSELECTIONTOOL_H
#define JETJVTEFFICIENCY_FJVTSELECTIONTOOL_H

#include "AsgTools/PropertyWrapper.h"
#include "JetJvtEfficiency/JvtSelectionToolBase.h"

namespace CP {
    class FJvtSelectionTool : public JvtSelectionToolBase {
        ASG_TOOL_CLASS(FJvtSelectionTool, IAsgSelectionTool)
    public:
        FJvtSelectionTool(const std::string &name);
        virtual ~FJvtSelectionTool() override = default;

        virtual StatusCode initialize() override;

    private:
        // Used to correctly initialize the ReadDecorHandle
        Gaudi::Property<std::string> m_jetContainer{
                this, "JetContainer", "",
                "The name of the jet container, used to correctly initialize the read handles"};
        Gaudi::Property<std::string> m_wp{
                this, "WorkingPoint", "Loose",
                "The working point to use. Set to 'Custom' to manually set the values"};
        Gaudi::Property<float> m_jvtCut{this, "JvtCut", 999, "The JVT selection to make"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_jvtMoment{
                this, "JvtMomentName", "FJvt", "The name of the Jvt moment to use"};
        SG::ReadDecorHandleKey<xAOD::JetContainer> m_timingMoment{
                this, "TimingMomentName", "Timing", "The name of the timing moment to use"};
        Gaudi::Property<float> m_timingCut{
                this, "TimingCut", 10, "Only accept jets with time less than this"};

        virtual bool select(const xAOD::IParticle *jet) const override;

        // TODO: TEMPORARY
        // Backup accessors to allow using these tools in the JetJvtEfficiency object which does not
        // know its parent jet container name
        SG::ConstAccessor<float> m_jvtAcc { m_jvtMoment.key() };
        SG::ConstAccessor<float> m_timingAcc { m_timingMoment.key() };
    };
} // namespace CP

#endif //> !JETJVTEFFICIENCY_JVTSELECTIONTOOL_H
