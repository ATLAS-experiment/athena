/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONTRACKFINDINGTEST_SEGMENTREFITTEST_H
#define MUONTRACKFINDINGTEST_SEGMENTREFITTEST_H

#include "AthenaBaseComps/AthHistogramAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"

#include "xAODMuon/MuonSegmentContainer.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonTesterTree/MuonTesterTree.h"

namespace MuonValR4{
    /** @brief Simple tester class to refit the Mdt segments with the Acts global chi2 fitter
     *         In the ideal case, the Acts fit should return the same parameters as the Muon segment
     *         fitter with the same measurements to be included on the fit */
    class SegmentRefitTest: public AthHistogramAlgorithm {
        public:
            using AthHistogramAlgorithm::AthHistogramAlgorithm;
            virtual StatusCode initialize() override final;
            virtual StatusCode execute() override final;
            virtual StatusCode finalize() override final;

        private:
            /** @brief Declare the data dependency on the standard Mdt+Rpc+Tgc segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_preFitKey{this, "PreFitContainer", "MuonSegmentsFromR4"};
            /** @brief Declare the data dependency on the post fit segment container */
            SG::ReadHandleKey<xAOD::MuonSegmentContainer> m_postFitKey{this, "PostFitContainer", "ActsRefitSegments"};
            /** @brief  Construct a link from the refitted segment to the input segment. */
            SG::ReadDecorHandleKey<xAOD::MuonSegmentContainer> m_linkKey{this, "Link", m_postFitKey, "prefitSegmentLink"};

            /** @brief IdHelperSvc to decode the Identifiers */
            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "IdHelperSvc",  "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            /** @brief The output muon tester tree */
            MuonVal::MuonTesterTree m_tree{"SegmentRefitTree", "SegmentRefitTest"};
            /// chamber index field 
            MuonVal::ScalarBranch<int>& m_chamberIndex{m_tree.newScalar<int>("chamberIndex")};
            /// +1 for A-, -1 of C-side 
            MuonVal::ScalarBranch<short>& m_stationSide{m_tree.newScalar<short>("stationSide")};
            /// phi index of the station
            MuonVal::ScalarBranch<int>& m_stationPhi{m_tree.newScalar<int>("stationPhi")};
            /** @brief Local X before the refit */
            MuonVal::ScalarBranch<float>& m_preFitLocX{m_tree.newScalar<float>("preFitLocX")};
            /** @brief Local Y before the refit */
            MuonVal::ScalarBranch<float>& m_preFitLocY{m_tree.newScalar<float>("preFitLocY")};
            /** @brief Local Theta before the refit */ 
            MuonVal::ScalarBranch<float>& m_preFitTheta{m_tree.newScalar<float>("preFitTheta")};
            /** @brief Local Phi before the refit */
            MuonVal::ScalarBranch<float>& m_preFitPhi{m_tree.newScalar<float>("preFitPhi")};
            /** @brief Uncertainty on the fitted local X (prefit) */
            MuonVal::ScalarBranch<float>& m_uncertLocX{m_tree.newScalar<float>("uncertLocX")};
            /** @brief Uncertainty on the fitted local Y (prefit) */
            MuonVal::ScalarBranch<float>& m_uncertLocY{m_tree.newScalar<float>("uncertLocY")};
            /** @brief Uncertainty on the fitted local Theta (prefit) */
            MuonVal::ScalarBranch<float>& m_uncertTheta{m_tree.newScalar<float>("uncertTheta")};
            /** @brief Uncertainty on the fitted local Phi (prefit) */
            MuonVal::ScalarBranch<float>& m_uncertPhi{m_tree.newScalar<float>("uncertPhi")};

            /** @brief Chi2 of the segment before the refit */
            MuonVal::ScalarBranch<float>& m_preFitChi2{m_tree.newScalar<float>("preFitChi2")};
            /** @brief nDoF of the segment before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_preFitNdoF{m_tree.newScalar<unsigned short>("preFitNdoF")};
            /** @brief Number of precision hits before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_preFitNPrecHits{m_tree.newScalar<unsigned short>("preFitNPrecHits")};
            /** @brief Number of eta trigger hits before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_preFitNTrigEtaHits{m_tree.newScalar<unsigned short>("preFitNTrigEtaHits")};
            /** @brief Number of phi trigger hits before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_preFitNTrigPhiHits{m_tree.newScalar<unsigned short>("preFitNTrigPhiHits")};
            /** @brief Flag whether the fit has succeeded or not */
            MuonVal::ScalarBranch<unsigned char>& m_goodFit{m_tree.newScalar<unsigned char>("goodFit", false)};
            /** @brief Local X after the refit */
            MuonVal::ScalarBranch<float>& m_seedFitLocX{m_tree.newScalar<float>("seedFitLocX", 0.f)};
            /** @brief Local Y after the refit */
            MuonVal::ScalarBranch<float>& m_seedFitLocY{m_tree.newScalar<float>("seedFitLocY", 0.f)};
            /** @brief Local Theta after the refit */ 
            MuonVal::ScalarBranch<float>& m_seedFitTheta{m_tree.newScalar<float>("seedFitTheta", 0.f)};
            /** @brief Local Phi after the refit */
            MuonVal::ScalarBranch<float>& m_seedFitPhi{m_tree.newScalar<float>("seedFitPhi", 0.f)};
            /** @brief Local X after the refit */
            MuonVal::ScalarBranch<float>& m_postFitLocX{m_tree.newScalar<float>("postFitLocX", 0.f)};
            /** @brief Local Y after the refit */
            MuonVal::ScalarBranch<float>& m_postFitLocY{m_tree.newScalar<float>("postFitLocY", 0.f)};
            /** @brief Local Theta after the refit */ 
            MuonVal::ScalarBranch<float>& m_postFitTheta{m_tree.newScalar<float>("postFitTheta", 0.f)};
            /** @brief Local Phi after the refit */
            MuonVal::ScalarBranch<float>& m_postFitPhi{m_tree.newScalar<float>("postFitPhi", 0.f)};
            /** @brief Chi2 of the segment after the refit */
            MuonVal::ScalarBranch<float>& m_postFitChi2{m_tree.newScalar<float>("postFitChi2", 0.f)};
            /** @brief nDoF of the segment after the refit */
            MuonVal::ScalarBranch<unsigned short>& m_postFitNdoF{m_tree.newScalar<unsigned short>("postFitNdoF", 0)};
            /** @brief Number of precision hits after the refit */
            MuonVal::ScalarBranch<unsigned short>& m_postFitNPrecHits{m_tree.newScalar<unsigned short>("postFitNPrecHits", 0)};
            /** @brief Number of eta trigger hits before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_postFitNTrigEtaHits{m_tree.newScalar<unsigned short>("postFitNTrigEtaHits", 0)};
            /** @brief Number of phi trigger hits before the refit */
            MuonVal::ScalarBranch<unsigned short>& m_postFitNTrigPhiHits{m_tree.newScalar<unsigned short>("postFitNTrigPhiHits", 0)};
    };
}

#endif