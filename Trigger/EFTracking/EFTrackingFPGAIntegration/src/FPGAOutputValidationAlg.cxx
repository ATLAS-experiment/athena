/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "FPGAOutputValidationAlg.h"

FPGAOutputValidationAlg::FPGAOutputValidationAlg(
  const std::string& name,
  ISvcLocator* pSvcLocator
) : AthReentrantAlgorithm(name, pSvcLocator)
{}

StatusCode FPGAOutputValidationAlg::initialize() {
  ATH_CHECK(m_pixelKeys.initialize(!m_pixelKeys.empty()));
  ATH_CHECK(m_stripKeys.initialize(!m_stripKeys.empty()));

  ATH_CHECK(m_monitoringTool.retrieve());
  
  return StatusCode::SUCCESS;
}

StatusCode FPGAOutputValidationAlg::execute(const EventContext& ctx) const { 

  if (m_pixelKeys.size() == 2) { /// just compare two for now
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key0 = m_pixelKeys[0];
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key1 = m_pixelKeys[1];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle0{key0, ctx};
    ATH_CHECK(handle0.isValid());
    SG::ReadHandle<xAOD::PixelClusterContainer> handle1{key1, ctx};
    ATH_CHECK(handle1.isValid());
    const xAOD::PixelClusterContainer pixelClusters1 = *handle1; 
    for (auto cluster0 : *handle0) {
      std::vector<const xAOD::PixelCluster*> matchedClusters = findMatchingCluster(cluster0, pixelClusters1);
      auto mon_nmatch = Monitored::Scalar<unsigned>("nmatched_clusters", matchedClusters.size());
      if (matchedClusters.size() == 1) {
	const xAOD::PixelCluster *cluster1 = matchedClusters[0];
	auto mon_diff_locx = Monitored::Scalar<float>("diff_locx",cluster0->localPosition<2>()[0] - cluster1->localPosition<2>()[0]);
	auto mon_diff_locy = Monitored::Scalar<float>("diff_locy",cluster0->localPosition<2>()[1] - cluster1->localPosition<2>()[1]);
	auto mon_diff_covxx = Monitored::Scalar<float>("diff_covxx",cluster0->localCovariance<2>()(0, 0) - cluster1->localCovariance<2>()(0, 0));
	auto mon_diff_covyy = Monitored::Scalar<float>("diff_covyy",cluster0->localCovariance<2>()(1, 1) - cluster1->localCovariance<2>()(1, 1));	
	auto mon_diff_omegax = Monitored::Scalar<float>("diff_omegax,",cluster0->omegaX() - cluster1->omegaX());
	auto mon_diff_omegay = Monitored::Scalar<float>("diff_omegay,",cluster0->omegaY() - cluster1->omegaY());
	auto mon_diff_globalx = Monitored::Scalar<float>("diff_globalx",cluster0->globalPosition()[0] - cluster1->globalPosition()[0]);
	auto mon_diff_globaly = Monitored::Scalar<float>("diff_globaly",cluster0->globalPosition()[1] - cluster1->globalPosition()[1]);
	auto mon_diff_globalz = Monitored::Scalar<float>("diff_globalz",cluster0->globalPosition()[2] - cluster1->globalPosition()[2]);
	auto mon_diff_channelsphi = Monitored::Scalar<float>("diff_channelsphi",cluster0->channelsInPhi() - cluster1->channelsInPhi());
	auto mon_diff_channelseta = Monitored::Scalar<float>("diff_channelseta",cluster0->channelsInEta() - cluster1->channelsInEta());
	auto mon_diff_widtheta = Monitored::Scalar<float>("diff_widtheta",cluster0->widthInEta() - cluster1->widthInEta());
	auto mon_diff_tot = Monitored::Scalar<float>("diff_tot",cluster0->totalToT() - cluster1->totalToT());
	Monitored::Group(m_monitoringTool,mon_diff_locx,mon_diff_locy,mon_diff_covxx,mon_diff_covyy,mon_diff_omegax,mon_diff_omegay,mon_diff_globalx,
			 mon_diff_globaly,mon_diff_globalz,mon_diff_channelsphi,mon_diff_channelseta,mon_diff_widtheta,mon_diff_tot);
      }
      Monitored::Group(m_monitoringTool,mon_nmatch);
    }
  }

  for (std::size_t index = 0; index < m_pixelKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key = m_pixelKeys[index];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle{key, ctx};

    Monitored::Group(
      m_monitoringTool,
      Monitored::Collection(key.key() + "_LOCALPOSITION_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localPosition<2>()[0];
      }),
      Monitored::Collection(key.key() + "_LOCALPOSITION_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localPosition<2>()[1];
      }),
      Monitored::Collection(key.key() + "_LOCALCOVARIANCE_XX", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localCovariance<2>()(0, 0);
      }),
      Monitored::Collection(key.key() + "_LOCALCOVARIANCE_YY", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->localCovariance<2>()(1, 1);
      }),
      Monitored::Collection(key.key() + "_OMEGA_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->omegaX();
      }),
      Monitored::Collection(key.key() + "_OMEGA_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->omegaY();
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_X", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[0];
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_Y", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[1];
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_Z", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->globalPosition()[2];
      }),
      Monitored::Collection(key.key() + "_CHANNELS_IN_PHI", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->channelsInPhi();
      }),
      Monitored::Collection(key.key() + "_CHANNELS_IN_ETA", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->channelsInEta();
      }),
      Monitored::Collection(key.key() + "_WIDTH_IN_ETA", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->widthInEta();
      }),
      Monitored::Collection(key.key() + "_TOTAL_TOT", *handle, [](const xAOD::PixelCluster* cluster){
        return cluster->totalToT();
      })
    );
  }
      
  for (std::size_t index = 0; index < m_stripKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key = m_stripKeys[index];
    SG::ReadHandle<xAOD::StripClusterContainer> handle{key, ctx};

    Monitored::Group(
      m_monitoringTool,
      Monitored::Collection(key.key() + "_LOCALPOSITION_X", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localPosition<1>()(0,0);
      }),
      Monitored::Collection(key.key() + "_LOCALCOVARIANCE_XX", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->localCovariance<1>()(0, 0);
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_X", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[0];
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_Y", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[1];
      }),
      Monitored::Collection(key.key() + "_GLOBALPOSITION_Z", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->globalPosition()[2];
      }),
      Monitored::Collection(key.key() + "_CHANNELS_IN_PHI", *handle, [](const xAOD::StripCluster* cluster){
        return cluster->channelsInPhi();
      })
    );
  }

  return StatusCode::SUCCESS;
}

std::vector<const xAOD::PixelCluster*> FPGAOutputValidationAlg::findMatchingCluster(const xAOD::PixelCluster* cluster0, const xAOD::PixelClusterContainer& pixelClusters1) const {
  std::vector<const xAOD::PixelCluster*> matchedClusters;
  const xAOD::DetectorIDHashType cl0 = cluster0->identifierHash();
  std::vector<Identifier> rdoList0 = cluster0->rdoList();
  std::sort(rdoList0.begin(), rdoList0.end()); 
  
  for (auto cluster1 : pixelClusters1) {
    const xAOD::DetectorIDHashType cl1 = cluster1->identifierHash();
    if (cl0 != cl1) continue;
    std::vector<Identifier> rdoList1 = cluster0->rdoList();
    std::sort (rdoList1.begin(), rdoList1.end());
    std::vector<Identifier> rdoMatchList;
    std::set_intersection(rdoList0.begin(), rdoList0.end(), rdoList1.begin(), rdoList1.end(), back_inserter(rdoMatchList));
    if (rdoMatchList.size() > 0) { // call this a match!
      matchedClusters.push_back(cluster1);
    }
  }
  return matchedClusters;
}
