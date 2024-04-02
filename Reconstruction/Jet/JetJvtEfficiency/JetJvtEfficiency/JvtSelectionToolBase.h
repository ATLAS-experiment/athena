/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef JETJVTEFFICIENCY_JVTSELECTIONTOOLBASE_H
#define JETJVTEFFICIENCY_JVTSELECTIONTOOLBASE_H

#include "AsgDataHandles/ReadDecorHandleKey.h"
#include "AsgTools/AsgTool.h"
#include "AsgTools/PropertyWrapper.h"
#include "PATCore/IAsgSelectionTool.h"
#include "xAODJet/JetContainer.h"

namespace CP {
    class JvtSelectionToolBase : public asg::AsgTool, virtual public IAsgSelectionTool {
    public:
        using asg::AsgTool::AsgTool;
        virtual ~JvtSelectionToolBase() = default;

        virtual StatusCode initialize() override;

        virtual const asg::AcceptInfo &getAcceptInfo() const override;

        virtual asg::AcceptData accept(const xAOD::IParticle *jet) const override;

    protected:
        Gaudi::Property<float> m_minPtForJvt{
                this, "MinPtForJvt", 20e3, "Accept all jets with pT below this"};
        Gaudi::Property<float> m_maxPtForJvt{
                this, "MaxPtForJvt", 60e3, "Accept all jets with pT above this"};
        Gaudi::Property<float> m_minEta{
                this, "MinEtaForJvt", -1, "Accept all jets with |eta| below this"};
        Gaudi::Property<float> m_maxEta{
                this, "MaxEtaForJvt", 2.5, "Accept all jets with |eta| above this"};
        // NB: Use a string not a read handle key as this is not written with a write handle key
        Gaudi::Property<std::string> m_jetEtaName{
                this, "JetEtaName", "DetectorEta", "The name of the jet eta to use."};

        // The template AcceptInfo object
        asg::AcceptInfo m_info;
        // The index to set in the info. I suspect that this is always 0 but better to be safe
        int m_cutPos = 0;
        // The accessor for the jet eta
        SG::ConstAccessor<float> m_etaAcc { m_jetEtaName };
        // Check the range
        virtual bool isInRange(const xAOD::IParticle *jet) const;
        // Check the score
        virtual bool select(const xAOD::IParticle *jet) const = 0;
    };
} // namespace CP

#endif //> !JETJVTEFFICIENCY_JVTSELECTIONTOOL_H
