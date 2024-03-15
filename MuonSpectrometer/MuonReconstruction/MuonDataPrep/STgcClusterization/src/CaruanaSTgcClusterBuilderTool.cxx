/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "STgcClusterBuilderCommon.h"
#include "CaruanaSTgcClusterBuilderTool.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"

using namespace Muon;

Muon::CaruanaSTgcClusterBuilderTool::CaruanaSTgcClusterBuilderTool(const std::string& t, const std::string& n, const IInterface* p) :
  AthAlgTool(t,n,p)
{
}

StatusCode Muon::CaruanaSTgcClusterBuilderTool::initialize()
{
  ATH_CHECK( m_idHelperSvc.retrieve() );
  ATH_CHECK(m_DetectorManagerKey.initialize());
  ATH_CHECK(m_uncertCalibKey.initialize());
  return StatusCode::SUCCESS;
}

//
// Build the clusters given a vector of single-hit PRD
//
StatusCode Muon::CaruanaSTgcClusterBuilderTool::getClusters(const EventContext& ctx,
                                                            std::vector<Muon::sTgcPrepData>&& stripsVect, 
                                                            std::vector<std::unique_ptr<Muon::sTgcPrepData>>& clustersVect) const
{

  ATH_MSG_DEBUG("Size of the input vector: " << stripsVect.size());

  if (stripsVect.empty()) {
    ATH_MSG_DEBUG("Size of the channel vectors is zero");
    return StatusCode::SUCCESS;
  }
  
  SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
  if (!errorCalibDB.isValid()) {
      ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
      return StatusCode::FAILURE;
  }

  Muon::STgcClusterBuilderCommon stgcClusterCommon(m_idHelperSvc->stgcIdHelper(), **errorCalibDB);

  // define the identifier hash
  Identifier chanId = stripsVect.at(0).identify();
  IdentifierHash hash = m_idHelperSvc->moduleHash(chanId);
  
  int channelType = m_idHelperSvc->stgcIdHelper().channelType(stripsVect.at(0).identify());
  double resolution = Amg::error(stripsVect.at(0).localCovariance(), Trk::locX);
  ATH_MSG_DEBUG("ChannelType: " << channelType << " Single channel resolution: " << resolution);

  // Separate hits by layer (using index = 0 to 7) and sort them
  std::array<std::vector<Muon::sTgcPrepData>, 8> stgcPrdsPerLayer = stgcClusterCommon.sortSTGCPrdPerLayer(std::move(stripsVect));

  SG::ReadCondHandle<MuonGM::MuonDetectorManager> detManager{m_DetectorManagerKey, ctx};
  if(!detManager.isValid()){
    ATH_MSG_ERROR("Null pointer to the read MuonDetectorManager conditions object");
    return StatusCode::FAILURE;
  }

  // Loop over the 8 layers, from 0 to 7
  for (std::vector<sTgcPrepData>& unmerged : stgcPrdsPerLayer) {
      // Get the strip clusters of the layer.
      std::vector<std::vector<Muon::sTgcPrepData>> layerClusters = stgcClusterCommon.findStripCluster(std::move(unmerged),
                                                                                                       m_maxHoleSize);

      // Loop over the clusters of that gap
      for (std::vector<Muon::sTgcPrepData>& cluster: layerClusters) {
        sTgcPrepData::Author author = sTgcPrepData::Author::unKnown;
        std::vector<Identifier> rdoList;
        //vectors to hold the properties of the elements of a cluster
        std::vector<int> elementsCharge;
        std::vector<short int> elementsTime;
        std::vector<uint16_t> elementsChannel;
        std::vector<double> elementsLocalPositions;
        std::vector<Identifier> elementsIdentifier;
        double posY = (cluster.at(0)).localPosition().y();

        // loop on the strips and set the cluster weighted position and charge
        for (const sTgcPrepData& it : cluster ){
          rdoList.push_back(it.identify());
          elementsCharge.push_back(it.charge());
          elementsChannel.push_back(m_idHelperSvc->stgcIdHelper().channel(it.identify()));
          elementsTime.push_back(it.time());
          elementsLocalPositions.push_back(it.localPosition().x());
          elementsIdentifier.push_back(it.identify());
        }

        // Use the Caruana method to reconstruct clusters, determining the cluster mean position and error.
        // If the Caruana method fails at some point, the reconstruction reverts to the weighted average method.
        Identifier clusterId;
        double reconstructedPosX{0};
        double sigmaSq{0};
        std::optional<Muon::STgcClusterPosition> optStripCluster = stgcClusterCommon.caruanaGaussianFitting(cluster,
                                                                                                            m_positionStripResolution,
                                                                                                            m_angularStripResolution,
                                                                                                            detManager.cptr());
        if (optStripCluster.has_value()) {
          clusterId = (*optStripCluster).getClusterId();
          reconstructedPosX = (*optStripCluster).getMeanPosition();
          sigmaSq = (*optStripCluster).getErrorSquared();
          author = sTgcPrepData::Author::Caruana;
        } else {
          ATH_MSG_DEBUG("sTGC cluster reconstruction using the Caruana method failed, reversing to the weighted average");

          // Fallback to the weighted average when the Caruana method fails
          bool isStrip = (channelType == sTgcIdHelper::sTgcChannelTypes::Strip);
          // Calculate the cluster position and error using the weighted average
          std::optional<Muon::STgcClusterPosition> optClusterWA = stgcClusterCommon.weightedAverage(cluster, resolution, isStrip);
          // Skip the cluster if the weighted average also fails
          if (!optClusterWA) {
            if (msgLvl(MSG::VERBOSE)) {
              std::stringstream sstr{};
              for (const Muon::sTgcPrepData& prd : cluster) {
                  sstr << m_idHelperSvc->stgcIdHelper().print_to_string(prd.identify())
                       << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
                       << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
                       << std::endl;
              }
              ATH_MSG_VERBOSE("Reject invalid cluster..." << std::endl << std::endl << sstr.str());
            }
            continue;
          }
          author = sTgcPrepData::Author::SimpleClusterBuilder;

          clusterId = (*optClusterWA).getClusterId();
          reconstructedPosX = (*optClusterWA).getMeanPosition();
          sigmaSq = (*optClusterWA).getErrorSquared();
        }

        Amg::Vector2D localPosition(reconstructedPosX,posY);
        auto covN = Amg::MatrixX(1,1);
        covN(0,0) = sigmaSq;
        ATH_MSG_DEBUG("Reconstructed a cluster at mean local position: "<< Amg::toString(localPosition,2)
                  << " with error on cluster: " << std::sqrt((covN)(0,0)));

        std::unique_ptr<sTgcPrepData> prdN = std::make_unique<sTgcPrepData>(clusterId,
                                                                            hash,
                                                                            std::move(localPosition),
                                                                            std::move(rdoList),
                                                                            std::move(covN),
                                                                            cluster.at(0).detectorElement(),
                                                                            std::accumulate(elementsCharge.begin(), elementsCharge.end(), 0),
                                                                            (short int)0,
                                                                            std::move(elementsChannel),
                                                                            std::move(elementsTime),
                                                                            std::move(elementsCharge));
        prdN->setAuthor(author);
        clustersVect.push_back(std::move(prdN));
      }
  }

  ATH_MSG_DEBUG("Size of the output cluster vector: " << clustersVect.size());

  return StatusCode::SUCCESS;
}


/// Dump cluster information
void CaruanaSTgcClusterBuilderTool::dumpStrips( std::vector<Muon::sTgcPrepData>& stripsVect,
					       std::vector<Muon::sTgcPrepData*>& clustersVect ) const
{

  ATH_MSG_INFO("====> Dumping all strips:  ");
  for ( const auto& it : stripsVect ) {
    Identifier stripId = it.identify();
    ATH_MSG_INFO("Strip identifier: " << m_idHelperSvc->stgcIdHelper().show_to_string(stripId) );
  }

  ATH_MSG_INFO("Dumping all clusters:  ");
  for ( auto *it : clustersVect ) {
    Identifier clusterId = it->identify();
    ATH_MSG_INFO("***> New cluster identifier: " << m_idHelperSvc->stgcIdHelper().show_to_string(clusterId) );
    ATH_MSG_INFO("Cluster size: " << it->rdoList().size() );
    ATH_MSG_INFO("List of associated RDO's: ");

  }

  }
