#include "JetJvtEfficiency/FJvtSelectionTool.h"
#include "AsgDataHandles/ReadDecorHandle.h"

namespace {
    const static std::map<std::string, float> workingPoints{{"Loose", 0.5}, {"Tight", 0.4}};
}

namespace CP {

    FJvtSelectionTool::FJvtSelectionTool(const std::string &name) : JvtSelectionToolBase(name) {
        m_minEta = 2.5;
        m_maxEta = 4.5;
    }

    StatusCode FJvtSelectionTool::initialize() {
        ATH_CHECK(JvtSelectionToolBase::initialize());

        
        m_jvtAcc = SG::ConstAccessor<float>(m_jvtMoment.key());
        m_timingAcc = SG::ConstAccessor<float>(m_timingMoment.key());
        if (m_jetContainer.empty()) {
            ATH_MSG_WARNING("No JetContainer set. This behaviour is deprecated");
            ATH_CHECK(m_jvtMoment.initialize(false));
            ATH_CHECK(m_timingMoment.initialize(false));
        }
        else {
            m_jvtMoment = m_jetContainer + "." + m_jvtMoment.key();
            ATH_CHECK(m_jvtMoment.initialize());
            m_timingMoment = m_jetContainer + "." + m_timingMoment.key();
            ATH_CHECK(m_timingMoment.initialize());
        }

        if (m_wp != "Custom") {
            auto itr = workingPoints.find(m_wp);
            if (itr == workingPoints.end()) {
                ATH_MSG_ERROR("Invalid fJvt working point name");
                return StatusCode::FAILURE;
            }
            m_jvtCut = itr->second;
        }

        return StatusCode::SUCCESS;
    }

    bool FJvtSelectionTool::select(const xAOD::IParticle *jet) const {
        return m_jvtAcc(*jet) <= m_jvtCut && m_timingAcc(*jet) <= m_timingCut;
    }

} // namespace CP
