/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "src/HgtdClusteringTool.h"
#include "HGTD_ReadoutGeometry/HGTD_ModuleDesign.h"

namespace ActsTrk {

  HgtdClusteringTool::HgtdClusteringTool(const std::string& type,
					 const std::string& name,
					 const IInterface* parent)
    : base_class(type, name, parent)
  {}

  StatusCode HgtdClusteringTool::initialize()
  {
    ATH_MSG_INFO("Initializing HgtdClusteringTool...");

    ATH_CHECK(detStore()->retrieve(m_hgtd_det_mgr, "HGTD"));
    return StatusCode::SUCCESS;
  }

  StatusCode HgtdClusteringTool::clusterize(const EventContext&,
					    const RawDataCollection& RDOs,
					    ClusterContainer& container) const
  {
    ATH_MSG_DEBUG("Clustering hits...");

    // Fast insertion trick
    std::size_t previousSize = container.size();
    std::vector<xAOD::HGTDCluster*> toAdd;
    toAdd.reserve(RDOs.size());
    for (std::size_t i(0), n(RDOs.size()); i < n; ++i)
      toAdd.push_back( new xAOD::HGTDCluster() );
    container.insert(container.end(), toAdd.begin(), toAdd.end());

    for	(std::size_t i(0), n(RDOs.size()); i < n; ++i) {
        const auto* rdo = RDOs[i];
        Identifier rdo_id = rdo->identify();
        const InDetDD::HGTD_DetectorElement* element = m_hgtd_det_mgr->getDetectorElement(rdo_id);

	InDetDD::SiCellId si_cell_id = element->cellIdFromIdentifier(rdo_id);

	InDetDD::SiLocalPosition si_pos = element->design().localPositionOfCell(si_cell_id);


	Eigen::Matrix<float, 3, 1> loc_pos(si_pos.xPhi(), si_pos.xEta(),rdo->getTOA());
	Eigen::Matrix<float, 3, 3> cov_matrix= Eigen::Matrix<float, 3, 3>::Zero();

	float xWidth = 1.3;
	float yWidth = 1.3;
	cov_matrix(0,0) = xWidth * xWidth / 12; // i.e. Cov XX
	cov_matrix(1,1) = yWidth * yWidth / 12; // i.e. Cov YY
	float time_of_arrival_err = 0.035;
	cov_matrix(2,2) = time_of_arrival_err * time_of_arrival_err; // i.e. Cov TT

        std::vector<Identifier> rdo_list = {rdo_id};
        std::vector<int> time_over_threshold = {static_cast<int>(rdo->getTOT())};

	IdentifierHash id_hash = RDOs.identifierHash();

	// Fill
	xAOD::HGTDCluster* cluster = container[previousSize + i];
	cluster->setMeasurement<3>(id_hash,loc_pos,cov_matrix);
	cluster->setIdentifier(rdo_id.get_compact());
	cluster->setRDOlist(std::move(rdo_list));
        cluster->setToTlist(std::move(time_over_threshold));
    }


    return StatusCode::SUCCESS;
  }


} // namespace

