/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "PixelClusteringTool.h"

#include <xAODInDetMeasurement/PixelCluster.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include <xAODInDetMeasurement/PixelClusterAuxContainer.h>
#include <InDetPrepRawData/SiWidth.h>
#include <TrkSurfaces/Surface.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

using CLHEP::micrometer;

namespace {
   inline bool isFEI3(const InDetDD::SiDetectorElement& element) {
      const InDetDD::PixelModuleDesign& design =
         static_cast<const InDetDD::PixelModuleDesign&>(element.design());
      return  design.getReadoutTechnology() == InDetDD::PixelReadoutTechnology::FEI3;
   }

   inline bool
   isGanged(Identifier rdoID,
            const InDetDD::SiDetectorElement& element)
   {
      InDetDD::SiCellId cellID = element.cellIdFromIdentifier( rdoID );
      return  ( element.numberOfConnectedCells( cellID ) > 1 );
   }

   inline std::optional<std::array<std::int16_t,2> >
   getGangedCoordinates(const std::array<std::int16_t,2> &coordinates,
                        const InDetDD::SiDetectorElement& element)
   {
      // If the pixel is ganged, returns a new identifier for it
      InDetDD::SiCellId cellID(coordinates[0],coordinates[1]);
      if ( element.numberOfConnectedCells( cellID ) > 1 ) {
         InDetDD::SiCellId gangedCellID = element.connectedCell( cellID, 1 );
         return std::array<std::int16_t,2>{ static_cast<std::int16_t>(gangedCellID.phiIndex()),
                                            static_cast<std::int16_t>(gangedCellID.etaIndex())};
      }
      return std::nullopt;
   }
   template <typename T1, typename T2>
   T1 check_integer_cast(T2 a) {
      assert( std::in_range<T1>(a) );
      return static_cast<T1>(a);
   }
}

namespace ActsTrk {

StatusCode PixelClusteringTool::initialize()
{
  ATH_MSG_DEBUG("Initializing " << name() << " ...");

  ATH_MSG_DEBUG("   " << m_addCorners );
  ATH_MSG_DEBUG("   " << m_useWeightedPos );
  ATH_MSG_DEBUG("   " << m_broadErrors );
  ATH_MSG_DEBUG("   " << m_checkGanged );
    
  ATH_CHECK(m_pixelLorentzAngleTool.retrieve());

  ATH_CHECK(m_chargeDataKey.initialize(not m_chargeDataKey.empty()));

  ATH_MSG_INFO("   Charge Data Key:" << m_chargeDataKey);
  ATH_MSG_INFO("   ID Helper Name:" << m_idHelperName);

  ATH_CHECK( detStore()->retrieve(m_pixelID, m_idHelperName) );
  
  ATH_MSG_DEBUG(name() << " successfully initialized");
  return StatusCode::SUCCESS;
}

PixelClusteringTool::PixelClusteringTool(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{}

template <bool GANGED>
std::pair<unsigned int, unsigned int>
PixelClusteringTool::countCellsImpl(const RDOContainer& rdoContainer,
                                    const std::vector<IdentifierHash> &listOfIds,
                                    const InDetDD::SiDetectorElementCollection &detector_elements) const {
   auto getNHits =[](const InDetRawDataCollection<PixelRDORawData> &RDOs,
                     const InDetDD::SiDetectorElementCollection &detector_elements )
      -> unsigned int
   {
      unsigned int n_hits = RDOs.size();
      if constexpr(GANGED) {
         const InDetDD::SiDetectorElement *element = detector_elements.at(RDOs.identifyHash());
         assert(element);
         if (isFEI3(*element)) {
            for(const PixelRDORawData* rdo : RDOs) {
               if (isGanged(rdo->identify(), *element)) {
                  ++n_hits;
               }
            }
         }
      }
      return n_hits;
   };
   unsigned int n_hits=0u;
   if (listOfIds.empty()) {
      for (const InDetRawDataCollection<PixelRDORawData> *RDOs : rdoContainer) {
         assert( RDOs);
         n_hits += getNHits(*RDOs, detector_elements);
      }
   }
   else {
      for (const IdentifierHash& id : listOfIds) {
         if (not id.is_valid()) continue;
         const InDetRawDataCollection<PixelRDORawData> *RDOs = rdoContainer.indexFindPtr(id);
         if (RDOs) {
            n_hits += getNHits(*RDOs, detector_elements);
         }
      }
   }
   // the total number of hits is the best estimate of the maximum number of clusters.
   // @TODO could apply a factor for the number clusters or only if the number of hits
   //       are above a certain threshold to reduce memory consumption though impact
   //       is likely small.
   return {n_hits,n_hits};
}

std::pair<unsigned int, unsigned int>
PixelClusteringTool::countCells(const RDOContainer& rdo_collection,
                                const std::vector<IdentifierHash> &listOfIds,
                                const InDetDD::SiDetectorElementCollection &detector_elements) const {
   if (m_isITk || !m_checkGanged ) {
      return countCellsImpl<false>(rdo_collection,listOfIds, detector_elements);
   }
   else {
      return countCellsImpl<true>(rdo_collection,listOfIds, detector_elements);
   }
}


StatusCode
PixelClusteringTool::makeCluster(size_t icluster,
                                 const PixelClusteringTool::ClusterProxy &cluster,
				 const InDetDD::SiDetectorElement& element,
				 const InDetDD::PixelModuleDesign& design,
                                 const InDetRawDataCollection<PixelRDORawData> &rdos,
				 const PixelChargeCalibCondData *calibData,
				 const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
				 const double lorentzShift,
                                 xAOD::PixelCluster::ClusterVars& clusterVars) const
{ 

  Amg::Vector2D pos_acc(0,0);
  float tot_acc = 0.f;

  InDetDD::PixelDiodeTree::CellIndexType rowmax = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::min();
  InDetDD::PixelDiodeTree::CellIndexType colmax = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::min();
  InDetDD::PixelDiodeTree::CellIndexType rowmin = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::max();
  InDetDD::PixelDiodeTree::CellIndexType colmin = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::max();
  InDetDD::PixelDiodeTree::DiodeProxyWithPosition colmin_diode{};
  InDetDD::PixelDiodeTree::DiodeProxyWithPosition colmax_diode{};
  InDetDD::PixelDiodeTree::DiodeProxyWithPosition rowmin_diode{};
  InDetDD::PixelDiodeTree::DiodeProxyWithPosition rowmax_diode{};

  // We temporary comment this since it is not used
  // bool hasGanged = false;
  unsigned int n_rdos = clusterVars.rdoList.getBeginIndex(icluster);
  assert( clusterVars.totList.getBeginIndex(icluster)==n_rdos );
  assert( calibData==nullptr || clusterVars.chargeList.getBeginIndex(icluster)==n_rdos );

  IdentifierHash idHash = element.identifyHash();
  assert( idHash == cluster.identifyHash());
  Identifier module_id = element.identify();
  std::optional<Identifier::value_type> first_rdo_id;
  int cluster_lvl1min = std::numeric_limits<int>::max();

  using CellProxy = InPlaceClusterization::CellProxy<const IPixelClusteringTool::CellContainer>;
  for (CellProxy cellProxy : cluster) {

    //Construct the identifier class
    Identifier rdo_id = m_pixelID->pixel_id(module_id, cellProxy.coordinates()[0], cellProxy.coordinates()[1]);
    if (!first_rdo_id.has_value()) {
       first_rdo_id=rdo_id.get_compact();
    }

    // We temporary comment this since it is not used
    // TODO: Check how the ganged info is used in legacy
    // if (multiChip)  {
    //   hasGanged = hasGanged ||
    // 	m_pixelRDOTool->isGanged(id, element).has_value();
    // }
    assert(cellProxy.srcIndex() < rdos.size());
    const PixelRDORawData *rdo = rdos[cellProxy.srcIndex()];
    cluster_lvl1min = std::min(cluster_lvl1min, rdo->getLVL1A());
    const int tot = rdo->getToT();
    float charge = tot;

    std::array<InDetDD::PixelDiodeTree::CellIndexType,2> diode_idx
       = InDetDD::PixelDiodeTree::makeCellIndex(cellProxy.coordinates()[0],
                                                cellProxy.coordinates()[1]);
    InDetDD::PixelDiodeTree::DiodeProxyWithPosition si_param ( design.diodeProxyFromIdxCachePosition(diode_idx));

    if (calibData) {
      // Retrieving the calibration only depends on FE and not per cell (can be further optimized)
      // Single FE modules could have an optimized getCharge function where the calib constants are cached
      std::uint32_t feValue = design.getFE(si_param);
      auto diode_type = design.getDiodeType(si_param);
      if (m_isITk){
        // @TODO only check in makeClusters or check at all ?
        if (design.getReadoutTechnology() != InDetDD::PixelReadoutTechnology::RD53) {
           ATH_MSG_ERROR("Chip type is not recognized!");
           return StatusCode::FAILURE;
        }

        charge = calibData->getCharge(diode_type,
              calibStrategy,
              idHash,
              feValue,
              tot);
      } else {
        charge = calibData->getCharge(diode_type,
                                      idHash,
                                      feValue,
                                      tot);

        // These numbers are taken from the Cluster Maker Tool
        if (design.getReadoutTechnology() != InDetDD::PixelReadoutTechnology::RD53 && (idHash < 12 or idHash > 2035)) {
          charge = tot/8.0*(8000.0-1200.0)+1200.0;
        }
      }
      clusterVars.chargeList.setValue(n_rdos,charge);
    }
    clusterVars.rdoList.setValue(n_rdos,rdo_id.get_compact());
    clusterVars.totList.setValue(n_rdos,tot);
    ++n_rdos;
    
    const InDetDD::PixelDiodeTree::CellIndexType &row = diode_idx[0];
    const InDetDD::PixelDiodeTree::CellIndexType &col = diode_idx[1];
    if (row>rowmax) {
       rowmax=row;
       rowmax_diode = si_param;
    }
    if (row<rowmin) {
       rowmin=row;
       rowmin_diode = si_param;
    }
    if (col>colmax) {
       colmax=col;
       colmax_diode = si_param;
    }
    if (col<colmin) {
       colmin=col;
       colmin_diode = si_param;
    }

    // We compute the digital position as a sum of all RDO positions
    // all with the same weight of 1
    // We do not compute a charge-weighted center of gravity here (by default) since
    // we observe it to be worse than the digital position
    // ToT-weighted center of gravity must not be used
    if (m_useWeightedPos) {
      pos_acc += charge * si_param.position();
      tot_acc += charge;
    } else {
      pos_acc += si_param.position();
      tot_acc += 1;
    }
    
  }
  assert(n_rdos>0); // clusters must not be empty
  if (tot_acc > 0)
    pos_acc /= tot_acc;

  
  const int colWidth = colmax - colmin + 1;
  const int rowWidth = rowmax - rowmin + 1;

  double etaWidth = colmax_diode.xEtaMax() - colmin_diode.xEtaMin(); // design.widthFromColumnRange(colmin, colmax);
  double phiWidth = rowmax_diode.xPhiMax() - rowmin_diode.xPhiMin(); // design.widthFromColumnRange(colmin, colmax);

  // ask for Lorentz correction, get global position
  const Amg::Vector2D localPos = pos_acc;
  Amg::Vector2D locpos(localPos[Trk::locX]+lorentzShift, localPos[Trk::locY]);
  // find global position of element
  const Amg::Transform3D& T = element.surface().transform();
  double Ax[3] = {T(0,0),T(1,0),T(2,0)};
  double Ay[3] = {T(0,1),T(1,1),T(2,1)};
  double R [3] = {T(0,3),T(1,3),T(2,3)};
  
  const Amg::Vector2D&    M = locpos;
  Amg::Vector3D globalPos(M[0]*Ax[0]+M[1]*Ay[0]+R[0],M[0]*Ax[1]+M[1]*Ay[1]+R[1],M[0]*Ax[2]+M[1]*Ay[2]+R[2]);

  // Compute error matrix
  float width0, width1;
  if (m_broadErrors) {
      // Use cluster width
      width0 = phiWidth;
      width1 = etaWidth;
  } else {
      // Use average pixel width
      width0 = phiWidth / rowWidth;
      width1 = etaWidth / colWidth;
  }

  // Actually create the cluster (i.e. fill the values)
  
  Eigen::Matrix<float,2,1> localPosition(locpos.x(), locpos.y());
  Eigen::Matrix<float,2,2> localCovariance = Eigen::Matrix<float,2,2>::Zero();
  localCovariance(0, 0) = width0 * width0 / 12.0f; 
  localCovariance(1, 1) = width1 * width1 / 12.0f;
  
  clusterVars.identifierHash[icluster] = idHash;
  xAOD::VectorMap<2>(clusterVars.localPositionDim2[icluster].data()) = localPosition;
  xAOD::MatrixMap<2>(clusterVars.localCovarianceDim2[icluster].data()) = localCovariance;
  assert( first_rdo_id.has_value());
  clusterVars.identifier[icluster] = *first_rdo_id;
  clusterVars.rdoList.updateEndIndex(icluster,n_rdos);
  xAOD::VectorMap<3>(clusterVars.globalPosition[icluster].data()) = globalPos.cast<float>();
  clusterVars.totList.updateEndIndex(icluster,n_rdos);
  clusterVars.chargeList.updateEndIndex(icluster, (calibData ? n_rdos : 0u));
  clusterVars.lvl1a[icluster] = cluster_lvl1min;
  clusterVars.channelsInPhi[icluster] = rowWidth;
  clusterVars.channelsInEta[icluster] = colWidth;
  clusterVars.widthInEta[icluster] = etaWidth;
    
  return StatusCode::SUCCESS;
}

StatusCode
PixelClusteringTool::clusterize([[maybe_unused]] const EventContext& ctx,
                                const RawDataCollection& RDOs,
                                const InDet::SiDetectorElementStatus& pixelDetElStatus,
                                const InDetDD::SiDetectorElement& element,
                                IPixelClusteringTool::CellContainer &cellContainer) const
{
  IdentifierHash idHash = RDOs.identifyHash();
  IPixelClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer.startNewModule(idHash));
  if ( pixelDetElStatus.isGood(idHash) ) {
     // Retrieve the cells from the detector element
     std::span<IPixelClusteringTool::CellContainer::Cell>
        cellRange = unpackRDOs(RDOs, pixelDetElStatus, element, cellContainer);

     static constexpr unsigned int SORT_BY_LOCAL_X=0u;
     namespace CL=Acts::InPlaceClusterization;
     CL::clusterize<SORT_BY_LOCAL_X, std::uint16_t>(cellRange,
                                                    CL::defaultConnectionHelper<CL::EConnectionType::CommonEdgeOrCorner>(cellRange));
     // set the cell range per cluster
     Acts::InPlaceClusterization::for_each_cluster(cellRange,
                                                   [&cellContainer](std::span<IPixelClusteringTool::CellContainer::Cell> &/*the_range*/,
                                                                    unsigned int idx_begin,
                                                                    unsigned int idx_end) {
        cellContainer.registerNewCluster(idx_begin,idx_end);
     });
  }
  // must add a range for every call otherwise the cell container and
  // the list of processed modules get out of sync.
  cellContainer.registerClustersForNewModule(rangeGuard.range());
  
  return StatusCode::SUCCESS;
}


std::any PixelClusteringTool::createEventDataCache(xAOD::PixelClusterContainer& cont,
                                                   [[maybe_unused]] std::size_t nClusterRDOs) const
{
  return std::any (xAOD::PixelCluster::ClusterVars (cont, nClusterRDOs));
}


StatusCode
PixelClusteringTool::makeClusters(const EventContext& ctx,
                                  const RDOContainer &rdoContainer,
                                  const IPixelClusteringTool::CellContainer& cellContainer,
                                  unsigned int imodule,
                                  const InDetDD::SiDetectorElement& element,
                                  unsigned int icluster,
                                  [[maybe_unused]] xAOD::PixelClusterContainer& cont,
                                  std::any& cache) const
{
  // Retrieve the calibration data
  const PixelChargeCalibCondData *calibData = nullptr;
  if (not m_chargeDataKey.empty()) {
    SG::ReadCondHandle<PixelChargeCalibCondData> calibDataHandle = SG::makeHandle( m_chargeDataKey, ctx );
    calibData = calibDataHandle.cptr();
    
    if (!calibData) {
      ATH_MSG_ERROR("PixelChargeCalibCondData requested but couldn't be retrieved from " << m_chargeDataKey.key());
      return StatusCode::FAILURE;
    }
  }
  
  // Get the element design
  const InDetDD::PixelModuleDesign& design = 
    static_cast<const InDetDD::PixelModuleDesign&>(element.design());
  
  // Get the calibration strategy for this module. 
  // Default to RD53 if the calibData is not available. That is fine because it won't be used anyway
  auto calibrationStrategy = calibData ? calibData->getCalibrationStrategy(element.identifyHash()) : PixelChargeCalibCondData::CalibrationStrategy::RD53;

  IdentifierHash idHash = element.identifyHash();
  double lorentzShift = m_pixelLorentzAngleTool->getLorentzShift(idHash, ctx);

  auto* clusterVars = std::any_cast<xAOD::PixelCluster::ClusterVars> (&cache);
  if (!clusterVars) throw std::bad_any_cast();

  const InDetRawDataCollection<PixelRDORawData>* rdos = rdoContainer.indexFindPtr(idHash);
  if (!rdos) return StatusCode::FAILURE;

  using CellContainerProxy = InPlaceClusterization::CellContainerProxy<const IPixelClusteringTool::CellContainer>;
  using ModuleProxy = InPlaceClusterization::ModuleProxy<const IPixelClusteringTool::CellContainer>;
  using ClusterProxy = InPlaceClusterization::ClusterProxy<const IPixelClusteringTool::CellContainer>;
  CellContainerProxy cellContainerProxy(&cellContainer);
  ModuleProxy moduleProxy(cellContainerProxy[imodule]);

  for (ClusterProxy clusterProxy: moduleProxy) {
     ATH_CHECK(makeCluster(icluster++,
                           clusterProxy,
                           element,
                           design,
                           *rdos,
                           calibData,
                           calibrationStrategy,
                           lorentzShift,
                           *clusterVars));
  }
  
  return StatusCode::SUCCESS;
}

std::span<IPixelClusteringTool::CellContainer::Cell>
PixelClusteringTool::unpackRDOs(const RawDataCollection& RDOs,
				const InDet::SiDetectorElementStatus& pixelDetElStatus,
				const InDetDD::SiDetectorElement& element,
                                IPixelClusteringTool::CellContainer &cellContainer) const
{
  // Get the element design
  const InDetDD::PixelModuleDesign& design =
    static_cast<const InDetDD::PixelModuleDesign&>(element.design());
  
  bool check_ganged = !m_isITk  && m_checkGanged && isFEI3(element);

  IPixelClusteringTool::CellContainer::ModuleRangeGuard rangeGuard(cellContainer, RDOs.identifyHash() );
  unsigned int rdo_i=0;
  for (const auto *const rdo : RDOs) {
    const Identifier& rdoID = rdo->identify();

    std::array<InDetDD::PixelDiodeTree::CellIndexType,2> diode_idx
      = InDetDD::PixelDiodeTree::makeCellIndex(m_pixelID->phi_index(rdoID),
                                               m_pixelID->eta_index(rdoID));
    InDetDD::PixelDiodeTree::DiodeProxy si_param ( design.diodeProxyFromIdx(diode_idx));
    std::uint32_t fe = design.getFE(si_param);
    
    // check if good RDO
    // the pixel RDO tool here says always good if m_useModuleMap is false
    if (pixelDetElStatus.isChipGood(rangeGuard.identifyHash(), fe)) {
       std::array<std::int16_t,2> coordinates{check_integer_cast<int16_t>(diode_idx[0]),
                                              check_integer_cast<int16_t>(diode_idx[1])};
       cellContainer.emplace_back_cell(coordinates, rdo_i);
    
       if ( check_ganged ) {
          std::optional<std::array<std::int16_t,2> > gangedCoordinates = getGangedCoordinates(coordinates, element);
          if (gangedCoordinates.has_value()) {
             cellContainer.emplace_back_cell(*gangedCoordinates, rdo_i);
          }
       }
    }
    ++rdo_i;
  }

  return rangeGuard.moduleCellSpan();
}

} // namespace ActsTrk
