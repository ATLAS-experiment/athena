/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#include "EFTrackingFPGAOutputValidation/FPGAOutputValidationAlg.h"
#include "InDetMeasurementUtilities/Helpers.h"

namespace {
  template <typename T>
  std::vector<const T*> findMatchingCluster(const T* cluster0, const std::unordered_multimap<xAOD::DetectorIdentType, const T*>& clusterMap,bool matchByID, const size_t& allowedMisses) {
    std::vector<const T*> matchedClusters{};

    std::unordered_set<Identifier> rdoSet0{};
    for (const auto& rdo : cluster0->rdoList()) {
      rdoSet0.insert(rdo);
    }
    const size_t rdoToMiss = (allowedMisses < rdoSet0.size()) ? allowedMisses : (rdoSet0.size()-1);
    if (matchByID) {
      auto range = clusterMap.equal_range(cluster0->identifier());
      for (auto it = range.first; it != range.second; ++it) {
        const auto& rdoList1 = it->second->rdoList();
        size_t matchedRdoIDs=0;
        for (const auto& rdo : rdoList1) {
          matchedRdoIDs+= rdoSet0.count(rdo);
          }
        if (matchedRdoIDs >= rdoSet0.size() - rdoToMiss) {
          matchedClusters.push_back(it->second);
        }
      }
    } else {
      for (const auto& [hash, cluster1] : clusterMap) {
        const auto& rdoList1 = cluster1->rdoList();
        size_t matchedRdoIDs=0;
        for (const auto& rdo : rdoList1) {
          matchedRdoIDs+= rdoSet0.count(rdo);
        }
        if (matchedRdoIDs >= rdoSet0.size() - rdoToMiss) {
          matchedClusters.push_back(cluster1);
        }
      }
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
  ATH_MSG_INFO("Initializing FPGAOutputValidationAlg");

  ATH_CHECK(m_pixelKeys.initialize(!m_pixelKeys.empty()));
  ATH_CHECK(m_stripKeys.initialize(!m_stripKeys.empty()));

  ATH_CHECK(m_monitoringTool.retrieve());
  
  ATH_CHECK(m_chrono.retrieve());

  ATH_CHECK(detStore()->retrieve(m_pixelid, "PixelID"));
  ATH_CHECK(detStore()->retrieve(m_stripid, "SCT_ID"));
  ATH_CHECK(detStore()->retrieve(m_SCT_mgr, "ITkStrip"));
  ATH_CHECK(detStore()->retrieve(m_PIX_mgr, "ITkPixel"));

  return StatusCode::SUCCESS;
}

StatusCode FPGAOutputValidationAlg::execute(const EventContext& ctx) const { 
  if (m_pixelKeys.size() == 2 && m_doDiffHistograms) { /// just compare two for now
    m_chrono->chronoStart("FPGAOutputValidationAlg::pixel diff");
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key0 = m_pixelKeys[0];
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key1 = m_pixelKeys[1];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle0{key0, ctx};
    ATH_CHECK(handle0.isValid());
    SG::ReadHandle<xAOD::PixelClusterContainer> handle1{key1, ctx};
    ATH_CHECK(handle1.isValid());

    const xAOD::PixelClusterContainer pixelClusters1 = *handle1;
    std::unordered_multimap<xAOD::DetectorIdentType, const xAOD::PixelCluster*> pixelClustersMap1;
    for (const auto* cluster1 : pixelClusters1) {
      const xAOD::DetectorIdentType hashId1 = cluster1->identifier();
      pixelClustersMap1.insert(std::make_pair(hashId1, cluster1));
    }

    for (auto cluster0 : *handle0) {
      const std::vector<const xAOD::PixelCluster*> matchedClusters = findMatchingCluster(cluster0, pixelClustersMap1,
                                                                                         m_matchByID,
                                                                                         m_allowedRdoMisses);

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<float>("nmatched_pixel_clusters", matchedClusters.size() - 0.5)
      );

      if (matchedClusters.size() == 0) {
        continue;
      }
      if (matchedClusters.size() > 1) {
        std::stringstream ss;
        for (const auto& cluster : matchedClusters) {
          ss << cluster->identifier()
             << " x: " << cluster->globalPosition().x() 
             << " y: " << cluster->globalPosition().y() 
             << " z: " << cluster->globalPosition().z() << "\n";
          for (const auto& rdo : cluster->rdoList()) {
            ss << "\t" << rdo.get_compact() << "\n";
          }
          ss << "\n";
        }
        ATH_MSG_ERROR("Found " << matchedClusters.size() << " pixel cluster matches\n" << ss.str());
        return StatusCode::FAILURE;
      }

      const xAOD::PixelCluster *cluster1 = matchedClusters[0];
      
      std::vector<std::string> regions {"all"};
      if(m_pixelid->barrel_ec(cluster0->rdoList()[0]) == 0) regions.push_back("barrel");
      else regions.push_back("endcap");

      for(auto const& region: regions)
      {
        Monitored::Group(
          m_monitoringTool,
          Monitored::Scalar<float>("diff_pixel_locx_" +region , cluster0->localPosition<2>()[0] - cluster1->localPosition<2>()[0]),
          Monitored::Scalar<float>("diff_pixel_locy_" +region , cluster0->localPosition<2>()[1] - cluster1->localPosition<2>()[1]),
          Monitored::Scalar<float>("diff_pixel_covxx_" +region , cluster0->localCovariance<2>()(0, 0) - cluster1->localCovariance<2>()(0, 0)),
          Monitored::Scalar<float>("diff_pixel_covyy_" +region , cluster0->localCovariance<2>()(1, 1) - cluster1->localCovariance<2>()(1, 1)),
          Monitored::Scalar<float>("diff_pixel_globalx_" +region , cluster0->globalPosition()[0] - cluster1->globalPosition()[0]),
          Monitored::Scalar<float>("diff_pixel_globaly_" +region , cluster0->globalPosition()[1] - cluster1->globalPosition()[1]),
          Monitored::Scalar<float>("diff_pixel_globalz_" +region , cluster0->globalPosition()[2] - cluster1->globalPosition()[2]),
          Monitored::Scalar<int>("diff_pixel_channelsphi_" +region , cluster0->channelsInPhi() - cluster1->channelsInPhi()),
          Monitored::Scalar<int>("diff_pixel_channelseta_" +region , cluster0->channelsInEta() - cluster1->channelsInEta()),
          Monitored::Scalar<float>("diff_pixel_widtheta_" +region , cluster0->widthInEta() - cluster1->widthInEta()),
          Monitored::Scalar<int>("diff_pixel_tot_" +region , cluster0->totalToT() - cluster1->totalToT())
        );
      }
    }
    m_chrono->chronoStop("FPGAOutputValidationAlg::pixel diff");
  }

  if (m_stripKeys.size() == 2 && m_doDiffHistograms) { /// just compare two for now
    m_chrono->chronoStart("FPGAOutputValidationAlg::strip diff");
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key0 = m_stripKeys[0];
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key1 = m_stripKeys[1];
    SG::ReadHandle<xAOD::StripClusterContainer> handle0{key0, ctx};
    ATH_CHECK(handle0.isValid());
    SG::ReadHandle<xAOD::StripClusterContainer> handle1{key1, ctx};
    ATH_CHECK(handle1.isValid());

    const xAOD::StripClusterContainer stripClusters1 = *handle1; 
    std::unordered_multimap<xAOD::DetectorIdentType, const xAOD::StripCluster*> stripClustersMap1;
    for (const auto *cluster1 : stripClusters1) {
      const xAOD::DetectorIdentType hashId1 = cluster1->identifier();
      stripClustersMap1.insert(std::make_pair(hashId1, cluster1));
    }

    for (auto cluster0 : *handle0) {
      const std::vector<const xAOD::StripCluster*> matchedClusters = findMatchingCluster(cluster0, stripClustersMap1,
                                                                                         m_matchByID,
                                                                                         m_allowedRdoMisses);

      Monitored::Group(
        m_monitoringTool,
        Monitored::Scalar<unsigned>("nmatched_strip_clusters", matchedClusters.size())
      );

      if (matchedClusters.size() == 0) {
        continue;
      }
      if (matchedClusters.size() > 1) {
        std::stringstream ss;
        for (const auto& cluster : matchedClusters) {
          ss << cluster->identifier()
             << " x: " << cluster->globalPosition().x() 
             << " y: " << cluster->globalPosition().y() 
             << " z: " << cluster->globalPosition().z() << "\n";
          for (const auto& rdo : cluster->rdoList()) {
            ss << "\t" << rdo.get_compact() << "\n";
          }
          ss << "\n";
        }
        ATH_MSG_ERROR("Found " << matchedClusters.size() << " strip cluster matches\n" << ss.str());
        return StatusCode::FAILURE;
      }

      const xAOD::StripCluster *cluster1 = matchedClusters[0];
      std::vector<std::string> regions {"all"};
      if(m_stripid->barrel_ec(cluster0->rdoList()[0]) == 0) regions.push_back("barrel");
      else regions.push_back("endcap");

      for(auto const& region: regions)
      {
        Monitored::Group(
          m_monitoringTool,
          Monitored::Scalar<float>("diff_strip_locx_" + region, cluster0->localPosition<1>()[0] - cluster1->localPosition<1>()[0]),
          Monitored::Scalar<float>("diff_strip_covxx_" + region, cluster0->localCovariance<1>()(0, 0) - cluster1->localCovariance<1>()(0, 0)),
          Monitored::Scalar<float>("diff_strip_globalx_" + region, cluster0->globalPosition()[0] - cluster1->globalPosition()[0]),
          Monitored::Scalar<float>("diff_strip_globaly_" + region, cluster0->globalPosition()[1] - cluster1->globalPosition()[1]),
          Monitored::Scalar<float>("diff_strip_globalz_" + region, cluster0->globalPosition()[2] - cluster1->globalPosition()[2]),
          Monitored::Scalar<float>("diff_strip_channelsphi_" + region, cluster0->channelsInPhi() - cluster1->channelsInPhi())
        );
      }
    }
    m_chrono->chronoStop("FPGAOutputValidationAlg::strip diff");
  }

  for (std::size_t index = 0; index < m_pixelKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::PixelClusterContainer>& key = m_pixelKeys[index];
    SG::ReadHandle<xAOD::PixelClusterContainer> handle{key, ctx};
    ATH_CHECK(handle.isValid());


    for(auto cluster : *handle)
    {
      std::vector<std::string> regions {"all"};
      if(m_pixelid->barrel_ec(cluster->rdoList()[0]) == 0) regions.push_back("barrel");
      else regions.push_back("endcap");

      for(auto const& region: regions)
      {
        Monitored::Group(
          m_monitoringTool,
          Monitored::Scalar<float>(key.key() + "_LOCALPOSITION_X_" + region, cluster->localPosition<2>()[0]),
          Monitored::Scalar<float>(key.key() + "_LOCALPOSITION_Y_" + region, cluster->localPosition<2>()[1]),
          Monitored::Scalar<float>(key.key() + "_LOCALCOVARIANCE_XX_" + region, cluster->localCovariance<2>()(0, 0)),
          Monitored::Scalar<float>(key.key() + "_LOCALCOVARIANCE_YY_" + region, cluster->localCovariance<2>()(1, 1)),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_X_" + region, cluster->globalPosition()[0]),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_Y_" + region, cluster->globalPosition()[1]),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_Z_" + region, cluster->globalPosition()[2]),
          Monitored::Scalar<int>(key.key() + "_CHANNELS_IN_PHI_" + region, cluster->channelsInPhi()),
          Monitored::Scalar<int>(key.key() + "_CHANNELS_IN_ETA_" + region, cluster->channelsInEta()),
          Monitored::Scalar<float>(key.key() + "_WIDTH_IN_ETA_" + region, cluster->widthInEta()),
          Monitored::Scalar<int>(key.key() + "_TOTAL_TOT_" + region, cluster->totalToT())
        );
      }
    }
  }
      
  for (std::size_t index = 0; index < m_stripKeys.size(); index++) {
    const SG::ReadHandleKey<xAOD::StripClusterContainer>& key = m_stripKeys[index];
    SG::ReadHandle<xAOD::StripClusterContainer> handle{key, ctx};
    ATH_CHECK(handle.isValid());

    for(auto cluster : *handle)
    {
      std::vector<std::string> regions {"all"};
      if(m_stripid->barrel_ec(cluster->rdoList()[0]) == 0) regions.push_back("barrel");
      else regions.push_back("endcap");

      for(auto const& region: regions)
      {
        Monitored::Group(
          m_monitoringTool,
          Monitored::Scalar<float>(key.key() + "_LOCALPOSITION_X_" + region, cluster->localPosition<1>()(0,0)),
          Monitored::Scalar<float>(key.key() + "_LOCALCOVARIANCE_XX_" + region, cluster->localCovariance<1>()(0, 0)),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_X_" + region, cluster->globalPosition()[0]),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_Y_" + region, cluster->globalPosition()[1]),
          Monitored::Scalar<float>(key.key() + "_GLOBALPOSITION_Z_" + region, cluster->globalPosition()[2]),
          Monitored::Scalar<float>(key.key() + "_CHANNELS_IN_PHI_" + region, cluster->channelsInPhi())
        );
      }
    }
  }

  return StatusCode::SUCCESS;
}

