/*
Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "PRDxAODConvertorAlg.h"
#include "xAODMuonPrepData/MdtDriftCircleAuxContainer.h"
#include "xAODMuonPrepData/RpcStripAuxContainer.h"
#include "xAODMuonPrepData/TgcStripAuxContainer.h"
#include "xAODMuonPrepData/MMClusterAuxContainer.h"
#include "xAODMuonPrepData/sTgcStripAuxContainer.h"


StatusCode Muon::PRDxAODConvertorAlg::initialize() {
  ATH_CHECK(m_mdtPrepRawDataKey.initialize(!m_mdtPrepRawDataKey.empty()));
  ATH_CHECK(m_tgcPrepRawDataKey.initialize(!m_tgcPrepRawDataKey.empty()));
  ATH_CHECK(m_rpcPrepRawDataKey.initialize(!m_rpcPrepRawDataKey.empty()));
  ATH_CHECK(m_mmPrepRawDataKey.initialize(!m_mmPrepRawDataKey.empty()));
  ATH_CHECK(m_stgcPrepRawDataKey.initialize(!m_stgcPrepRawDataKey.empty()));

  ATH_CHECK(m_mdtxAODKey.initialize(!m_mdtPrepRawDataKey.empty()));
  ATH_CHECK(m_rpcxAODKey.initialize(!m_rpcPrepRawDataKey.empty()));
  ATH_CHECK(m_tgcxAODKey.initialize(!m_tgcPrepRawDataKey.empty()));
  ATH_CHECK(m_mmxAODKey.initialize(!m_mmPrepRawDataKey.empty()));
  ATH_CHECK(m_stgcxAODKey.initialize(!m_stgcPrepRawDataKey.empty())); 

  
  ATH_CHECK(m_idHelperSvc.retrieve());

  return StatusCode::SUCCESS;
}

StatusCode Muon::PRDxAODConvertorAlg::execute(
    const EventContext& ctx) const {
 
  ATH_MSG_VERBOSE("About to create trackContainer");

  ATH_CHECK( (getAndFillContainer < Muon::MdtPrepDataContainer, xAOD::MdtDriftCircleContainer, xAOD::MdtDriftCircleAuxContainer > (m_mdtPrepRawDataKey, m_mdtxAODKey, ctx) ) );
  ATH_CHECK( (getAndFillContainer < Muon::RpcPrepDataContainer, xAOD::RpcStripContainer, xAOD::RpcStripAuxContainer > (m_rpcPrepRawDataKey, m_rpcxAODKey, ctx) ) );
  ATH_CHECK( (getAndFillContainer < Muon::TgcPrepDataContainer, xAOD::TgcStripContainer, xAOD::TgcStripAuxContainer > (m_tgcPrepRawDataKey, m_tgcxAODKey, ctx) ) );
  ATH_CHECK( (getAndFillContainer < Muon::MMPrepDataContainer, xAOD::MMClusterContainer, xAOD::MMClusterAuxContainer > (m_mmPrepRawDataKey, m_mmxAODKey, ctx) ) );
  ATH_CHECK( (getAndFillContainer < Muon::sTgcPrepDataContainer, xAOD::sTgcStripContainer, xAOD::sTgcStripAuxContainer > (m_stgcPrepRawDataKey, m_stgcxAODKey, ctx) ) );

  return StatusCode::SUCCESS;
}

// I *think* it's possible to not need to pass OUTTYPEAUX, but construct it from OUTTYPE.
template <class INTYPE, class OUTTYPE, class OUTTYPEAUX>
  StatusCode Muon::PRDxAODConvertorAlg::getAndFillContainer(const SG::ReadHandleKey<INTYPE> &inKey,
                                 const SG::WriteHandleKey<OUTTYPE> &outKey, const EventContext& ctx) const {
  if(inKey.empty()) {
    ATH_MSG_VERBOSE("No key configured to convert "<<typeid(INTYPE).name()<<" into xAOD objects. Skipping.");
    return StatusCode::SUCCESS;
  }

  SG::ReadHandle<INTYPE> handle(inKey, ctx);

  ATH_MSG_VERBOSE("Trying to load " << handle.key());
  ATH_CHECK(handle.isValid());
  ATH_MSG_VERBOSE("Which has " << handle->numberOfCollections()
                               << " collections: ");

  SG::WriteHandle<OUTTYPE> outputContainer(outKey, ctx);
  ATH_CHECK( outputContainer.record (std::make_unique<OUTTYPE>(),
						 std::make_unique<OUTTYPEAUX>()) );
  ATH_MSG_DEBUG( "Recorded xAOD container with key: " << outputContainer.key()  );

  for (const auto &coll : *handle) {
    for (const auto &prd : *coll) {
      auto * xprd = new typename OUTTYPE::base_value_type;
      outputContainer->push_back(xprd);
      fillxPRD(*prd, *xprd);
    }
  }

  return StatusCode::SUCCESS;
}

// Keeping this in since it seems to compile with gcc13 and not 11 and I want to understand why.
// template<class PRD, class xPRD> void Muon::PRDxAODConvertorAlg::fillxPRD(
//     const PRD& prd, xPRD& xprd) const {
//       static_assert(false, "FillxPRD requires explicit specialization.");
// }

template<> void Muon::PRDxAODConvertorAlg::fillxPRD(
    const Muon::MdtPrepData& prd, xAOD::MdtDriftCircle& xprd) const {
  const MdtIdHelper& id_helper{m_idHelperSvc->mdtIdHelper()};

  const Identifier prdId{prd.identify()};
  xprd.setIdentifier(prd.identify().get_identifier32().get_compact());
  
  xAOD::MeasVector<1> localPosition(prd.localPosition().x());
  xAOD::MeasMatrix<1> localCovariance{};
  localCovariance(0, 0) = prd.localCovariance()(0, 0);
  xprd.setMeasurement(m_idHelperSvc->detElementHash(prdId), localPosition, localCovariance);
  
  xprd.setTdc(prd.tdc());
  xprd.setAdc(prd.adc());
  xprd.setTube(id_helper.tube(prd.identify()));
  xprd.setLayer(id_helper.tubeLayer(prd.identify()));
  xprd.setStatus(prd.status());
  // TODO tubePosInStation - but this needs ReadoutElement?
}

template<> void Muon::PRDxAODConvertorAlg::fillxPRD(
    const Muon::RpcPrepData& prd, xAOD::RpcStrip& xprd) const {
  
  const RpcIdHelper& idHelper{m_idHelperSvc->rpcIdHelper()};
  const Identifier prdId{prd.identify()};
  xprd.setIdentifier(prdId.get_identifier32().get_compact());
  
  xprd.setStripNumber(idHelper.strip(prdId));
  xprd.setGasGap(idHelper.gasGap(prdId));
  xprd.setMeasuresPhi(idHelper.measuresPhi(prdId));
  xprd.setDoubletPhi(idHelper.doubletPhi(prdId));

  xAOD::MeasVector<1> localPosition(prd.localPosition().x());
  xAOD::MeasMatrix<1> localCovariance{};
  localCovariance(0, 0) = prd.localCovariance()(0, 0);
  xprd.setMeasurement(m_idHelperSvc->detElementHash(prdId), localPosition, localCovariance);
  
  xprd.setTime(prd.time());
  xprd.setTriggerInfo(prd.triggerInfo());
  xprd.setAmbiguityFlag(prd.ambiguityFlag());
  xprd.setTimeOverThreshold(prd.timeOverThreshold());

}

template<> void Muon::PRDxAODConvertorAlg::fillxPRD(
    const Muon::TgcPrepData& prd, xAOD::TgcStrip& xprd) const {
  
  const TgcIdHelper& idHelper{m_idHelperSvc->tgcIdHelper()};
  const Identifier prdId{prd.identify()};
  xprd.setIdentifier(prdId.get_identifier32().get_compact());
  xprd.setMeasuresPhi(idHelper.measuresPhi(prdId));
  xprd.setGasGap(idHelper.gasGap(prdId));
  xprd.setChannelNumber(idHelper.channel(prdId));
  xAOD::MeasVector<1> localPosition(prd.localPosition().x());
  xAOD::MeasMatrix<1> localCovariance{};
  localCovariance(0, 0) = prd.localCovariance()(0, 0);
  xprd.setMeasurement(m_idHelperSvc->detElementHash(prdId), localPosition, localCovariance);
  
  xprd.setBcBitMap(prd.getBcBitMap());

}

template<> void Muon::PRDxAODConvertorAlg::fillxPRD(
    const Muon::MMPrepData& prd, xAOD::MMCluster& xprd) const {
  
  const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
  const Identifier prdId{prd.identify()};
  xprd.setIdentifier(prdId.get_identifier32().get_compact());
  xprd.setGasGap(idHelper.gasGap(prdId));
  xprd.setChannelNumber(idHelper.channel(prdId));
  xAOD::MeasVector<1> localPosition(prd.localPosition().x());
  xAOD::MeasMatrix<1> localCovariance{};
  localCovariance(0, 0) = prd.localCovariance()(0, 0);
  xprd.setMeasurement(m_idHelperSvc->detElementHash(prdId), localPosition, localCovariance);
  
  xprd.setTime(prd.time());
  xprd.setCharge(prd.charge());
  xprd.setDriftDist(prd.driftDist());
  xprd.setAngle(prd.angle());
  xprd.setChiSqProb(prd.chisqProb());
  xprd.setStripDriftErrors(prd.stripDriftErrors());
  xprd.setStripDriftDist(prd.stripDriftDist());
  xprd.setStripDriftDist(prd.stripDriftDist());
  xprd.setStripCharges(prd.stripCharges());
  xprd.setStripTimes(prd.stripTimes());
  xprd.setStripNumbers(prd.stripNumbers());
}

template<> void Muon::PRDxAODConvertorAlg::fillxPRD(
    const Muon::sTgcPrepData& prd, xAOD::sTgcStrip& xprd) const {
  xprd.setIdentifier(prd.identify().get_identifier32().get_compact());
  
  Eigen::Matrix<float,1,1> localPosition(prd.localPosition().x());
  Eigen::Matrix<float,1,1> localCovariance;
  localCovariance.setZero();
  localCovariance(0, 0) = prd.localCovariance()(0, 0);
  xprd.setMeasurement(prd.collectionHash(), localPosition, localCovariance);
  
  xprd.setBcBitMap(prd.getBcBitMap());
  xprd.setTime(prd.time());
  xprd.setCharge(prd.charge());
}