/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#include "STgcClusterBuilderCommon.h"
#include "MuonPrepRawData/sTgcPrepData.h"
#include "MuonReadoutGeometry/MuonDetectorManager.h"


Muon::STgcClusterBuilderCommon::STgcClusterBuilderCommon(const sTgcIdHelper& idHelper, const NswErrorCalibData& errorCalibData)
        : AthMessaging("STgcClusterBuilderCommon"),
          m_stgcIdHelper(idHelper),
          m_errorCalibData{errorCalibData}
{
}


//=============================================================================
std::array<std::vector<Muon::sTgcPrepData>, 8>
Muon::STgcClusterBuilderCommon::sortSTGCPrdPerLayer(std::vector<Muon::sTgcPrepData>&& stripPrds) const {

    
    std::array<std::vector<Muon::sTgcPrepData>, 8> stgcPrdsPerLayer;
    for (Muon::sTgcPrepData& prd : stripPrds) {
      const Identifier id = prd.identify();
      int layer = 4 * (m_stgcIdHelper.multilayer(id) - 1) + (m_stgcIdHelper.gasGap(id) - 1);
      ATH_MSG_DEBUG("Sorting PRD into layer, layer: " << layer << " gas_gap: " << m_stgcIdHelper.gasGap(id)
                    << "multilayer: " << m_stgcIdHelper.multilayer(id));
      stgcPrdsPerLayer.at(layer).push_back(std::move(prd));
    }
    
    // sort prds by channel
    for (unsigned int i_layer = 0; i_layer < stgcPrdsPerLayer.size(); i_layer++) {
      std::sort(stgcPrdsPerLayer.at(i_layer).begin(), stgcPrdsPerLayer.at(i_layer).end(),
                [this](const sTgcPrepData& a, const sTgcPrepData& b) {
                  return m_stgcIdHelper.channel(a.identify()) < m_stgcIdHelper.channel(b.identify());
                });
    }  
    return stgcPrdsPerLayer;
}


//=============================================================================
std::vector<std::vector<Muon::sTgcPrepData>> Muon::STgcClusterBuilderCommon::findStripCluster(std::vector<Muon::sTgcPrepData>&& strips,
                                                                                              const int maxMissingStrip) const {
  std::vector<std::vector<Muon::sTgcPrepData>> clusters;

  if (strips.empty()) return clusters;

  // Get the Id of the first strip to verify all the strips are from the same layer
  const Identifier& firstStripId = strips.front().identify();

  for (Muon::sTgcPrepData& prd : strips) {
    // If no element has been added to the vector of clusters yet
    if (clusters.empty()) {
      clusters.emplace_back();
      clusters.back().push_back(std::move(prd));
      continue;
    }

    const Identifier prd_id = prd.identify();
    if (m_stgcIdHelper.multilayerID(firstStripId) != m_stgcIdHelper.multilayerID(prd_id) ||
        m_stgcIdHelper.gasGap(firstStripId) != m_stgcIdHelper.gasGap(prd_id) ||
        m_stgcIdHelper.channelType(firstStripId) != m_stgcIdHelper.channelType(prd_id)) {
      ATH_MSG_WARNING("Strips must be separated by layer before attempting to build clusters,"
                       << " but found strips from different chambers");
      continue;
    }

    // Add another strip to a  vector of clusters
    if (std::abs(m_stgcIdHelper.channel(prd.identify()) - 
                 m_stgcIdHelper.channel(clusters.back().back().identify())) > maxMissingStrip + 1) {
      clusters.emplace_back();
    }
    clusters.back().push_back(std::move(prd));   
  }

  return clusters;
}


//=============================================================================
std::optional<Muon::STgcClusterPosition> Muon::STgcClusterBuilderCommon::weightedAverage(
        const std::vector<Muon::sTgcPrepData>& cluster,
        const double resolution,
        bool isStrip) const
{

  if (cluster.empty()) {
    ATH_MSG_VERBOSE("Skipping empty cluster");
    return std::nullopt;
  }

  double weightedPosX{0.0};
  double maxCharge{-1.0};
  double sumWeight{0.0};
  double sigmaSq{0.0};
  Identifier clusterId;
  Amg::Vector3D clusDir{Amg::Vector3D::Zero()};

  ATH_MSG_DEBUG("Running weighted average method on a cluster with " << cluster.size() << " strips");
  for (const Muon::sTgcPrepData& prd : cluster) {
      // Skip channel if its charge is negative
      if (prd.charge() < 0) continue;

      double weight = isStrip ? prd.charge() : 1.0;
      ATH_MSG_DEBUG("isStrip: " << isStrip << " weight: " << weight);

      weightedPosX += prd.localPosition().x()*weight;
      sumWeight    += weight;
      sigmaSq      += weight*weight*resolution*resolution;
      clusDir += weight * NswClustering::toLocal(prd);
      ATH_MSG_DEBUG("Channel local position and charge: " << prd.localPosition().x() << " " << prd.charge() );

      // Set the cluster identifier to the max charge strip
      if (!isStrip) {
          clusterId = prd.identify();
      } else if (prd.charge() > maxCharge) {
          maxCharge = prd.charge();
          clusterId = prd.identify();
      }
  }

  // Charge in PRD is an integer, so the sum of weights should be greater than 1 unless the data is corrupted.
  // Skip corrupted cluster
  if (sumWeight < 1) {
    ATH_MSG_VERBOSE("Got unexpected sum of weights: " << sumWeight);
    return std::nullopt;
  }

  // Mean position of the cluster
  double reconstructedPosX = weightedPosX / sumWeight;

  NswErrorCalibData::Input errorCalibIn{};
  errorCalibIn.stripId = clusterId;
  errorCalibIn.clusterAuthor = static_cast<unsigned>(sTgcPrepData::Author::SimpleClusterBuilder);
  errorCalibIn.locPhi = clusDir.phi();
  errorCalibIn.locTheta = clusDir.theta();
  errorCalibIn.localPos = Amg::Vector2D{reconstructedPosX, 0};
  errorCalibIn.clusterSize = cluster.size();

  const double localUncertainty = m_errorCalibData.clusterUncertainty(errorCalibIn);

  ATH_MSG_DEBUG("Reconstructed a cluster using the weighted average,"
                 << " cluster Id: " << m_stgcIdHelper.print_to_string(clusterId)
                 << ", mean position = " << reconstructedPosX
                 << ", uncertainty = " << std::sqrt(sigmaSq));

  return std::make_optional<Muon::STgcClusterPosition>(clusterId, reconstructedPosX, localUncertainty*localUncertainty);
}


//=============================================================================
std::optional<Muon::STgcClusterPosition> Muon::STgcClusterBuilderCommon::caruanaGaussianFitting(
        const std::vector<sTgcPrepData>& cluster,
        const double positionResolution,
        const double angularResolution,
        const MuonGM::MuonDetectorManager* detManager) const
{

  // Verify that there are 3 or more strips with non-zero charge, and the shape is not similar to a staircase
  std::vector<int> charges;
  charges.reserve(cluster.size());
  std::vector<double> stripPositions;
  stripPositions.reserve(cluster.size());
  bool isStairUp{true}, isStairDown{true};
  int multiplicity = 0;
  for (unsigned int j = 0; j < cluster.size(); ++j){
    int stripCharge = cluster.at(j).charge();
    if (stripCharge > 0) {
      multiplicity += 1;
      charges.push_back(stripCharge);
      stripPositions.push_back(cluster.at(j).localPosition().x());

      if (j > 0) {
        int prevStripCharge = cluster.at(j-1).charge();
        if (isStairUp && (stripCharge - prevStripCharge < 0))
          isStairUp = false;
        if (isStairDown && (prevStripCharge > 0) && (stripCharge - prevStripCharge > 0))
          isStairDown = false;
      }
    }
  }

  // Caruana fitting method doesn't support clusters having least than 3 strips or staircase-like clusters
  if (multiplicity < 3 || isStairUp || isStairDown) {
    if (msgLvl(MSG::VERBOSE)) {
      std::stringstream sstr{};
      for (const Muon::sTgcPrepData& prd : cluster) {
          sstr << m_stgcIdHelper.print_to_string(prd.identify())
               << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
               << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
               << std::endl;
      }
      ATH_MSG_VERBOSE("Reject cluster incompatible with the Caruana method..." << std::endl << sstr.str());
    }
    return std::nullopt;
  }

  // Here we implement the Caruana method to reconstruct the position of the cluster
  AmgSymMatrix(3) elementPosMatrix;
  for (int i=0; i<3; i++) {
    for (int j=0; j<=i; j++) elementPosMatrix.fillSymmetric(i, j, 0);
  }
  Amg::Vector3D chargeVector(0., 0., 0.);

  // The reconstruction method becomes much simpiler when the strip positions are shifted such that 
  // the center of the cluster is at zero.
  float clusterCenter = 0;
  for (size_t i_strip = 0; i_strip < stripPositions.size(); i_strip++) {
    clusterCenter += stripPositions.at(i_strip);
  }
  clusterCenter = clusterCenter / stripPositions.size();

  std::vector<double> stripPositions_shifted = {};
  for (size_t i_strip = 0; i_strip < stripPositions.size(); i_strip++) {
    stripPositions_shifted.push_back(stripPositions.at(i_strip)-clusterCenter);
  }

  // Now build the matrix equation
  for (size_t i_element = 0; i_element < stripPositions_shifted.size(); i_element++) {
    elementPosMatrix.fillSymmetric(0, 0, elementPosMatrix(0,0) + 1);
    elementPosMatrix.fillSymmetric(0, 1, elementPosMatrix(0,1) + stripPositions_shifted.at(i_element));
    elementPosMatrix.fillSymmetric(0, 2, elementPosMatrix(0,2) + std::pow(stripPositions_shifted.at(i_element), 2));
    elementPosMatrix.fillSymmetric(1, 2, elementPosMatrix(1,2) + std::pow(stripPositions_shifted.at(i_element), 3));
    elementPosMatrix.fillSymmetric(2, 2, elementPosMatrix(2,2) + std::pow(stripPositions_shifted.at(i_element), 4));
    const double log_charge = std::log(charges.at(i_element));
    chargeVector(0) += log_charge;
    chargeVector(1) += stripPositions_shifted.at(i_element) * log_charge;
    chargeVector(2) += std::pow(stripPositions_shifted.at(i_element), 2) * log_charge;
  }
  elementPosMatrix(1,1) = elementPosMatrix(0,2);

  // If the matrix is singular then the reconstruction method will fail
  if (elementPosMatrix.determinant() == 0) {
    if (msgLvl(MSG::VERBOSE)) {
      std::stringstream sstr{};
      for (const Muon::sTgcPrepData& prd : cluster) {
          sstr << m_stgcIdHelper.print_to_string(prd.identify())
               << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
               << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
               << std::endl;
      }
      ATH_MSG_VERBOSE("Failed to compute the position of the cluster..." << std::endl << sstr.str());
    }
    return std::nullopt;
  }

  // Solve the matrix equation to find the Caruana parameters
  Amg::Vector3D caruanaPars = (elementPosMatrix.inverse()) * chargeVector;

  // The 3rd parameter is related to the standard deviation: sigma = sqrt(-1 / (2 * caruanaPars(2))).
  // Hence use the Caruana method if the std dev is small i.e. < 3*strip_pitch or caruanaPars(2) > -0.005,
  // or if std dev is not physical i.e. caruanaPars(2) >= 0
  double reconstructedPosX{0.};
  if (caruanaPars(2) < -0.005) {
    reconstructedPosX = clusterCenter - caruanaPars(1) / (2*caruanaPars(2));
  } else { 
    if (msgLvl(MSG::VERBOSE)) {
      std::stringstream sstr{};
      for (const Muon::sTgcPrepData& prd : cluster) {
          sstr << m_stgcIdHelper.print_to_string(prd.identify())
               << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
               << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
               << std::endl;
      }
      ATH_MSG_VERBOSE("Reject cluster with large standard deviation..." << std::endl << sstr.str());
    }
    return std::nullopt;
  }

  // Find the channel that the cluster position reconstructs on top of and set the cluster id to its id
  double minCenterDistance = 9999.99;
  int channelIndex = 0;
  for (size_t i_elem = 0; i_elem < cluster.size(); i_elem++) {
    double strip_localPos = cluster.at(i_elem).localPosition().x();
    if (minCenterDistance > abs(strip_localPos - reconstructedPosX)) {
      minCenterDistance = abs(strip_localPos - reconstructedPosX);
      channelIndex = i_elem;
    }
  }
  const Identifier clusterId = cluster.at(channelIndex).identify();

  // Get the sTGC readoutElement and the strip pitch
  const MuonGM::sTgcReadoutElement* detEl = detManager->getsTgcReadoutElement(clusterId);
  const double stripPitch = detEl->channelPitch(clusterId);

  // If the reconstructed cluster position is far, such as one strip pitch, from any strip position in the cluster,
  // then the Caruana reconstruction method isn't a good method to use.
  if (minCenterDistance > stripPitch) {
    if (msgLvl(MSG::VERBOSE)) {
      std::stringstream sstr{};
      for (const Muon::sTgcPrepData& prd : cluster) {
          sstr << m_stgcIdHelper.print_to_string(prd.identify())
               << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
               << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
               << std::endl;
      }
      ATH_MSG_VERBOSE("Reject cluster since its position is more than " << stripPitch << " mm too far from the nearest strip..." 
                      << std::endl << sstr.str());
    }
    return std::nullopt;
  }

  // We denote caruanaPars = (a, b, c) and find the error on the b and c components
  double gamma0 = 0;
  double gamma2 = 0;
  double gamma4 = 0;
  for (size_t i_strip = 0; i_strip < stripPositions_shifted.size(); i_strip++) {
    gamma0 += 1;
    gamma2 += std::pow(stripPositions_shifted.at(i_strip), 2);
    gamma4 += std::pow(stripPositions_shifted.at(i_strip), 4);
  }

  // We also need the tan(theta) of the cluster
  Amg::Vector3D globPos(0., 0., 0.);
  detEl->stripGlobalPosition(clusterId, globPos);
  double tan_theta = std::hypot(globPos.x(),  globPos.y()) / globPos.z();
  double spreadFactor = std::hypot(positionResolution, angularResolution * tan_theta);

  double sigma_b = spreadFactor / std::sqrt(gamma2);
  double sigma_c = spreadFactor * std::sqrt(gamma0 / (gamma0 * gamma4 - gamma2 * gamma2));

  // Now propagate the Uncertainty to find the uncertainty on the mean
  double sigmaSq = std::pow((1 / (2 * caruanaPars(2))) * sigma_b, 2) +
                   std::pow((caruanaPars(1) / (2 * caruanaPars(2) * caruanaPars(2))) * sigma_c, 2);

  // Caruana method fails, if the uncertainty on the mean position is too large such as greater than 2mm
  if (sigmaSq > 4) {
    if (msgLvl(MSG::VERBOSE)) {
      std::stringstream sstr{};
      for (const Muon::sTgcPrepData& prd : cluster) {
          sstr << m_stgcIdHelper.print_to_string(prd.identify())
               << ", local pos: ("<< prd.localPosition().x() << "," << prd.localPosition().y()
               << "), charge: " << prd.charge() << ", time: " << static_cast<int>(prd.time())
               << std::endl;
      }
      ATH_MSG_VERBOSE("Reject cluster with large error on the mean position..." << std::endl << sstr.str());
    }
    return std::nullopt;
  }
  
  
  Amg::Vector3D globalClusterPos {detEl->surface(clusterId).transform()*Amg::Vector3D{reconstructedPosX, 0,0}}; 
  Amg::Vector3D clusDir{ NswClustering::toLocal(detEl->surface(clusterId), globalClusterPos) };
  
  NswErrorCalibData::Input errorCalibIn{};
  errorCalibIn.stripId = clusterId;
  errorCalibIn.clusterAuthor = static_cast<unsigned>(sTgcPrepData::Author::Caruana);
  errorCalibIn.clusterError = std::sqrt(sigmaSq); 
  errorCalibIn.locPhi = clusDir.phi();
  errorCalibIn.locTheta = clusDir.theta();
  errorCalibIn.localPos = Amg::Vector2D{reconstructedPosX, 0};
  errorCalibIn.clusterSize = cluster.size();

  const double localUncertainty = m_errorCalibData.clusterUncertainty(errorCalibIn);

  ATH_MSG_DEBUG("Reconstructed a cluster using the Caruana method,"
                 << " cluster Id: " << m_stgcIdHelper.print_to_string(clusterId)
                 << ", mean position = " << reconstructedPosX
                 << ", uncertainty = " << std::sqrt(sigmaSq));

  return std::make_optional<Muon::STgcClusterPosition>(clusterId, reconstructedPosX, localUncertainty*localUncertainty);
}
