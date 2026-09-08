/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/


// EDM include(s):
#include "xAODCore/AuxStoreAccessorMacros.h"

// Local include(s):
#include "xAODMuon/versions/MuonSegment_v1.h"

namespace {
  using namespace Muon::MuonStationIndex;
}

namespace xAOD {


    Amg::Vector3D MuonSegment_v1::position() const {
        return Amg::Vector3D{x(), y(), z()};
    }
    Amg::Vector3D MuonSegment_v1::direction() const {
        return Amg::Vector3D{px(), py(), pz()};
    }

  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, x )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, y )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, z )

  void MuonSegment_v1::setPosition(float x, float y, float z) {
    static const Accessor<float> accX( "x" );
    static const Accessor<float> accY( "y" );
    static const Accessor<float> accZ( "z" );

    accX( *this ) = x;
    accY( *this ) = y;
    accZ( *this ) = z;
  }
  
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, px )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, py )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, pz )

  void MuonSegment_v1::setDirection(float px, float py, float pz) {
    static const Accessor<float> accX( "px" );
    static const Accessor<float> accY( "py" );
    static const Accessor<float> accZ( "pz" );
    accX(*this) = px;
    accY(*this) = py;
    accZ(*this) = pz;
  }

  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, t0      )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, t0error )

  void MuonSegment_v1::setT0Error(float t0, float t0error) {
    static const Accessor<float> acc1("t0");
    static const Accessor<float> acc2("t0error");
    acc1( *this ) = t0;
    acc2( *this ) = t0error;   
  }

  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, chiSquared )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, float, numberDoF  )

  void MuonSegment_v1::setFitQuality(float chiSquared, float numberDoF) {
    static const Accessor<float> acc1( "chiSquared" );
    static const Accessor<float> acc2( "numberDoF" );
    acc1( *this ) = chiSquared;     
    acc2( *this ) = numberDoF;   
  }

  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, int, sector )
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MuonSegment_v1, int, ChIndex, chamberIndex )
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, int, etaIndex )
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MuonSegment_v1, int, TechnologyIndex, technology )

  void MuonSegment_v1::setIdentifier(const std::uint8_t sector, 
                                     const ChIndex chamberIndex, 
                                     const std::int8_t etaIndex, 
                                     const TechnologyIndex technology) {
    static const Accessor<int> acc1( "sector" );
    static const Accessor<int> acc2( "chamberIndex" );
    static const Accessor<int> acc3( "etaIndex" );
    static const Accessor<int> acc4( "technology" );
    acc1(*this) = sector;
    acc2(*this) = toInt(chamberIndex);
    acc3(*this) = etaIndex;
    acc4(*this) = toInt(technology);
  }

  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MuonSegment_v1, int, std::uint8_t, nPrecisionHits )
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MuonSegment_v1, int, std::uint8_t, nPhiLayers     )
  AUXSTORE_PRIMITIVE_GETTER_WITH_CAST( MuonSegment_v1, int, std::uint8_t, nTrigEtaLayers )

  void MuonSegment_v1::setNHits(const std::uint8_t nPrecisionHits,
                                const std::uint8_t nPhiLayers,
                                const std::uint8_t nTrigEtaLayers) {
    static const Accessor<int> acc1{"nPrecisionHits"};
    static const Accessor<int> acc2{"nPhiLayers"};
    static const Accessor<int> acc3{"nTrigEtaLayers"};

    acc1(*this) = nPrecisionHits;
    acc2(*this) = nPhiLayers;
    acc3(*this) = nTrigEtaLayers;
  }


  void MuonSegment_v1::setNOutliers(const std::uint8_t nPrecOutliers,
                                    const std::uint8_t nTrigPhiOutliers,
                                    const std::uint8_t nTrigEtaOutliers) {

    static const Accessor<std::uint8_t> acc_prec{"nPrecisionOutliers"};
    static const Accessor<std::uint8_t> acc_trigEta{"nTriggerPhiOutliers"};
    static const Accessor<std::uint8_t> acc_trigPhi{"nTriggerEtaOutliers"};

    acc_prec(*this) = nPrecOutliers;
    acc_trigEta(*this) = nTrigEtaOutliers;
    acc_trigPhi(*this) = nTrigPhiOutliers;
  }
  
  void MuonSegment_v1::setNHoles(const std::uint8_t nPrecHoles,
                                 const std::uint8_t nTrigPhiHoles,
                                 const std::uint8_t nTrigEtaHoles) {
    static const Accessor<std::uint8_t> acc_prec{"nPrecisionHoles"};
    static const Accessor<std::uint8_t> acc_trigEta{"nTriggerPhiHoles"};
    static const Accessor<std::uint8_t> acc_trigPhi{"nTriggerEtaHoles"};

    acc_prec(*this) = nPrecHoles;
    acc_trigEta(*this) = nTrigEtaHoles;
    acc_trigPhi(*this) = nTrigPhiHoles;
  }
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nPrecisionOutliers)
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nTriggerPhiOutliers)
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nTriggerEtaOutliers)
  
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nPrecisionHoles)
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nTriggerPhiHoles)
  AUXSTORE_PRIMITIVE_GETTER( MuonSegment_v1, std::uint8_t, nTriggerEtaHoles)
 


  #if !(defined(GENERATIONBASE) || defined(XAOD_ANALYSIS))
    AUXSTORE_OBJECT_SETTER_AND_GETTER( MuonSegment_v1, ElementLink< ::Trk::SegmentCollection > , muonSegment, setMuonSegment )
  #endif // not XAOD_ANALYSIS or GENERATIONBASE

} // namespace xAOD
