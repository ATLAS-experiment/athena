/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// Tell clang not to allow spurious FPEs.
#include "CxxUtils/trapping_fp.h"
CXXUTILS_TRAPPING_FP;

#include "PixelClusteringTool.h"

#include <xAODInDetMeasurement/PixelCluster.h>
#include <xAODInDetMeasurement/PixelClusterContainer.h>
#include <xAODInDetMeasurement/PixelClusterAuxContainer.h>
#include <InDetPrepRawData/SiWidth.h>
#include <TrkSurfaces/Surface.h>
#include <xAODInDetMeasurement/Utilities.h>

#include "details/PixelRDOCollectionAdapter.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <cstdint>
#include <limits>
#include <optional>
#include <stdexcept>

using CLHEP::micrometer;

namespace {

   template <typename T1, typename T2>
   T1 check_integer_cast(T2 a) {
      assert( std::in_range<T1>(a) );
      return static_cast<T1>(a);
   }
   
   inline bool isFEI3(const InDetDD::PixelModuleDesign& design) {
      return  design.getReadoutTechnology() == InDetDD::PixelReadoutTechnology::FEI3;
   }

   template <typename IndexType>
   inline std::optional<std::array<std::int16_t,2> >
   getGangedCoordinates(const std::array<IndexType,2> &coordinates,
                        const InDetDD::PixelModuleDesign& design)
   {
      // If the pixel is ganged, returns a new identifier for it
      InDetDD::SiCellId cellId(check_integer_cast<int>(coordinates[0]),check_integer_cast<int>(coordinates[1]));
      InDetDD::SiReadoutCellId readoutId = design.readoutIdOfCell(cellId);
      if ( design.numberOfConnectedCells( readoutId ) > 1 ) {
         InDetDD::SiCellId gangedCellId = design.connectedCell( readoutId, 1 );
         return std::array<std::int16_t,2>{ static_cast<std::int16_t>(gangedCellId.phiIndex()),
                                            static_cast<std::int16_t>(gangedCellId.etaIndex())};
      }
      return std::nullopt;
   }

   // PixelRDO_Container
   std::array<std::int16_t,2>
   makeCellCoordinates(const std::array<InDetDD::PixelDiodeTree::CellIndexType,2> &diode_idx) {
      return std::array<std::int16_t,2>{
         check_integer_cast<std::int16_t>(diode_idx[0]),
         check_integer_cast<std::int16_t>(diode_idx[1])};
   }
   // PixelRDO_Container
   const std::array<InDetDD::PixelDiodeTree::CellIndexType,2> &
   makeDiodeIdx(const std::array<InDetDD::PixelDiodeTree::CellIndexType,2> &diode_idx) {
      return diode_idx;
   }

   // PhaseIIPixelRawDataContainer
   std::array<std::int16_t,2>
   makeCellCoordinates(const std::array<std::int16_t,2> &diode_idx) {
      return diode_idx;
   }
   // PhaseIIPixelRawDataContainer
   std::array<InDetDD::PixelDiodeTree::CellIndexType,2>
   makeDiodeIdx(const std::array<std::int16_t,2> &diode_idx) {
      return std::array<InDetDD::PixelDiodeTree::CellIndexType,2>{
         check_integer_cast<InDetDD::PixelDiodeTree::CellIndexType>(diode_idx[0]),
         check_integer_cast<InDetDD::PixelDiodeTree::CellIndexType>(diode_idx[1])};
   }

}

namespace ActsTrk {

template <typename T_RDOContainer>
StatusCode PixelClusteringToolImpl<T_RDOContainer>::initialize()
{
  ATH_MSG_DEBUG("Initializing " << this->name() << " ...");

  ATH_MSG_DEBUG("   " << m_addCorners );
  ATH_MSG_DEBUG("   " << m_useWeightedPos );
  ATH_MSG_DEBUG("   " << m_broadErrors );
  ATH_MSG_DEBUG("   " << m_checkGanged );
    
  ATH_CHECK(this->m_pixelLorentzAngleTool.retrieve());

  ATH_CHECK(this->m_chargeDataKey.initialize(not m_chargeDataKey.empty()));

  ATH_MSG_INFO("   Charge Data Key:" << m_chargeDataKey);
  ATH_MSG_INFO("   ID Helper Name:" << m_idHelperName);

  ATH_CHECK( this->detStore()->retrieve(m_pixelID, m_idHelperName) );
  
  ATH_MSG_DEBUG(this->name() << " successfully initialized");
  return StatusCode::SUCCESS;
}

template <typename T_RDOContainer>
PixelClusteringToolImpl<T_RDOContainer>::PixelClusteringToolImpl(
    const std::string& type, const std::string& name, const IInterface* parent)
    : base_class(type,name,parent)
{}


template <typename T_RDOContainer>
template <bool GANGED>
std::pair<unsigned int, unsigned int>
PixelClusteringToolImpl<T_RDOContainer>::countCellsImpl(const T_RDOContainer& rdo_collection,
                                                        const std::vector<IdentifierHash> &listOfIds,
                                                        const InDetDD::SiDetectorElementCollection &detector_elements) const {
   auto getNHits =[](const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
                     const InDetDD::SiDetectorElementCollection &elements,
		     const PixelID* pixelID)
      -> unsigned int
   {
      unsigned int n_hits = RDOs.size();
      if constexpr(GANGED) {
         assert(elements.at(RDOs.identifyHash()));
         assert(dynamic_cast<const InDetDD::PixelModuleDesign *>(&elements.at(RDOs.identifyHash())->design()) != nullptr);
         const InDetDD::PixelModuleDesign &design = static_cast<const InDetDD::PixelModuleDesign &>(elements.at(RDOs.identifyHash())->design());
         if (isFEI3(design)) {
            for(RDOAdapter<T_RDOContainer> rdo : RDOs) {
               if (rdo.isGanged(design, *pixelID)) {
                  ++n_hits;
               }
            }
         }
      }
      return n_hits;
   };
   unsigned int n_hits=0u;
   if (listOfIds.empty()) {
      for (const RDOCollectionAdapter<T_RDOContainer> RDOs : RDOCollectionAdapter<T_RDOContainer>::range(rdo_collection)) {
         assert( RDOs.isValid());
         n_hits += getNHits(*RDOs, detector_elements, m_pixelID);
      }
   }
   else {
      for (const IdentifierHash& id : listOfIds) {
         if (not id.is_valid()) continue;
         std::optional<RDOCollectionAdapter<T_RDOContainer> > RDOs = RDOCollectionAdapter<T_RDOContainer>::make(rdo_collection,id);
         if (RDOs.has_value()) {
	   n_hits += getNHits(*(RDOs.value()), detector_elements, m_pixelID);
         }
      }
   }
   // the total number of hits is the best estimate of the maximum number of clusters.
   // @TODO could apply a factor for the number clusters or only if the number of hits
   //       are above a certain threshold to reduce memory consumption though impact
   //       is likely small.
   return {n_hits,n_hits};
}

template <typename T_RDOContainer>
std::pair<unsigned int, unsigned int>
PixelClusteringToolImpl<T_RDOContainer>::countCells(const T_RDOContainer& rdo_collection,
                                                const std::vector<IdentifierHash> &listOfIds,
                                                const InDetDD::SiDetectorElementCollection &detector_elements) const {
   if (m_isITk || !m_checkGanged ) {
      return countCellsImpl<false>(rdo_collection,listOfIds, detector_elements);
   }
   else {
      return countCellsImpl<true>(rdo_collection,listOfIds, detector_elements);
   }
}

template <typename T_RDOContainer>
StatusCode
PixelClusteringToolImpl<T_RDOContainer>::makeCluster(size_t icluster,
                                                 const PixelClusteringToolImpl<T_RDOContainer>::ClusterProxy &cluster,
                                                 const InDetDD::SiDetectorElement& element,
                                                 const InDetDD::PixelModuleDesign& design,
                                                 const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &rdos,
                                                 const PixelChargeCalibCondData *calibData,
                                                 const PixelChargeCalibCondData::CalibrationStrategy calibStrategy,
                                                 const double lorentzShift,
                                                 xAOD::PixelCluster::ClusterVars& clusterVars) const
{ 
  Amg::Vector2D pos_acc(0,0);
  float tot_acc = 0.f;
  //to start, set max to the min possible int and min to the max possible int
  static constexpr InDetDD::PixelDiodeTree::CellIndexType defaultMax = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::min();
  static constexpr InDetDD::PixelDiodeTree::CellIndexType defaultMin = std::numeric_limits<InDetDD::PixelDiodeTree::CellIndexType>::max();
  //
  InDetDD::PixelDiodeTree::CellIndexType rowmax = defaultMax;
  InDetDD::PixelDiodeTree::CellIndexType colmax = defaultMax;
  InDetDD::PixelDiodeTree::CellIndexType rowmin = defaultMin;
  InDetDD::PixelDiodeTree::CellIndexType colmin = defaultMin;
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
  float totalCharge = 0.f;
  
  using CellProxy = InPlaceClusterization::CellProxy<const typename IClusteringToolType::CellContainer>;
  for (CellProxy cellProxy : cluster) {
    
    //Construct the identifier class

    // We temporary comment this since it is not used
    // TODO: Check how the ganged info is used in legacy
    // if (multiChip)  {
    //   hasGanged = hasGanged ||
    // 	m_pixelRDOTool->isGanged(id, element).has_value();
    // }
    assert(cellProxy.srcIndex() < rdos.size());

    RDOAdapter<T_RDOContainer> rdo(rdos[cellProxy.srcIndex()]);
    if constexpr(std::is_same_v<T_RDOContainer, PhaseIIPixelRawDataContainer>) {
       assert( rdo.index() >= rdos.beginIndex() && rdo.index() < rdos.endIndex() );
    }

    Identifier rdo_id = rdo.computeIdentifier(*m_pixelID,module_id, cellProxy);
    if (!first_rdo_id.has_value()) {
       first_rdo_id=rdo_id.get_compact();
    }

    assert(rdo.getLVL1A()>=0 &&  rdo.getLVL1A() < std::numeric_limits<uint8_t>::max());
    cluster_lvl1min = std::min(cluster_lvl1min, static_cast<int>(rdo.getLVL1A()) );

    const int tot = rdo.getToT();
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
    totalCharge += charge;
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
    
  } // loop on cluster's cells
  assert(n_rdos>0); // clusters must not be empty
  if (tot_acc > 0)
    pos_acc /= tot_acc;
  
  const long long diffCol = colmax - colmin + 1;
  const long long diffRow = rowmax - rowmin + 1;
  assert(std::in_range<int>(diffCol));
  assert(std::in_range<int>(diffRow));
  const int colWidth = static_cast<int>(diffCol);
  const int rowWidth = static_cast<int>(diffRow);

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
  clusterVars.totalCharge[icluster] = totalCharge;
  clusterVars.lvl1a[icluster] = cluster_lvl1min;
  clusterVars.channelsInPhi[icluster] = rowWidth;
  clusterVars.channelsInEta[icluster] = colWidth;
  clusterVars.widthInEta[icluster] = etaWidth;
    
  return StatusCode::SUCCESS;
}

template <typename T_RDOContainer>
StatusCode
PixelClusteringToolImpl<T_RDOContainer>::clusterize([[maybe_unused]] const EventContext& ctx,
                                                const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
                                                const InDet::SiDetectorElementStatus& pixelDetElStatus,
                                                const InDetDD::SiDetectorElement& element,
                                                ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType::CellContainer &cellContainer) const
{
  IdentifierHash idHash = RDOs.identifyHash();
  typename IClusteringToolType::CellContainer::ModuleRangeGuard rangeGuard(cellContainer.startNewModule(idHash));
  if ( pixelDetElStatus.isGood(idHash) ) {
     // Retrieve the cells from the detector element
     std::span<typename IClusteringToolType::CellContainer::Cell>
        cellRange = unpackRDOs(RDOs, pixelDetElStatus, element, cellContainer);

     static constexpr unsigned int SORT_BY_LOCAL_X=0u;
     namespace CL=Acts::InPlaceClusterization;
     CL::clusterize<SORT_BY_LOCAL_X, std::uint16_t>(cellRange,
                                                    CL::defaultConnectionHelper<CL::EConnectionType::CommonEdgeOrCorner>(cellRange));
     // set the cell range per cluster
     Acts::InPlaceClusterization::for_each_cluster(cellRange,
                                                   [&cellContainer](std::span<typename IClusteringToolType::CellContainer::Cell> &/*the_range*/,
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


template <typename T_RDOContainer>
std::any PixelClusteringToolImpl<T_RDOContainer>::createEventDataCache(xAOD::PixelClusterContainer& cont,
                                                   [[maybe_unused]] std::size_t nClusterRDOs) const
{
  return std::any (xAOD::PixelCluster::ClusterVars (cont, nClusterRDOs));
}


template <typename T_RDOContainer>
StatusCode
PixelClusteringToolImpl<T_RDOContainer>::makeClusters(
   const EventContext& ctx,
   const T_RDOContainer &rdoContainer,
   const typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType::CellContainer& cellContainer,
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

  std::optional<RDOCollectionAdapter<T_RDOContainer> > rdos_optional(RDOCollectionAdapter<T_RDOContainer>::make(rdoContainer,idHash));
  if (!rdos_optional.has_value()) return StatusCode::FAILURE;
  const RDOCollectionAdapter<T_RDOContainer> &rdos(*rdos_optional);

  using CellContainerProxy = InPlaceClusterization::CellContainerProxy<const typename IClusteringToolType::CellContainer>;
  using ModuleProxy = InPlaceClusterization::ModuleProxy<const typename IClusteringToolType::CellContainer>;
  using ClusterProxy = InPlaceClusterization::ClusterProxy<const typename IClusteringToolType::CellContainer>;
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

template <typename T_RDOContainer>
std::span<typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType::CellContainer::Cell>
PixelClusteringToolImpl<T_RDOContainer>::unpackRDOs(
    const ActsTrk::RDOContainerTraits<T_RDOContainer>::PerModuleRDOs &RDOs,
    const InDet::SiDetectorElementStatus& pixelDetElStatus,
    const InDetDD::SiDetectorElement& element,
    typename ActsTrk::RDOContainerTraits<T_RDOContainer>::IClusteringToolType::CellContainer &cellContainer) const
{
  // Get the element design
  const InDetDD::PixelModuleDesign& design =
    static_cast<const InDetDD::PixelModuleDesign&>(element.design());
  
  bool check_ganged = !m_isITk  && m_checkGanged && isFEI3(design);

  typename IClusteringToolType::CellContainer::ModuleRangeGuard rangeGuard(cellContainer, RDOs.identifyHash() );
  unsigned int rdo_i=0;
  for (RDOAdapter<T_RDOContainer> rdo : RDOs) {
    auto coordinates=rdo.coordinates(*m_pixelID);
    InDetDD::PixelDiodeTree::DiodeProxy si_param ( design.diodeProxyFromIdx(makeDiodeIdx(coordinates)));
    std::uint32_t fe = design.getFE(si_param);

    // check if good RDO
    // the pixel RDO tool here says always good if m_useModuleMap is false
    if (pixelDetElStatus.isChipGood(rangeGuard.identifyHash(), fe)) {
       cellContainer.emplace_back_cell(makeCellCoordinates(coordinates), rdo_i);
    
       if ( check_ganged ) {
          std::optional<std::array<std::int16_t,2> > gangedCoordinates = getGangedCoordinates(coordinates, design);
          if (gangedCoordinates.has_value()) {
             cellContainer.emplace_back_cell(*gangedCoordinates, rdo_i);
          }
       }
    }
    ++rdo_i;
  }

  return rangeGuard.moduleCellSpan();
}

template class PixelClusteringToolImpl<PixelRDO_Container>;
template class PixelClusteringToolImpl<PhaseIIPixelRawDataContainer>;
} // namespace ActsTrk
