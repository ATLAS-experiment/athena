/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#include "HgtdClusteringToolBase.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"

#include "details/HgtdCollectionAdapter.h"
#include "details/HgtdAuxDataCache.h"

#include "AthenaKernel/Units.h"

namespace {
   static constexpr float acts_ns_over_athena_ns = Acts::UnitConstants::ns / Athena::Units::nanosecond;
   float toActsTime(float athena_time) {
      return athena_time * acts_ns_over_athena_ns;
   }
}

namespace ActsTrk {
  HgtdClusteringToolBase::HgtdClusteringToolBase(const std::string& type,
						   const std::string& name,
						   const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode HgtdClusteringToolBase::initialize()
  {
    ATH_MSG_DEBUG("Initializing " << name() << "...");

    ATH_CHECK(detStore()->retrieve(m_hgtd_det_mgr, "HGTD"));
    ATH_CHECK(detStore()->retrieve(m_hgtd_id, "HGTD_ID"));
    ATH_CHECK(m_hgtd_tdc_calib_tool.retrieve(EnableTool{m_use_altiroc_rdo.value()}));

    return StatusCode::SUCCESS;
  }

  std::pair<unsigned int, unsigned int>
  HgtdClusteringToolBase::countCells(const RDOContainerVariant& rdoContainer,
                                      const std::vector<IdentifierHash> &listOfIds) const {
     unsigned int n_hits=std::visit([&listOfIds](const auto *theRdoContainer) -> unsigned int {
        auto getNHits =[](const auto &RDOs)
           -> unsigned int
        {
           return RDOs.size();
        };
        unsigned int n_hits=0;
        if (listOfIds.empty()) {
           for (const auto *RDOs : *theRdoContainer) {
              assert( RDOs);
              n_hits += getNHits(*RDOs);
           }
        }
        else {
           for (const IdentifierHash& id : listOfIds) {
              if (not id.is_valid()) continue;
              const auto *RDOs = theRdoContainer->indexFindPtr(id);
              if (RDOs) { // necessary ?
                 n_hits += getNHits(*RDOs);
              }
           }
        }
        return n_hits;
     },rdoContainer);
     // best guess for the number of clusters is the number of RDOs
     return {n_hits,n_hits};
  }

  // @TODO move to header
  struct HgtdAuxDataCache : xAOD::VariableStruct {
      HgtdAuxDataCache(SG::AuxVectorData& cont, unsigned int n_cluster_rdos)
         : xAOD::VariableStruct(cont)
      {
         if (n_cluster_rdos>0) {
            auto *store = cont.getStore();
            assert(store);
            SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::rdoListAcc(), n_cluster_rdos, rdoList, rdoListPayload);
            SG::setJaggedVectorData(cont,*store,xAOD::HGTDCluster::totListAcc(), n_cluster_rdos, totList, totListPayload);
         }
      }
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Elt_span     rdoList;
      SG::Accessor<SG::JaggedVecElt<Identifier::value_type> >::Payload_span rdoListPayload;
      SG::Accessor<SG::JaggedVecElt<int> >::Elt_span     totList;
      SG::Accessor<SG::JaggedVecElt<int> >::Payload_span totListPayload;
  };

   std::any HgtdClusteringToolBase::createEventDataCache(xAOD::HGTDClusterContainer& cont, std::size_t nClusterRDOs) const {
       return HgtdAuxDataCache(cont,nClusterRDOs);
   }

  StatusCode HgtdClusteringToolBase::makeClusters([[maybe_unused]] const EventContext& ctx,
                                                   const RDOContainerVariant& rdoContainer,
                                                   const CellContainer& cellContainer,
                                                   unsigned int imodule,
                                                   unsigned int icluster_in,
                                                   xAOD::HGTDClusterContainer& container,
                                                   std::any& cache) const
  {
    HgtdAuxDataCache* auxDataCache = std::any_cast<HgtdAuxDataCache> (&cache);
    if (!auxDataCache) throw std::bad_any_cast();
    return std::visit([auxDataCache, this, &cellContainer,imodule,icluster_in,&container](const auto *rdoContainer)
                      -> StatusCode
    {
       using CellContainerProxy = InPlaceClusterization::CellContainerProxy<const IHGTDClusteringTool::CellContainer>;
       using ModuleProxy = InPlaceClusterization::ModuleProxy<const IHGTDClusteringTool::CellContainer>;
       using ClusterProxy = InPlaceClusterization::ClusterProxy<const IHGTDClusteringTool::CellContainer>;
       

       CellContainerProxy cellContainerProxy(&cellContainer);
       ModuleProxy moduleProxy(cellContainerProxy[imodule]);

       const auto* RDOs = rdoContainer->indexFindPtr(moduleProxy.identifyHash());
       if (!RDOs) return StatusCode::FAILURE;
       HgtdCollectionAdapter< std::remove_cvref_t<decltype(*RDOs)> > rdoAdapter(*m_hgtd_det_mgr,
                                                                               *m_hgtd_tdc_calib_tool,
                                                                               RDOs->identify());

       unsigned int icluster=icluster_in;
       assert( icluster+moduleProxy.size() <= container.size() );
       for (ClusterProxy clusterProxy: moduleProxy) {
          assert(icluster < container.size());
          assert(icluster == container[icluster]->index());
          xAOD::HGTDCluster *hgtdCluster= container[icluster];
          assert(hgtdCluster);
          ATH_CHECK(makeCluster(icluster, *hgtdCluster, clusterProxy, RDOs, rdoAdapter, auxDataCache));
          ++icluster;
       }
       
       return StatusCode::SUCCESS;

    }, rdoContainer);
  }

  template <typename T_RDOCollection>
  StatusCode HgtdClusteringToolBase::makeCluster(size_t icluster,
                                                  xAOD::HGTDCluster& xaodcluster,
                                                  const HgtdClusteringToolBase::ClusterProxy &clusterProxy,
                                                  const T_RDOCollection *RDOs,
                                                  const HgtdCollectionAdapter<T_RDOCollection> &rdoAdapter,
                                                  HgtdAuxDataCache* auxDataCache) const
  {
    assert(!clusterProxy.empty());
    InDetDD::SiLocalPosition pos_acc(0,0);
    double av_time_of_flight = 0;

    unsigned int n_rdos = (icluster> 0 ?  auxDataCache->rdoList[icluster-1].end() : 0u);
    assert(icluster==0 || n_rdos == auxDataCache->totList[icluster-1].end());
    assert( n_rdos+clusterProxy.size() <= auxDataCache->rdoListPayload.size() );
    using CellProxy = InPlaceClusterization::CellProxy<const IHGTDClusteringTool::CellContainer>;
    Identifier waferId (rdoAdapter.identify());
    std::optional<Identifier> first_id;
    for (CellProxy cellProxy : clusterProxy) {
      Identifier rdo_id = m_hgtd_id->pixel_id(waferId, cellProxy.coordinates()[0], cellProxy.coordinates()[1]);
      if (!first_id.has_value()) { first_id=rdo_id; }
      // @TODO get detector only once, all clusters should belong to the same detector element
      InDetDD::SiCellId si_cell_id( cellProxy.coordinates()[0],
                                    cellProxy.coordinates()[1]);
      InDetDD::SiLocalPosition si_pos = rdoAdapter.element().design().localPositionOfCell(si_cell_id);

      auxDataCache->rdoListPayload[n_rdos]=rdo_id.get_compact();

      assert(cellProxy.srcIndex() < RDOs->size());
      const  auto *rdo = (*RDOs)[cellProxy.srcIndex()];
      
      auxDataCache->totListPayload[n_rdos]=rdoAdapter.ToT(*rdo);
      ++n_rdos;

      pos_acc += si_pos;
      float time_of_flight = toActsTime(rdoAdapter.calibratedTime(*rdo, cellProxy.coordinates()[2]));
      av_time_of_flight += time_of_flight;
    }

    double inv_n_rdos = 1./clusterProxy.size();
    pos_acc *= inv_n_rdos;
    av_time_of_flight *= inv_n_rdos;

    // Create the cluster
    Eigen::Matrix<float, 3, 1> loc_pos(pos_acc.xPhi(), pos_acc.xEta(), static_cast<float>(av_time_of_flight));
    Eigen::Matrix<float, 3, 3> cov_matrix= Eigen::Matrix<float, 3, 3>::Zero();

    constexpr float xWidth = 1.3;
    constexpr float yWidth = 1.3;
    // @TODO should the width not just be divided by the number of cells in x/y-direction ?
    cov_matrix(0,0) = xWidth * xWidth / 12 * inv_n_rdos; // i.e. Cov XX 
    cov_matrix(1,1) = yWidth * yWidth / 12 * inv_n_rdos; // i.e. Cov YY 
    constexpr float time_of_arrival_err = 0.035;
    cov_matrix(2,2) = time_of_arrival_err * time_of_arrival_err * inv_n_rdos; // i.e. Cov TT
    // @TODO get detector element only once by caller aka. makeClusters
    IdentifierHash id_hash = clusterProxy.identifyHash();

    // Fill
    xaodcluster.setMeasurement<3>(id_hash,loc_pos,cov_matrix);
    assert(first_id.has_value());
    xaodcluster.setIdentifier(first_id->get_compact());
    auxDataCache->rdoList[icluster] = n_rdos;
    auxDataCache->totList[icluster] = n_rdos;

    return StatusCode::SUCCESS;
  }
} // namespace

