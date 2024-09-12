/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// MergedPixelsAlg.h
// SimplePixelClusteringTool plus implementation of merging clusters which
// contain a common RDO Identifier  
// Input from RDOs  
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////

#ifndef SICLUSTERIZATIONTOOL_MERGEDPIXELSTOOL_H
#define SICLUSTERIZATIONTOOL_MERGEDPIXELSTOOL_H

#include "SiClusterizationTool/PixelRDOTool.h"

#include "Identifier/Identifier.h"
// forward declare not possible (typedef)
#include "InDetPrepRawData/PixelClusterCollection.h"
#include "InDetPrepRawData/PixelGangedClusterAmbiguities.h"
#include "InDetRawData/InDetRawDataCollection.h"
#include "InDetRawData/PixelRDORawData.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "PixelConditionsData/PixelChargeCalibCondData.h"
#include "PixelConditionsData/PixelOfflineCalibData.h"

#include "GaudiKernel/ServiceHandle.h"

#include <atomic>
#include <vector>
#include <array>

class PixelID;

namespace InDetDD {
  class SiDetectorElement;
}


namespace InDet {
  
  struct network {
  public:
    int               NC{};
    std::array<int,8> CON{};
  };
  
  const auto pixel_less = [] (UnpackedPixelRDO const&  id1,UnpackedPixelRDO const& id2) -> bool {
    if(id1.COL == id2.COL) return id1.ROW < id2.ROW;
    return id1.COL < id2.COL;
  };
 
  class MergedPixelsTool final: public extends<AthAlgTool, IPixelClusteringTool> {
  public:


    // Constructor with parameters:
    MergedPixelsTool(const std::string& type,
                     const std::string& name,
                     const IInterface* parent);

    virtual ~MergedPixelsTool() = default;

    // Called by the PixelPrepRawDataFormation algorithm once for every pixel 
    // module (with non-empty RDO collection...). 
    // It clusters together the RDOs with a pixell cell side in common.
    // [Implementation of the IPixelClusteringTool interface]
    virtual PixelClusterCollection* clusterize(
        const InDetRawDataCollection<PixelRDORawData>& RDOs,
        const PixelID& pixelID, 
        DataPool<PixelCluster>* dataItemsPool,
        const EventContext& ctx) const override;

    // Once the lists of RDOs which makes up the clusters have been found by the
    // clusterize() method, this method is called for each of these lists.
    // The method computes the local position of the cluster, and create 
    // a "cluster object" with all the required information (including charge 
    // interpolation variables Omegax and Omegay, and whether the cluster 
    // contains ganged pixels) 
    // This method calls the ClusterMakerTool to compute global position and 
    // position errors.
    // Input parameters are the list of RDOs identifier of the would-be 
    // cluster, the list of TOT values, the module the cluster belongs to,
    // the pixel helper tool and the number of RDOS of the cluster.
    PixelCluster makeCluster(
        const std::vector<Identifier>& group, 
        const std::vector<int>& totgroup,
        const std::vector<int>& lvl1group,
        const InDetDD::SiDetectorElement* element, 
        const PixelID& pixelID,
        int& clusterNumber, 
        bool split, 
        double splitProb1, 
        double splitProb2,
        const PixelChargeCalibCondData* calibData,
        const PixelCalib::PixelOfflineCalibData* offlineCalibData,
        const EventContext& ctx) const;

    ///Retrieve the necessary services in initialize                
    virtual StatusCode initialize() override;
        
    ///Statistics output                
    virtual StatusCode finalize() override;


  private:
    MergedPixelsTool();
    MergedPixelsTool(const MergedPixelsTool&);
    MergedPixelsTool &operator=(const MergedPixelsTool&);

    void addClusterNumber(const int& r, 
                          const int& Ncluster,
                          const std::vector<network>& connections,    
                          std::vector<UnpackedPixelRDO>& collectionID) const;

    BooleanProperty m_addCorners{this, "AddCorners", true};

    ToolHandle<ClusterMakerTool> m_clusterMaker {this, "globalPosAlg", "InDet::ClusterMakerTool"};
    ToolHandle<PixelRDOTool> m_pixelRDOTool {this, "PixelRDOTool", "InDet::PixelRDOTool"};

    SG::ReadCondHandleKey<PixelChargeCalibCondData> m_chargeDataKey {this, "PixelChargeCalibCondData", "PixelChargeCalibCondData", "Pixel charge calibration data"};
    SG::ReadCondHandleKey<PixelCalib::PixelOfflineCalibData> m_clusterErrorKey{this, "PixelOfflineCalibData", "PixelOfflineCalibData", "Output key of pixel cluster"};


    IntegerProperty m_posStrategy{this, "posStrategy", 0};
    IntegerProperty m_errorStrategy{this, "errorStrategy", 1};

    mutable std::atomic_uint m_processedClusters{0};    //!< statistics output
    mutable std::atomic_bool m_printw{true};
  };

}

#endif // SICLUSTERIZATIONTOOL_MERGEDPIXELSALG_H
