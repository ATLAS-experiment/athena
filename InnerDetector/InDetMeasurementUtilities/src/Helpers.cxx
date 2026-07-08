/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

#include "InDetMeasurementUtilities/Helpers.h"

namespace TrackingUtilities {

  std::pair<float, float> computeOmegas(const xAOD::PixelCluster& cluster,
					const PixelID& pixelID)
  {    
    SG::ConstAccessor<SG::JaggedVecElt<Identifier::value_type> >::element_type
       rdo_list_cluster = cluster.rdoList();
    SG::ConstAccessor<SG::JaggedVecElt<float> >::element_type
       charge_list_cluster = cluster.chargeList();
    SG::ConstAccessor<SG::JaggedVecElt<int> >::element_type
       tot_list_cluster = cluster.totList();

    // Prefer the calibrated charge for the omega weights. When charge calibration
    // is unavailable (e.g. digital clustering) the charge list is left empty, so
    // fall back to the ToT list, mirroring InDet::ClusterMakerTool. This keeps the
    // analogue charge-interpolation correction defined instead of silently
    // disabling it (returning -1) whenever the charge list is missing.
    const bool useCharge = rdo_list_cluster.size() == charge_list_cluster.size();
    if (not useCharge and rdo_list_cluster.size() != tot_list_cluster.size()) {
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
    
    for (std::size_t i(0); i<rdo_list_cluster.size(); ++i) {
      Identifier this_rdo(rdo_list_cluster.at(i));
      const float this_charge = useCharge ? charge_list_cluster.at(i)
                                          : static_cast<float>(tot_list_cluster.at(i));
      
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

