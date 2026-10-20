/*
 *   Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
 */

#ifndef L1MUONNSW_MMT_HIT_H
#define L1MUONNSW_MMT_HIT_H

#include "GeoPrimitives/GeoPrimitives.h"
#include "Identifier/Identifier.h"
#include "MuonIdHelpers/MmIdHelper.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"
#include "MuonReadoutGeometryR4/MmReadoutElement.h"
#include "ActsGeometryInterfaces/GeometryContext.h"

#include "L1MMDefinitions.h"

#include <cmath>

namespace L1Muon::L1MM {

  /**
   * @brief One MM strip hit as seen by the MM trigger processor.
   *
   * Built from one MM digit (strip identifier, strip time and the bunch crossing derived from it).
   * Everything is computed once in the constructor:
   *  - the identifier fields (station eta/phi, sector, strip) and the plane 0-7,
   *    (multilayer-1)*4 + gasGap-1;
   *  - the layer type, X, U or V, from the stereo angle of the strip design of the layer;
   *  - the electronics mapping: the strip index on the sector layer, and the VMM (0-127) and
   *    ART ASIC (0-3) of the strip;
   *  - from the Phase-II geometry (with the alignment of the geometry context): the global R and Z of
   *    the strip centre, the slope R/Z, the strip pitch over Z, and the shift, i.e. the slope origin
   *    of the road windows of MMT_Road for this hit.
   * Only the age changes afterwards (setAge), when the hit is copied into a road.
   *
   * The caller must guarantee that the strip is inside the active range of its layer: the Phase-II
   * strip position of a strip outside it is undefined.
   */
  class MMT_Hit {
    public:
      MMT_Hit(const Identifier& id, const float stripTime, const int BC,
              const MuonGMR4::MuonDetectorManager* detManager,
              const ActsTrk::GeometryContext& gctx);

      ~MMT_Hit() = default;

      // Getters
      int getART() const { return m_ART_ASIC; }
      int getAge() const { return m_age; }
      int getBC() const { return m_BC_time; }
      int getChannel() const { return m_strip; }
      /// Strip index on the sector layer: (|stationEta|-1)*kStripsEta1 + channel
      int getGlobalStrip() const { return m_globalStrip; }
      int getPlane() const { return m_plane; }
      char getSector() const { return m_sector; }
      double getRZSlope() const { return m_RZslope; }
      int getVMM() const { return m_VMM_chip; }
      double getShift() const { return m_shift; }
      int getStationEta() const { return m_station_eta; }
      int getStationPhi() const { return m_station_phi; }
      int getSectorPhi() const { return m_sector_phi; }
      double getR() const { return m_R; }
      double getRp() const { return m_Rp; }
      double getZ() const { return m_Z; }
      double getPitchOverZ() const { return m_PitchOverZ; }
      float getTime() const { return m_time; }
      LayerType getType() const { return m_type; }
      bool isX() const { return m_type == LayerType::X; }
      bool isU() const { return m_type == LayerType::U; }
      bool isV() const { return m_type == LayerType::V; }
      bool infSlope() const { return std::isinf(m_RZslope); }

      // Debug getters (new w.r.t. Run 3): reference quantities entering m_Rp / m_shift
      /// radius of the bottom edge of the eta=1 module
      double getRefR0() const { return m_refR0; }
      /// z of the first active strip, multilayer 1, gas gap 1
      double getRefZ0() const { return m_refZ0; }

      // Setters
      void setAge(int age) { m_age = age; }

    private:
      double m_RZslope{-1};
      double m_Rp{-1};
      double m_Z{-1};
      double m_R{-1};
      double m_PitchOverZ{-1};
      double m_shift{-1};
      double m_refR0{-1};
      double m_refZ0{-1};
      float m_time{-1.f};
      int m_VMM_chip{-1};
      int m_ART_ASIC{-1};
      int m_plane{-1};
      int m_station_eta{0};
      int m_station_phi{0};
      int m_sector_phi{0};
      int m_strip{-1};
      int m_globalStrip{-1};
      int m_BC_time{-1};
      int m_age{-1};
      char m_sector{'-'};
      LayerType m_type{LayerType::Undefined};
  };

}  // namespace L1Muon::L1MM

#endif  // L1MUONNSW_MMT_HIT_H
