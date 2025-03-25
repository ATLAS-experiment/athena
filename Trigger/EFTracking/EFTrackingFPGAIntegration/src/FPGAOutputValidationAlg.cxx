/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "FPGAOutputValidationAlg.h"

namespace {
template <typename T>
std::vector<const T*> findMatchingCluster(const T* cluster0, const DataVector<T>& clusters1) {
  std::vector<const T*> matchedClusters{};
  const xAOD::DetectorIDHashType hashId0 = cluster0->identifierHash();

  std::vector<Identifier> rdoList0 = cluster0->rdoList();
  std::sort(rdoList0.begin(), rdoList0.end()); 
  
  for (auto cluster1 : clusters1) {
    const xAOD::DetectorIDHashType hashId1 = cluster1->identifierHash();

    if (hashId0 != hashId1) {
      continue;
    }

    std::vector<Identifier> rdoList1 = cluster1->rdoList();
    std::sort(rdoList1.begin(), rdoList1.end());
    std::vector<Identifier> rdoMatchList{};
    std::set_intersection(rdoList0.begin(), 
                          rdoList0.end(), 
                          rdoList1.begin(), 
                          rdoList1.end(), 
                          back_inserter(rdoMatchList));

    if (rdoMatchList.size() == 0) {
      continue;
    }

    matchedClusters.push_back(cluster1);
  }

  return matchedClusters;
}
}

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
  if (m_pixelKeys.size() == 2 && m_doDiffHistograms) { /// just compare two for now
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key0 = m_pixelKeys[0];
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key1 = m_pixelKeys[1];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle0{key0, ctx};
    ATH_CHECK(handle0.isValid());
    SG::ReadHandle<xAOD::PixelClusterContainer> handle1{key1, ctx};
    ATH_CHECK(handle1.isValid());
    const xAOD::PixelClusterContainer pixelClusters1 = *handle1; 
    for (auto cluster0 : *handle0) {
      const std::vector<const xAOD::PixelCluster*> matchedClusters = findMatchingCluster(cluster0, pixelClusters1);

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<unsigned>("nmatched_pixel_clusters", matchedClusters.size())
      );

      if (matchedClusters.size() == 0) {
        continue;
      }

      const xAOD::PixelCluster *cluster1 = matchedClusters[0];

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<float>("diff_pixel_locx",cluster0->localPosition<2>()[0] - cluster1->localPosition<2>()[0]),
        Monitored::Scalar<float>("diff_pixel_locy",cluster0->localPosition<2>()[1] - cluster1->localPosition<2>()[1]),
        Monitored::Scalar<float>("diff_pixel_covxx",cluster0->localCovariance<2>()(0, 0) - cluster1->localCovariance<2>()(0, 0)),
        Monitored::Scalar<float>("diff_pixel_covyy",cluster0->localCovariance<2>()(1, 1) - cluster1->localCovariance<2>()(1, 1)),  
        Monitored::Scalar<float>("diff_pixel_omegax,",cluster0->omegaX() - cluster1->omegaX()),
        Monitored::Scalar<float>("diff_pixel_omegay,",cluster0->omegaY() - cluster1->omegaY()),
        Monitored::Scalar<float>("diff_pixel_globalx",cluster0->globalPosition()[0] - cluster1->globalPosition()[0]),
        Monitored::Scalar<float>("diff_pixel_globaly",cluster0->globalPosition()[1] - cluster1->globalPosition()[1]),
        Monitored::Scalar<float>("diff_pixel_globalz",cluster0->globalPosition()[2] - cluster1->globalPosition()[2]),
        Monitored::Scalar<float>("diff_pixel_channelsphi",cluster0->channelsInPhi() - cluster1->channelsInPhi()),
        Monitored::Scalar<float>("diff_pixel_channelseta",cluster0->channelsInEta() - cluster1->channelsInEta()),
        Monitored::Scalar<float>("diff_pixel_widtheta",cluster0->widthInEta() - cluster1->widthInEta()),
        Monitored::Scalar<float>("diff_pixel_tot",cluster0->totalToT() - cluster1->totalToT())
      );
    }
  }

  if (m_stripKeys.size() == 2 && m_doDiffHistograms) { /// just compare two for now
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key0 = m_stripKeys[0];
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key1 = m_stripKeys[1];
    SG::ReadHandle<xAOD::StripClusterContainer> handle0{key0, ctx};
    ATH_CHECK(handle0.isValid());
    SG::ReadHandle<xAOD::StripClusterContainer> handle1{key1, ctx};
    ATH_CHECK(handle1.isValid());
    const xAOD::StripClusterContainer stripClusters1 = *handle1; 
    for (auto cluster0 : *handle0) {
      const std::vector<const xAOD::StripCluster*> matchedClusters = findMatchingCluster(cluster0, stripClusters1);

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<unsigned>("nmatched_strip_clusters", matchedClusters.size())
      );

      if (matchedClusters.size() == 0) {
        continue;
      }

      const xAOD::StripCluster *cluster1 = matchedClusters[0];

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<float>("diff_strip_locx",cluster0->localPosition<1>()[0] - cluster1->localPosition<1>()[0]),
        Monitored::Scalar<float>("diff_strip_covxx",cluster0->localCovariance<1>()(0, 0) - cluster1->localCovariance<1>()(0, 0)),
        Monitored::Scalar<float>("diff_strip_globalx",cluster0->globalPosition()[0] - cluster1->globalPosition()[0]),
        Monitored::Scalar<float>("diff_strip_globaly",cluster0->globalPosition()[1] - cluster1->globalPosition()[1]),
        Monitored::Scalar<float>("diff_strip_globalz",cluster0->globalPosition()[2] - cluster1->globalPosition()[2]),
        Monitored::Scalar<float>("diff_strip_channelsphi",cluster0->channelsInPhi() - cluster1->channelsInPhi())
      );
    }
  }

  for (std::size_t index = 0; index < m_pixelKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key = m_pixelKeys[index];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle{key, ctx};
    ATH_CHECK(handle.isValid());

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
    ATH_CHECK(handle.isValid());

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

