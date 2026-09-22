/**
 * Copyright (C) 2002-2022 CERN for the benefit of the ATLAS collaboration.
 *
 * @file HGTD_PrepRawData/src/HGTD_Cluster.cxx
 * @author Alexander Leopold <alexander.leopold@cern.ch>
 * @date Apr, 2021
 * @brief Implementation file for class HGTD_Cluster.
 */

#include "HGTD_PrepRawData/HGTD_Cluster.h"

#include <utility>


// Constructor for use from tp converter
HGTD_Cluster::HGTD_Cluster(const Identifier& rdo_id,
                           const Amg::Vector2D& loc_pos,
                           std::vector<Identifier>&& rdo_list,
                           const InDet::SiWidth& width,
                           const InDetDD::SolidStateDetectorElementBase* det_el,
                           Amg::MatrixX&& loc_err_matx,
                           const float time_of_arrival,
                           const float time_of_arrival_err,
                           std::vector<int>&& time_over_threshold)
    : PrepRawData(rdo_id, loc_pos, std::move(rdo_list), std::move(loc_err_matx)),
      m_width(width),
      m_det_el(det_el),
      m_time(time_of_arrival),
      m_time_resolution(time_of_arrival_err),
      m_time_over_threshold(std::move(time_over_threshold)) {}

// Default constructor:
HGTD_Cluster::HGTD_Cluster() : PrepRawData() {
  m_det_el = nullptr;
}
