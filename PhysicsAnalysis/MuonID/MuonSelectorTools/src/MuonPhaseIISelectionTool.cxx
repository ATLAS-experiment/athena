/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonPhaseIISelectionTool.h"

#include <format>

#define NOT_IMPLEMENTED() \
    ATH_MSG_WARNING("{:}() - {:} Method {:} is not implemented", \
                    __FILE__, __LINE__, __func__);


namespace CP {
/** @copydoc IMuonSelectionTool::getAcceptInfo */
    void MuonPhaseIISelectionTool::print() const {
        NOT_IMPLEMENTED()
    }
    const asg::AcceptInfo& MuonPhaseIISelectionTool::getAcceptInfo() const { 
        NOT_IMPLEMENTED() 
        return m_acceptInfo;
    }     
    asg::AcceptData MuonPhaseIISelectionTool::accept(const xAOD::Muon&) const { 
        asg::AcceptData data{&m_acceptInfo};
        NOT_IMPLEMENTED();
        return data; 
    }
    void MuonPhaseIISelectionTool::setPassesIDCuts(xAOD::Muon&) const { NOT_IMPLEMENTED() }
    void MuonPhaseIISelectionTool::setQuality(xAOD::Muon& ) const { NOT_IMPLEMENTED() }
    bool MuonPhaseIISelectionTool::passedIDCuts(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED() 
        return false; 
    }
    bool MuonPhaseIISelectionTool::passedMuonCuts(const xAOD::Muon&) const { 
         NOT_IMPLEMENTED() 
        return false; 
    }
    bool MuonPhaseIISelectionTool::passedIDCuts(const xAOD::TrackParticle&) const { 
        NOT_IMPLEMENTED()
        return false;
    }
    bool MuonPhaseIISelectionTool::passedHighPtCuts(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED() 
        return false;
    }
    bool MuonPhaseIISelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED()
        return false; 
    }
    bool MuonPhaseIISelectionTool::passedLowPtEfficiencyCuts(const xAOD::Muon&, xAOD::Muon::Quality) const { 
        NOT_IMPLEMENTED() 
        return false;
    }
         
    bool MuonPhaseIISelectionTool::passedErrorCutCB(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED() 
        return false;
    }
    bool MuonPhaseIISelectionTool::isBadMuon(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED() 
        return false;
    }
    xAOD::Muon::Quality MuonPhaseIISelectionTool::getQuality(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED()
        return xAOD::Muon::Quality::VeryLoose; 
    }
    bool MuonPhaseIISelectionTool::passedCaloTagQuality(const xAOD::Muon& ) const { 
        NOT_IMPLEMENTED()
        return false; 
    }
    bool MuonPhaseIISelectionTool::passedCaloScore(const xAOD::Muon& ) const { 
        NOT_IMPLEMENTED() 
        return false;
    }
    int MuonPhaseIISelectionTool::getResolutionCategory(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED()
        return 0;
    }
    float MuonPhaseIISelectionTool::getTightNNScore(const xAOD::Muon&) const { 
        NOT_IMPLEMENTED()
        return 0.f;
    }
}