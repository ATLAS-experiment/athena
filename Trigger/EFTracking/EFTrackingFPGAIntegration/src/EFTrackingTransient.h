/*
    Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

/**
 * @file src/EFTrackingTransient.h
 * @author zhaoyuan.cui@cern.ch
 * @author yuan-tang.chou@cern.ch
 * @date Apr. 22, 2024
 * @brief Temporary data struct design for the EF tracking FPGA integration
 * development
 */

#ifndef EFTRACKING_FPGA_INTEGRATION_EFTRACKING_TRANSIENT_H
#define EFTRACKING_FPGA_INTEGRATION_EFTRACKING_TRANSIENT_H

#include <vector>

#include "xAODMeasurementBase/UncalibratedMeasurementContainer.h"

namespace EFTrackingTransient {
// The struct of the StripCluster and PixelCluster are not aligned at the moment
// They might be aligned in the future for efficient device memory usage

/**
 * @brief The StripClusters struct contains all xAOD::StripCluster data members
 *
 * This struct is only used as the input to the xAODTransfer/pass-through kernel
 * The outputs of the xAODTransfer kernel are struct of arrays, see below
 */
struct StripCluster {
  float localPosition = 0.0f;
  float localCovariance = 0.0f;
  unsigned int idHash = 0;
  long unsigned int id = 0;
  float globalPosition[3] = {0.0f, 0.0f, 0.0f};
  unsigned long long rdoList[1000] = {0};
  int channelsInPhi = 0;
  int sizeOfRDOList = 0;
};

/**
 * @brief The PixelClusters struct contains all xAOD::PixelCluster data members
 *
 * This struct is only used as the input to the xAODTransfer/pass-through kernel
 * The outputs of the xAODTransfer kernel are struct of arrays, see below
 */
struct PixelCluster {
  long unsigned int id = 0;
  unsigned int idHash = 0;
  float localPosition[2] = {0.0f, 0.0f};
  float localCovariance[2] = {0.0f, 0.0f};
  float globalPosition[3] = {0.0f, 0.0f, 0.0f};
  unsigned long long rdoList[1000] = {0};
  int channelsInPhi = 0;
  int channelsInEta = 0;
  float widthInEta = 0.0f;
  float omegaX = 0.0f;
  float omegaY = 0.0f;
  int totList[1000] = {0};
  int totalToT = 0;
  float chargeList[1000] = {0.0f};
  float totalCharge = 0.0f;
  float energyLoss = 0.0f;
  char isSplit = 0;
  float splitProbability1 = 0.0f;
  float splitProbability2 = 0.0f;
  int lvl1a = 0;
  int sizeOfRDOList = 0;
  int sizeOfTotList = 0;
  int sizeOfChargeList = 0;
};

/**
 * @brief The SpacePoint struct contains all xAOD::SpacePoint data members
 *
 * This struct is only used as the input to the xAODTransfer/pass-through kernel
 * The outputs of the xAODTransfer kernel are struct of arrays, see below
 */
struct SpacePoint {

  unsigned int idHash[2] = {0};
  float globalPosition[3] = {0.0f, 0.0f, 0.0f};
  float radius = 0.0f;
  float cov_r = 0.0f;
  float cov_z = 0.0f;
  int measurementIndex[100] = {0};
  // Strip
  float topHalfStripLength = 0.0f;
  float bottomHalfStripLength = 0.0f;
  float topStripDirection[3] = {0.0f, 0.0f, 0.0f};
  float bottomStripDirection[3] = {0.0f, 0.0f, 0.0f};
  float stripCenterDistance[3] = {0.0f, 0.0f, 0.0f};
  float topStripCenter[3] = {0.0f, 0.0f, 0.0f};
};

/**
 * @brief The structure of the Metadata containing data after clusterization
 *
 * These data are required to create xAOD container from the FPGA output
 */
struct Metadata {
  unsigned long numOfStripClusters = 0;
  unsigned long numOfPixelClusters = 0;
  unsigned long numOfStripSpacePoints = 0;
  unsigned int numOfPixelSpacePoints = 0;
  unsigned int scRdoIndex[500000] = {0};
  unsigned int pcRdoIndex[500000] = {0};
  unsigned int pcTotIndex[500000] = {0};
  unsigned int pcChargeIndex[500000] = {0};
  unsigned int pcRdoIndexSize = 0;
  unsigned int scRdoIndexSize = 0;
  unsigned int pcTotIndexSize = 0;
  unsigned int pcChargeIndexSize = 0;
};

/**
 * @brief The StripClusters struct contains the output arrays from the FPGA

*/
struct StripClusterOutput {

  float *scLocalPosition;
  float *scLocalCovariance;
  unsigned int *scIdHash;
  long unsigned int *scId;
  float *scGlobalPosition;
  unsigned long long *scRdoList;
  int *scChannelsInPhi;
};

/**
 * @brief The PixelClusters struct contains the output arrays from the FPGA

*/
struct PixelClusterOutput {

  float *pcLocalPosition;
  float *pcLocalCovariance;
  unsigned int *pcIdHash;
  long unsigned int *pcId;
  float *pcGlobalPosition;
  unsigned long long *pcRdoList;
  int *pcChannelsInPhi;
  int *pcChannelsInEta;
  float *pcWidthInEta;
  float *pcOmegaX;
  float *pcOmegaY;
  int *pcTotList;
  int *pcTotalToT;
  float *pcChargeList;
  float *pcTotalCharge;
  float *pcEnergyLoss;
  char *pcIsSplit;
  float *pcSplitProbability1;
  float *pcSplitProbability2;
  int *pcLvl1a;
};

/**
 * @brief The Pixel/Strip SpacePoints struct contains the output arrays from the
 * FPGA
 *
 */

struct SpacePointOutput {

  unsigned int *spIdHash;
  float *spGlobalPosition;
  float *spRadius;
  float *spVarianceR;
  float *spVarianceZ;
  int *spMeasurementIndexes;
  float *spTopHalfStripLength;
  float *spBottomHalfStripLength;
  float *spTopStripDirection;
  float *spBottomStripDirection;
  float *spStripCenterDistance;
  float *spTopStripCenter;
};

/**
 * @brief The StripClusterAuxInput struct is used to simplify the creaction of
 * the xAOD::StripClusterContainer
 */
struct StripClusterAuxInput {
  std::vector<float> localPosition;
  std::vector<float> localCovariance;
  std::vector<unsigned int> idHash;
  std::vector<long unsigned int> id;
  std::vector<float> globalPosition;
  std::vector<unsigned long long> rdoList;
  std::vector<int> channelsInPhi;
};

/**
 * @brief The PixelClusterAuxInput struct is used to simplify the creaction of
 * the xAOD::PixelClusterContainer
 */
struct PixelClusterAuxInput {
  std::vector<long unsigned int> id;
  std::vector<unsigned int> idHash;
  std::vector<float> localPosition;
  std::vector<float> localCovariance;
  std::vector<float> globalPosition;
  std::vector<unsigned long long> rdoList;
  std::vector<int> channelsInPhi;
  std::vector<int> channelsInEta;
  std::vector<float> widthInEta;
  std::vector<float> omegaX;
  std::vector<float> omegaY;
  std::vector<int> totList;
  std::vector<int> totalToT;
  std::vector<float> chargeList;
  std::vector<float> totalCharge;
  std::vector<float> energyLoss;
  std::vector<char> isSplit;
  std::vector<float> splitProbability1;
  std::vector<float> splitProbability2;
  std::vector<int> lvl1a;
  std::vector<int> sizeOfRDOList;
  std::vector<int> sizeOfTotList;
  std::vector<int> sizeOfChargeList;
};
/**
 * @brief The SpacePointAuxInput struct is used to simplify the creaction of the
 * xAOD::SpacePointContainer
 */
struct SpacePointAuxInput {
  std::vector<unsigned int> elementIdList;
  std::vector<float> globalPosition;
  std::vector<float> radius;
  std::vector<float> varianceR;
  std::vector<float> varianceZ;
  std::vector<float> topHalfStripLength;
  std::vector<float> bottomHalfStripLength;
  std::vector<float> topStripDirection;
  std::vector<float> bottomStripDirection;
  std::vector<float> stripCenterDistance;
  std::vector<float> topStripCenter;
  std::vector<int> measurementIndexes;
};

}  // namespace EFTrackingDataTransient

#endif  // EFTRACKING_FPGA_INTEGRATION_EFTRACKING_TRANSIENT_H