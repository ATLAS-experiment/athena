/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#ifndef INDETALIGNGENTOOLS_INDETALIGNFILLSICLUSTER_H
#define INDETALIGNGENTOOLS_INDETALIGNFILLSICLUSTER_H
// ================================================
// InDetAlignFillSiCluster
// ================================================
//
// InDetAlignFillSiCluster.h
// Headerfile for InDetAlignFillSiCluster
//
// Carlos Escobar, started 08/03/2008
//
// AthAlgTool to create store Silicon Cluster information in a ntuple

#include <string>

#include "AthenaBaseComps/AthAlgTool.h"

#include "InDetPrepRawData/PixelClusterContainer.h"
#include "InDetPrepRawData/SCT_ClusterContainer.h"

#include "InDetReadoutGeometry/SiDetectorElementCollection.h"
#include "StoreGate/ReadCondHandleKey.h"

#include "IInDetAlignFillSiCluster.h"
#include "GaudiKernel/INTupleSvc.h"

// Forward declaration
class PixelID;
class SCT_ID;

class InDetAlignFillSiCluster: public extends<AthAlgTool, IInDetAlignFillSiCluster> {

  typedef InDet::PixelClusterContainer PixelClusterContainer;
  typedef InDet::SCT_ClusterContainer SCT_ClusterContainer;

 public:
  using base_class::base_class;
  virtual ~InDetAlignFillSiCluster() = default;
  
  virtual StatusCode initialize() override;
  virtual StatusCode finalize() override;
  
  virtual StatusCode FillSiCluster() override;

 private:
  ServiceHandle<INTupleSvc>  m_ntupleSvc{this,"NTupleSvc","NTupleSvc"};

  const PixelID*                        m_pixelid{nullptr};           //!< Pixel ID helper  
  const SCT_ID*                         m_sctID{nullptr};           //!< SCT ID helper

  Gaudi::Property<std::string> m_Pixel_SiClustersName{this, "Pixel_SiClusterContainerName", "PixelClusters"};
  Gaudi::Property<std::string> m_Sct_SiClustersName{this, "SCT_SiClusterContainerName", "SCT_Clusters"};

  const InDet::PixelClusterContainer*          m_Pixel_clcontainer{nullptr};
  const InDet::SCT_ClusterContainer*           m_Sct_clcontainer{nullptr};

  // For P->T converter of PixelClusters
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_pixelDetEleCollKey{this, "PixelDetEleCollKey", "PixelDetectorElementCollection", "Key of SiDetectorElementCollection for Pixel"};
  // For P->T converter of SCT_Clusters
  SG::ReadCondHandleKey<InDetDD::SiDetectorElementCollection> m_SCTDetEleCollKey{this, "SCTDetEleCollKey", "SCT_DetectorElementCollection", "Key of SiDetectorElementCollection for SCT"};
  
  // methods
  void bookNtuple();
  StatusCode RetrieveSCTSiClusters();
  StatusCode RetrievePixelSiClusters();
  void FillSCTSiNtuple();
  void FillPixelSiNtuple();

  // variables
  Gaudi::Property<std::string> m_ntupleName{this, "NtupleName", "/NTUPLES/GLOBFILE"};    //!< ntuple name

  NTuple::Item<long> m_pixel_nclusters;   //!< number of Pixel Clusters
  NTuple::Item<long> m_sct_nclusters;   //!< number of SCT Clusters
  
  // ----------------------------------------------------------------------
  // Pixel Clusters
  NTuple::Array<float> m_pixel_clx;       //!< Pixel Cluster X
  NTuple::Array<float> m_pixel_cly;       //!< Pixel Cluster Y
  NTuple::Array<float> m_pixel_clz;       //!< Pixel Cluster Z
  NTuple::Array<long>  m_pixel_groupsize; //!< Pixel Cluster Group Size
  NTuple::Array<long>  m_pixel_layer;     //!< Pixel Cluster layer
  NTuple::Array<long>  m_pixel_phi;       //!< Pixel Cluster phi
  NTuple::Array<float> m_pixel_LocX;      //!< Pixel Cluster Local X
  NTuple::Array<float> m_pixel_LocY;      //!< Pixel Cluster Local Y
  NTuple::Matrix<long> m_pixel_clrow;     // list of pixel row
  NTuple::Matrix<long> m_pixel_clcol;     // list of pixel col
  // ----------------------------------------------------------------------

  // ----------------------------------------------------------------------
  // SCT Clusters
  NTuple::Array<float> m_sct_clx;       //!< SCT Cluster X
  NTuple::Array<float> m_sct_cly;       //!< SCT Cluster Y
  NTuple::Array<float> m_sct_clz;       //!< SCT Cluster Z
  NTuple::Array<long>  m_sct_groupsize; //!< SCT Cluster Group Size
  NTuple::Array<long>  m_sct_layer;     //!< SCT Cluster layer
  NTuple::Array<long>  m_sct_eta;       //!< SCT Cluster eta
  NTuple::Array<long>  m_sct_phi;       //!< SCT Cluster phi
  NTuple::Array<long>  m_sct_side;      //!< SCT Cluster side
  // ----------------------------------------------------------------------

};

#endif // INDETALIGNGENTOOLS_INDETALIGNFILLSICLUSTER_H
