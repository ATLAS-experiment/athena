/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "STgcClusterBuilderCommon.h"
#include "SimpleSTgcClusterBuilderTool.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "EventPrimitives/EventPrimitivesHelpers.h"
#include "GeoPrimitives/GeoPrimitivesToStringConverter.h"


using namespace Muon;

//============================================================================
Muon::SimpleSTgcClusterBuilderTool::SimpleSTgcClusterBuilderTool(const std::string& t, const std::string& n, const IInterface* p) 
: AthAlgTool(t,n,p) {}


//============================================================================
StatusCode Muon::SimpleSTgcClusterBuilderTool::initialize() {
    ATH_CHECK( m_idHelperSvc.retrieve() );
    ATH_CHECK(m_uncertCalibKey.initialize());
    return StatusCode::SUCCESS;
}


//============================================================================
// Build the clusters given a vector of single-hit PRD
StatusCode Muon::SimpleSTgcClusterBuilderTool::getClusters(const EventContext& ctx,
                                                           std::vector<Muon::sTgcPrepData>&& stripsVect, 
                                                           std::vector<std::unique_ptr<Muon::sTgcPrepData>>& clustersVect) const {

    ATH_MSG_DEBUG("Size of the input vector: " << stripsVect.size()); 

    if (stripsVect.empty()) return StatusCode::SUCCESS;
  

    // define the identifier hash
    Identifier chanId = stripsVect.at(0).identify();
    IdentifierHash hash = m_idHelperSvc->moduleHash(chanId);
  
    double resolution = Amg::error(stripsVect.at(0).localCovariance(), Trk::locX);
    bool isStrip = ( m_idHelperSvc->stgcIdHelper().channelType(chanId) == sTgcIdHelper::Strip );
    ATH_MSG_DEBUG(" channelType " << m_idHelperSvc->stgcIdHelper().channelType(chanId));
    ATH_MSG_DEBUG(" isStrip: " << isStrip << "Single channel resolution: " << resolution);

    SG::ReadCondHandle<NswErrorCalibData> errorCalibDB{m_uncertCalibKey, ctx};
    if (!errorCalibDB.isValid()) {
        ATH_MSG_FATAL("Failed to retrieve the parameterized errors "<<m_uncertCalibKey.fullKey());
        return StatusCode::FAILURE;
    }

    Muon::STgcClusterBuilderCommon stgcClusterCommon(m_idHelperSvc->stgcIdHelper(), **errorCalibDB);
    std::array<std::vector<Muon::sTgcPrepData>, 8> stgcPrdsPerLayer = stgcClusterCommon.sortSTGCPrdPerLayer(std::move(stripsVect));
 
    for (std::vector<Muon::sTgcPrepData>& layPrds : stgcPrdsPerLayer) {
        // Get the strip clusters of the layer.
        std::vector<std::vector<Muon::sTgcPrepData>> layerClusters = stgcClusterCommon.findStripCluster(std::move(layPrds),
                                                                                                        m_maxHoleSize);

        // Loop on the clusters of that gap
        for (const std::vector<Muon::sTgcPrepData>& cluster: layerClusters) {
            if (cluster.empty()) continue;

            // Calculate the cluster position and error using the weighted average
            std::optional<Muon::STgcClusterPosition> optClusterPos = stgcClusterCommon.weightedAverage(cluster, resolution, isStrip);
            if (!optClusterPos) {
              if (msgLvl(MSG::VERBOSE)) {
                std::stringstream sstr{};
                for (const Muon::sTgcPrepData& prd : cluster) {
                    sstr << m_idHelperSvc->toString(prd.identify())
                         << ", local pos: "<< Amg::toString(prd.localPosition(), 2)
                         << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
                         << std::endl;
                }
                ATH_MSG_VERBOSE("Reject invalid cluster..." << std::endl << std::endl << sstr.str());
              }
              continue;
            }

            Identifier clusterId = (*optClusterPos).getClusterId();
            double posY = cluster[0].localPosition().y();
            Amg::Vector2D localPosition((*optClusterPos).getMeanPosition(), posY);
            auto covN = Amg::MatrixX(1,1);
            covN(0,0) = (*optClusterPos).getErrorSquared();

            std::vector<Identifier> rdoList;
            std::vector<int>        elementsCharge;
            std::vector<short int>  elementsTime;
            std::vector<uint16_t>   elementsChannel;
            for (const Muon::sTgcPrepData& prd : cluster ) {
                rdoList.push_back(prd.identify());
                elementsCharge.push_back(prd.charge());
                elementsChannel.push_back(m_idHelperSvc->stgcIdHelper().channel(prd.identify()));
                elementsTime.push_back(prd.time());
            }

            // memory allocated dynamically for the PrepRawData is managed by Event Store in the converters
            ATH_MSG_DEBUG("error on cluster " << std::sqrt((covN)(0,0)));

            std::unique_ptr<sTgcPrepData> prdN =  std::make_unique<sTgcPrepData>(
                                                      clusterId,
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
            
            prdN->setAuthor(sTgcPrepData::Author::SimpleClusterBuilder);
            clustersVect.push_back(std::move(prdN));
        }
    }

    ATH_MSG_DEBUG("Size of the output cluster vector: " << clustersVect.size());

    return StatusCode::SUCCESS;
}


//============================================================================
/// sort the strips if needed
void SimpleSTgcClusterBuilderTool::dumpStrips(std::vector<Muon::sTgcPrepData>& stripsVect, std::vector<Muon::sTgcPrepData*>& clustersVect ) const {
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
