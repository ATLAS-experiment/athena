/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMeasurementUtilities/Helpers.h"

namespace TrackingUtilities {

  std::pair<float, float> computeOmegas(const xAOD::PixelCluster& cluster,
					const PixelID& pixelID)
  {    
    const std::vector<Identifier>& rod_list_cluster = cluster.rdoList();
    const std::vector<float>& charge_list_cluster = cluster.chargeList();
    
    if (rod_list_cluster.size() != charge_list_cluster.size()) {
      return {-1.f, -1.f};
    }

    int colmax = std::numeric_limits<int>::min();
    int rowmax = std::numeric_limits<int>::min();
    int colmin = std::numeric_limits<int>::max();
    int rowmin = std::numeric_limits<int>::max();
    
    float qRowMin = 0.f;
    float qRowMax = 0.f;
    float qColMin = 0.f;
    float qColMax = 0.f;
    
    for (std::size_t i(0); i<rod_list_cluster.size(); ++i) {
      const Identifier& this_rdo = rod_list_cluster.at(i);
      const float this_charge = charge_list_cluster.at(i);
      
      const int row = pixelID.phi_index(this_rdo);
      if (row > rowmax) {
        rowmax = row;
        qRowMax = this_charge;
      } else if (row == rowmax) {
        qRowMax += this_charge;
      }
      
      if (row < rowmin) {
        rowmin = row;
        qRowMin = this_charge;
      } else if (row == rowmin) {
        qRowMin += this_charge;
      }

      const int col = pixelID.eta_index(this_rdo);
      if (col > colmax) {
        colmax = col;
	qColMax = this_charge;
      } else if (col == colmax) {
        qColMax += this_charge;
      }

      if (col < colmin) {
	colmin = col;
        qColMin = this_charge;
      } else if (col == colmin) {
        qColMin += this_charge;
      }
    } // loop on rdos and charges

    float omegax = -1.f;
    float omegay = -1.f;
    if(qRowMin + qRowMax > 0) omegax = qRowMax/(qRowMin + qRowMax);
    if(qColMin + qColMax > 0) omegay = qColMax/(qColMin + qColMax);

    return std::make_pair(omegax, omegay);
  }

}

