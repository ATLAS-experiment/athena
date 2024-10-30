/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "TrigCountSpacePoints.h"

#include "InDetPrepRawData/PixelCluster.h"
namespace {
  bool isInNoiseArea(float r, float zz) {
    if(r<40){
      if(zz>-3.5 && zz<-1.5) return true;
      if(zz>38. && zz<40.) return true;
      if(zz>79.5 && zz<81.5) return true;
      if(zz>121. && zz<123.) return true;
      if(zz>162. && zz<165.) return true;
      if(zz>204. && zz<206.) return true;
      if(zz>245.5 && zz<247.5) return true;
      if(zz>-45. && zz<-43.5) return true;
      if(zz>-86.5 && zz<-84.5) return true;
      if(zz>-128. && zz<-126.) return true;
      if(zz>-170. && zz<-167.) return true;
      if(zz>-211. && zz<-209.) return true;
      if(zz>-252.5 && zz<-250.5) return true;
    }
    return false;
  }
}

TrigCountSpacePoints::TrigCountSpacePoints(const std::string &name, ISvcLocator *pSvcLocator) : AthReentrantAlgorithm(name, pSvcLocator) {}

StatusCode TrigCountSpacePoints::initialize() {
  ATH_CHECK(m_pixelSpKey.initialize());
  ATH_CHECK(m_pixelHelperKey.initialize());
  ATH_CHECK(m_sctSpKey.initialize());
  ATH_CHECK(m_sctHelperKey.initialize());
  ATH_CHECK(m_spacePointsKey.initialize());
  ATH_CHECK(m_spacePointsAuxKey.initialize());
  if (!m_monTool.empty()) {
    ATH_CHECK(m_monTool.retrieve());
  }
  return StatusCode::SUCCESS;
}

StatusCode TrigCountSpacePoints::execute(const EventContext &context) const {

  // For Pixel info decoding
  SG::ReadHandle<PixelID> pixelHelper(m_pixelHelperKey, context);

  // counters
  int pixCLBeforeCuts{};
  int pixCL{};
  int pixCLnoToT{};
  int pixCL_1{};
  int pixCL_2{};
  int pixCLmin3{};
  int pixCLBarrel{};
  int pixCLEndcapA{};
  int pixCLEndcapC{};
  int pixModulesOverThreshold{};
  std::complex<double> pixQ2TotSum;

  SG::ReadHandle<SpacePointContainer> pixelSP(m_pixelSpKey, context);
  ATH_MSG_DEBUG("Successfully retrieved pixel SP container of size " << pixelSP->size());

  for (const auto pixSPointColl : *pixelSP) {
    if (pixSPointColl == nullptr) {
      continue;
    }

    const Identifier pixid = (pixSPointColl)->identify();
    if (m_doOnlyBLayer == true && pixelHelper->layer_disk(pixid) != 0) {
      continue;
    }
    const int bec = pixelHelper->barrel_ec(pixid);

    int nPixSP{}, nPixCL_1{}, nPixCL_2{}, nPixCLmin3{}, nPixCLnoToT{};
    std::complex<double> pixQ2sum;

    for (const auto pSP : *pixSPointColl) {
      pixCLBeforeCuts++;
      if (m_removeBLayerModuleEdgeNoise) {
        if ( isInNoiseArea(pSP->r(), pSP->globalPosition()[Amg::z]) ) {
          continue;
        }
      }

      const InDet::PixelCluster *pixClust = static_cast<const InDet::PixelCluster *>(pSP->clusterList().first);

      const int pixClSize = (pixClust->rdoList()).size();
      const int pixclToT = pixClust->totalToT();

      ++nPixCLnoToT;
      pixQ2sum += std::polar(1.0, 2.0*pSP->phi()); // this is complex value exp(2i*phi)
      if (pixclToT > m_pixelClusToTCut) {
        ++nPixSP;
        if (pixClSize == 1) {
          ++nPixCL_1;
        }
        if (pixClSize == 2) {
          ++nPixCL_2;
        }
        if (pixClSize >= 3) {
          ++nPixCLmin3;
        }
      }
    }

    // total
    if (nPixSP > m_pixModuleThreshold) {
      ATH_MSG_DEBUG(" This pixel module : " << pixid << " produced " << nPixSP << " pix spacepoints. Ignoring these spacepoints as the maximum allowed spacepoints per module is " << m_pixModuleThreshold);
      pixModulesOverThreshold++;
    } else {
      pixCL += nPixSP;
      pixQ2TotSum += pixQ2sum;
      pixCLnoToT += nPixCLnoToT;
      pixCL_1 += nPixCL_1;
      pixCL_2 += nPixCL_2;
      pixCLmin3 += nPixCLmin3;
      if (bec == 0) {
        pixCLBarrel += nPixSP;
        ATH_MSG_VERBOSE(" Formed  " << nPixSP << " PIX spacepoints in PIX Barrel after ToT cut.");
      } else if (bec == 2) {
        pixCLEndcapA += nPixSP;
        ATH_MSG_VERBOSE(" Formed  " << nPixSP << " PIX spacepoints in PIX ECA after ToT cut.");
      } else if (bec == -2) {
        pixCLEndcapC += nPixSP;
        ATH_MSG_VERBOSE(" Formed  " << nPixSP << " PIX spacepoints in PIX ECC after ToT cut.");
      }
    }
  }
  const float pixQ2mod = pixCLnoToT !=0 ? std::abs(pixQ2TotSum)/pixCLnoToT : 0.0;
  ATH_MSG_DEBUG("REGTEST : Formed  " << pixCLBeforeCuts << " pixel spacepoints in total before cuts.");
  ATH_MSG_DEBUG("REGTEST : " << pixCL_1 << " have cl size == 1 in total.");
  ATH_MSG_DEBUG("REGTEST : " << pixCL_2 << " have cl size == 2 in total.");
  ATH_MSG_DEBUG("REGTEST : " << pixCLmin3 << "  have cl size >= 3 in total.");
  ATH_MSG_DEBUG("REGTEST : Formed " << pixCLnoToT << " pixel spacepoints without ToT cut in total.");
  ATH_MSG_DEBUG("REGTEST : Formed " << pixCL << " pixel spacepoints after ToT cut in total.");
  ATH_MSG_DEBUG("REGTEST : Formed " << pixCLBarrel << " SP in pixel barrel in total.");
  ATH_MSG_DEBUG("REGTEST : Formed " << pixCLEndcapA << " SP in pixel ECA in total.");
  ATH_MSG_DEBUG("REGTEST : Formed " << pixCLEndcapC << " SP in pixel ECC in total.");

  ////For SCT

  SG::ReadHandle<SpacePointContainer> SctSP(m_sctSpKey, context);
  SG::ReadHandle<SCT_ID> SctHelper(m_sctHelperKey, context);
  ATH_MSG_DEBUG("Successfully retrieved SCT SP container of size " << SctSP->size());

  // counters
  int nSctSP{};
  int sctSPBarrel{};
  int sctSPEndcapA{};
  int sctSPEndcapC{};
  int sctSP{};
  int sctModulesOverThreshold{};

  for (const auto SctSPointColl : *SctSP) {
    if (SctSPointColl == nullptr) {
      continue;
    }

    nSctSP = (SctSPointColl)->size();
    const Identifier Sctid = (SctSPointColl)->identify();
    const int bec = (int)SctHelper->barrel_ec(Sctid);

    ATH_MSG_VERBOSE(" Formed " << nSctSP << " sct spacepoints"
                               << " with sctid module " << Sctid);
    // barrel
    // total
    if (nSctSP < m_sctModuleLowerThreshold && nSctSP > m_sctModuleHigherThreshold) {
      // this is noise
      ATH_MSG_DEBUG(" This SCT module : " << Sctid << " produced " << nSctSP << " SCT spacepoints. Ignoring these spacepoints as the number of allowed spacepoints per module is between"
                                          << m_sctModuleLowerThreshold << " and " << m_sctModuleHigherThreshold);
      sctModulesOverThreshold++;
    } else {  // Accept the spacepoints
      if (bec == 0) {
        sctSPBarrel += nSctSP;
        ATH_MSG_VERBOSE(" Formed  " << nSctSP << " SCT barrel spacepoints .");
      } else if (bec == 2) {  // endcap, side A
        sctSPEndcapA += nSctSP;
        ATH_MSG_VERBOSE(" Formed  " << nSctSP << " SCT ECA spacepoints.");
      } else if (bec == -2) {  // endcap, side C
        sctSPEndcapC += nSctSP;
        ATH_MSG_VERBOSE(" Formed  " << nSctSP << " SCT ECC spacepoints.");
      }
    }
  }

  sctSP = sctSPEndcapC + sctSPBarrel + sctSPEndcapA;

  ATH_MSG_DEBUG("REGTEST : Formed  " << sctSP << " sct spacepoints in total.");
  ATH_MSG_DEBUG("REGTEST : Formed  " << sctSPEndcapC << " sct ECC spacepoints in total.");
  ATH_MSG_DEBUG("REGTEST : Formed  " << sctSPBarrel << " sct Barr spacepoints in total.");
  ATH_MSG_DEBUG("REGTEST : Formed  " << sctSPEndcapA << " sct ECA spacepoints in total.");

  // Recording Data
  SG::WriteHandle<xAOD::TrigCompositeContainer> spacePointHandle(m_spacePointsKey, context);

  auto spacePoints = std::make_unique<xAOD::TrigCompositeContainer>();
  auto spacePointsAux = std::make_unique<xAOD::TrigCompositeAuxContainer>();
  spacePoints->setStore(spacePointsAux.get());

  xAOD::TrigComposite *spCounts = new xAOD::TrigComposite();
  spacePoints->push_back(spCounts);

  std::vector<std::reference_wrapper<Monitored::IMonitoredVariable>> monitoredVariables;

#define SAVE_AND_MONITOR(__VARNAME)                                \
  spCounts->setDetail(#__VARNAME, __VARNAME);                      \
  auto mon_##__VARNAME = Monitored::Scalar(#__VARNAME, __VARNAME); \
  monitoredVariables.emplace_back(mon_##__VARNAME);

  SAVE_AND_MONITOR(pixCL);
  SAVE_AND_MONITOR(pixCLnoToT);
  SAVE_AND_MONITOR(pixCL_1);
  SAVE_AND_MONITOR(pixCL_2);
  SAVE_AND_MONITOR(pixCLmin3);
  SAVE_AND_MONITOR(pixCLBarrel);
  SAVE_AND_MONITOR(pixCLEndcapA);
  SAVE_AND_MONITOR(pixCLEndcapC);
  SAVE_AND_MONITOR(sctSP);
  SAVE_AND_MONITOR(sctSPBarrel);
  SAVE_AND_MONITOR(sctSPEndcapA);
  SAVE_AND_MONITOR(sctSPEndcapC);
  SAVE_AND_MONITOR(pixQ2mod);
#undef SAVE_AND_MONITOR

  auto mon_pixCLBeforeCuts = Monitored::Scalar<int>("pixCLBeforeCuts", pixCLBeforeCuts);
  monitoredVariables.emplace_back(mon_pixCLBeforeCuts);
  auto mon_pixModulesOverThreshold = Monitored::Scalar<int>("pixModulesOverThreshold", pixModulesOverThreshold);
  monitoredVariables.emplace_back(mon_pixModulesOverThreshold);
  auto mon_sctModulesOverThreshold = Monitored::Scalar<int>("sctModulesOverThreshold", sctModulesOverThreshold);
  monitoredVariables.emplace_back(mon_sctModulesOverThreshold);
  Monitored::Group(m_monTool, monitoredVariables);

  ATH_CHECK(spacePointHandle.record(std::move(spacePoints), std::move(spacePointsAux)));

  return StatusCode::SUCCESS;
}
