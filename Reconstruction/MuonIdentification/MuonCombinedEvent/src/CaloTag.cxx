/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCombinedEvent/CaloTag.h"

namespace MuonCombined {

    /** constructors */
    CaloTag::CaloTag(void) :
        TagBase(xAOD::Muon::Author::CaloTag, xAOD::Muon::MuonType::CaloTagged),
        Trk::EnergyLoss(0, 0, 0, 0) {}

    CaloTag::CaloTag(const Trk::EnergyLoss& eloss) :
        TagBase(xAOD::Muon::Author::CaloTag, xAOD::Muon::MuonType::CaloTagged),
        Trk::EnergyLoss(eloss) {}

    CaloTag::CaloTag(xAOD::Muon::Author author, float deltaE, float sigmaDeltaE, float sigmaMinusDeltaE, float sigmaPlusDeltaE,
                     unsigned short energyLossType, float likelihood, float muonScore, unsigned short tag) :
        TagBase(author, xAOD::Muon::MuonType::CaloTagged),
        Trk::EnergyLoss(deltaE, sigmaDeltaE, sigmaMinusDeltaE, sigmaPlusDeltaE),
        m_energyLossType (static_cast<CaloTag::EnergyLossType>(energyLossType)),
        m_caloLRLikelihood(likelihood),
        m_caloMuonScore(muonScore),
        m_caloMuonIdTag(tag) {
    }

    CaloTag::CaloTag(xAOD::Muon::Author author, float deltaE, float sigmaDeltaE, float sigmaMinusDeltaE, float sigmaPlusDeltaE,
                     unsigned short energyLossType, float likelihood, float muonScore, unsigned short tag,
                     const std::vector<DepositInCalo>& deposits)

        :
        TagBase(author, xAOD::Muon::MuonType::CaloTagged),
        Trk::EnergyLoss(deltaE, sigmaDeltaE, sigmaMinusDeltaE, sigmaPlusDeltaE),
        m_energyLossType (static_cast<CaloTag::EnergyLossType>(energyLossType)),
        m_caloLRLikelihood(likelihood),
        m_caloMuonScore(muonScore),
        m_caloMuonIdTag(tag),
        m_deposits (deposits) {}

    CaloTag::~CaloTag() = default;

}  // namespace MuonCombined
