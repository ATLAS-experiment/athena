/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


#include "TgcDigitToPrepDataCnvTool.h"

#include "xAODMuonPrepData/TgcStripAuxContainer.h"

#include "Acts/Utilities/Helpers.hpp"

namespace MuonR4 {

    StatusCode TgcDigitToPrepDataCnvTool::initialize() {
        ATH_CHECK(m_idHelperSvc.retrieve());
        ATH_CHECK(m_readKey.initialize());
        ATH_CHECK(m_writeKey.initialize());
        ATH_CHECK(detStore()->retrieve(m_detMgr));
        return StatusCode::SUCCESS;
    }
    StatusCode TgcDigitToPrepDataCnvTool::decode(const EventContext& ctx, 
                                                 const std::vector<IdentifierHash>& idVect) const {
        const TgcDigitContainer* digitContainer{nullptr};
        ATH_CHECK(SG::get(digitContainer, m_readKey, ctx));

        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::TgcStripContainer>(),
                                     std::make_unique<xAOD::TgcStripAuxContainer>()));

        const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
        for (const TgcDigitCollection* coll : *digitContainer) {
            if (!idVect.empty() && !Acts::rangeContainsValue(idVect, coll->identifierHash())) {
                ATH_MSG_VERBOSE("Do not encode measurements from "<<
                    m_idHelperSvc->toStringChamber(coll->identify()));
                continue;
            }
            const MuonGMR4::TgcReadoutElement* reEle = m_detMgr->getTgcReadoutElement(coll->identify());
            const std::size_t nPrdBefore = writeHandle->size();
            for (const TgcDigit* digit : *coll) {
                // Check whether the same channel has been fired but under different BC tag
                xAOD::TgcStripContainer::iterator sameHitOtherBC = 
                std::find_if(writeHandle->begin() + nPrdBefore, writeHandle->end(), [digit](const xAOD::TgcStrip* prd) {
                        return prd->identify() == digit->identify();
                });
                const std::uint16_t bcTag = (1<< digit->bcTag());
                if (sameHitOtherBC != writeHandle->end()) {
                    xAOD::TgcStrip* updateMe{*sameHitOtherBC};
                    updateMe->setBcBitMap(updateMe->bcBitMap() | bcTag);
                    continue;
                }
                xAOD::TgcStrip* newStrip = writeHandle->push_back(std::make_unique<xAOD::TgcStrip>());               

                newStrip->setBcBitMap(bcTag);
                newStrip->setChannelNumber(idHelper.channel(digit->identify()));
                newStrip->setGasGap(idHelper.gasGap(digit->identify()));
                newStrip->setMeasuresPhi(idHelper.measuresPhi(digit->identify()));
                newStrip->setReadoutElement(reEle);


                const Amg::Vector3D measPos = reEle->sensorLayout(newStrip->layerHash())
                                                   ->localStripPosition(newStrip->channelNumber(),
                                                                        newStrip->measuresPhi());
                xAOD::MeasVector<1> locPos{xAOD::MeasVector<1>::Zero()};
                xAOD::MeasMatrix<1> locCov{xAOD::MeasVector<1>::Identity()};

                if (newStrip->measuresPhi()) {
                    locPos[0] = measPos[1];
                    const auto& radDesign = reEle->stripLayout(newStrip->measurementHash());
                    locCov(0,0) = Acts::square(radDesign.stripPitch(newStrip->channelNumber())) / 12.;
                } else {
                    locPos[0] = measPos[0];
                    const auto& wireDesign = reEle->wireGangLayout(newStrip->measurementHash());
                    locCov(0,0) = Acts::square(wireDesign.stripPitch() *
                                  wireDesign.numWiresInGroup(newStrip->channelNumber())) / 12.;
                }

                ATH_MSG_VERBOSE("Convert new "<<m_idHelperSvc->toString(newStrip->identify())
                                <<" @ "<<Amg::toString(measPos)<<" 1D: "<<locPos[0]
                                <<" with covariance "<<locCov(0,0));
                newStrip->setMeasurement(reEle->identHash(), locPos, locCov);
            }
        }
        ATH_MSG_DEBUG("Recorded in total "<<writeHandle->size()<<" measurements.");
        return StatusCode::SUCCESS;
    }
    StatusCode TgcDigitToPrepDataCnvTool::decode(const EventContext& /*ctx*/,
                                                 const std::vector<uint32_t>& /*robIds*/) const {
        ATH_MSG_ERROR("Decoding with ROBs not implemented.");
        return StatusCode::FAILURE;
    }
    StatusCode TgcDigitToPrepDataCnvTool::provideEmptyContainer(const EventContext& ctx) const {
        SG::WriteHandle writeHandle{m_writeKey, ctx};
        ATH_CHECK(writeHandle.record(std::make_unique<xAOD::TgcStripContainer>(),
                                     std::make_unique<xAOD::TgcStripAuxContainer>()));
        return StatusCode::SUCCESS;
    }

}