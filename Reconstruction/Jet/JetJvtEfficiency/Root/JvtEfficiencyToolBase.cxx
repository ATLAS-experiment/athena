/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "JetJvtEfficiency/JvtEfficiencyToolBase.h"
#include "AsgDataHandles/ReadDecorHandle.h"
#include "PathResolver/PathResolver.h"

#include <TFile.h>

namespace {
    bool getBin(const TAxis &axis, float value, int &bin) {
        bin = axis.FindBin(value);
        return (bin != 0 && bin != axis.GetNbins());
    }
    bool getBinContentAndError(const TH2 &h, float x, float y, float &content, float &error) {
        int xBin{}, yBin{};
        if (!getBin(*h.GetXaxis(), x, xBin) || !getBin(*h.GetYaxis(), y, yBin)) {

            content = -1;
            error = -1;
            return false;
        }
        content = h.GetBinContent(xBin, yBin);
        error = h.GetBinError(xBin, yBin);
        return true;
    }
} // namespace

namespace CP {
    StatusCode JvtEfficiencyToolBase::initialize() {
        m_etaAcc = SG::ConstAccessor<float>(m_jetEtaName);
        if (!m_doTruthRequirement)
            ATH_MSG_WARNING("No truth requirement will be performed, which is not recommended.");
        m_accIsHS.emplace(m_truthHSLabel.key());
        if (m_jetContainer.empty()) {
            ATH_MSG_WARNING("No JetContainer set. This behaviour is deprecated");
            ATH_CHECK(m_truthHSLabel.initialize(false));
        }
        else {
            m_truthHSLabel = m_jetContainer + "." + m_truthHSLabel.key();
            ATH_CHECK(m_truthHSLabel.initialize(m_doTruthRequirement));
        }
        return StatusCode::SUCCESS;
    }

    CorrectionCode
    JvtEfficiencyToolBase::getEfficiencyScaleFactor(const xAOD::Jet &jet, float &sf) const {
        if (!isInRange(jet)) {
            sf = -1;
            return CorrectionCode::OutOfValidityRange;
        }
        if (m_doTruthRequirement) {
            if (!m_accIsHS->isAvailable(jet)) {
                ATH_MSG_ERROR("Truth tagging required but not available");
                return CorrectionCode::Error;
            }
            if (!(*m_accIsHS)(jet)) {
                sf = 1;
                return CorrectionCode::Ok;
            }
        }
        return getEffImpl(jet.pt(), m_etaAcc(jet), sf);
    }

    CorrectionCode
    JvtEfficiencyToolBase::getInefficiencyScaleFactor(const xAOD::Jet &jet, float &sf) const {
        if (!isInRange(jet)) {
            sf = -1;
            return CorrectionCode::OutOfValidityRange;
        }
        if (m_doTruthRequirement) {
            if (!m_accIsHS->isAvailable(jet)) {
                ATH_MSG_ERROR("Truth tagging required but not available");
                return CorrectionCode::Error;
            }
            if (!(*m_accIsHS)(jet)) {
                sf = 1;
                return CorrectionCode::Ok;
            }
        }
        return getIneffImpl(jet.pt(), m_etaAcc(jet), sf);
    }

    StatusCode JvtEfficiencyToolBase::initHists(const std::string &file, const std::string &wp) {
        if (file.empty()) {
            m_useDummySFs = true;
            return StatusCode::SUCCESS;
        }
        std::string resolved = PathResolverFindCalibFile(file);
        if (resolved.empty()) {
            ATH_MSG_ERROR("Could not locate file " << file);
            return StatusCode::FAILURE;
        }
        std::unique_ptr<TFile> fIn(TFile::Open(resolved.c_str(), "READ"));
        if (!fIn) {
            ATH_MSG_ERROR("Failed to open file " <<  resolved);
            return StatusCode::FAILURE;
        }

        std::string jvtName = "Jvt" + wp;
        m_jvtHist.reset(fIn->Get<TH2>(jvtName.c_str()));
        if (!m_jvtHist) {
            ATH_MSG_ERROR(
                    "Could not open SF histogram "
                    << jvtName << ". Please check the supported working points.");
            return StatusCode::FAILURE;
        }
        m_jvtHist->SetDirectory(nullptr);
        std::string effName = "Eff" + wp;
        m_effHist.reset(fIn->Get<TH2>(effName.c_str()));
        if (!m_effHist) {
            ATH_MSG_ERROR(
                    "Could not open efficiency histogram "
                    << jvtName << ". Please check the supported working points.");
            return StatusCode::FAILURE;
        }
        m_effHist->SetDirectory(nullptr);

        return StatusCode::SUCCESS;
    }

    CorrectionCode JvtEfficiencyToolBase::getEffImpl(float x, float y, float &sf) const {
        float baseFactor = 1;
        float errorTerm = m_dummySFError;
        if (!m_useDummySFs) {
            if (!getBinContentAndError(*m_jvtHist, x, y, baseFactor, errorTerm)) {
                sf = -1;
                return CorrectionCode::OutOfValidityRange;
            }
        }
        sf = baseFactor + m_appliedSysSigma * errorTerm;
        return CorrectionCode::Ok;
    }

    CorrectionCode JvtEfficiencyToolBase::getIneffImpl(float x, float y, float &sf) const {
        if (m_useDummySFs) {
            sf = 1 + m_dummySFError * m_appliedSysSigma;
            return CorrectionCode::Ok;
        }
        float baseFactor, errorTerm, effFactor, errorEffTerm;
        if (!getBinContentAndError(*m_jvtHist, x, y, baseFactor, errorTerm) ||
            !getBinContentAndError(*m_effHist, x, y, effFactor, errorEffTerm)) {
            sf = -1;
            return CorrectionCode::OutOfValidityRange;
        }
        baseFactor += errorTerm * m_appliedSysSigma;
        effFactor += errorEffTerm * m_appliedSysSigma;

        sf = (1 - baseFactor * effFactor) / (1 - effFactor);
        return CorrectionCode::Ok;
    }

    bool JvtEfficiencyToolBase::isInRange(const xAOD::Jet &jet) const {
        if (jet.pt() < m_minPtForJvt || jet.pt() > m_maxPtForJvt)
            return false;
        float eta = m_etaAcc(jet);
        return std::abs(eta) >= m_minEta && std::abs(eta) <= m_maxEta;
    }
} // namespace CP
