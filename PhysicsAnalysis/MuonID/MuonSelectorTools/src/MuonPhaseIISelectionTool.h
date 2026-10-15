/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSELECTORTOOLS_MUONPHASEIISELECTIONTOOL_H
#define MUONSELECTORTOOLS_MUONPHASEIISELECTIONTOOL_H

#include "MuonAnalysisInterfaces/IMuonSelectionTool.h"
#include "AthenaBaseComps/AthAlgTool.h"

namespace CP{
    /** @brief Dummy selection tool to make the PhaseII workflows work with the arrival
     *         of MudiCo in the Acts version */
    class MuonPhaseIISelectionTool : public extends<AthAlgTool,  IMuonSelectionTool> {
        public:
            using base_class::base_class;
            /** @copydoc IMuonSelectionTool::print */
            void print() const override final;
            /** @copydoc IMuonSelectionTool::getAcceptInfo */
            virtual const asg::AcceptInfo& getAcceptInfo() const override final;
            /** @copydoc IMuonSelectionTool::accept */
            virtual asg::AcceptData accept(const xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::setPassesIDCuts */
            virtual void setPassesIDCuts(xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::setQuality */
            virtual void setQuality(xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::passedIDCuts */
            virtual bool passedIDCuts(const xAOD::Muon&) const override final;
             /** @copydoc IMuonSelectionTool::passedMuonCuts */
            virtual bool passedMuonCuts(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::passedIDCuts */
            virtual bool passedIDCuts(const xAOD::TrackParticle&) const override final;
            /** @copydoc IMuonSelectionTool::passedHighPtCuts */
            virtual bool passedHighPtCuts(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::passedLowPtEfficiencyCuts */
            virtual bool passedLowPtEfficiencyCuts(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::passedLowPtEfficiencyCuts */
            virtual bool passedLowPtEfficiencyCuts(const xAOD::Muon&, xAOD::Muon::Quality thisMu_quality) const override final;
            /** @copydoc IMuonSelectionTool::passedErrorCutCB */
            virtual bool passedErrorCutCB(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::isBadMuon */
            virtual bool isBadMuon(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::getQuality */            
            virtual xAOD::Muon::Quality getQuality(const xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::passedCaloTagQuality */
            virtual bool passedCaloTagQuality(const xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::passedCaloScore */
            virtual bool passedCaloScore(const xAOD::Muon& mu) const override final;
            /** @copydoc IMuonSelectionTool::getResolutionCategory */
            virtual int getResolutionCategory(const xAOD::Muon&) const override final;
            /** @copydoc IMuonSelectionTool::getTightNNScore */
            virtual float getTightNNScore(const xAOD::Muon& muon) const override final;
        private:
            /// Store selection information.
            asg::AcceptInfo m_acceptInfo{"MuonSelection"};

    };
}
#endif