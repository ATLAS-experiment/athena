/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

// RIO_OnTrackCreator.h
//   Header file for class RIO_OnTrackCreator
///////////////////////////////////////////////////////////////////
// (c) ATLAS Detector software
///////////////////////////////////////////////////////////////////
// Wolfgang.Liebig@cern.ch, Andreas.Salzburger@cern.ch
///////////////////////////////////////////////////////////////////


#ifndef TRKTOOLS_RIOONTRACKCREATOR_H
#define TRKTOOLS_RIOONTRACKCREATOR_H

// Athena
#include "AthenaBaseComps/AthAlgTool.h"
// Trk
#include "TrkToolInterfaces/IRIO_OnTrackCreator.h"
#include "TrkParameters/TrackParameters.h"

class AtlasDetectorID;

namespace Trk {

  class PrepRawData;
  class RIO_OnTrack;

  /** @class RIO_OnTrackCreator

      @brief general tool to converts clusters or driftcircles
      (Trk::PrepRawData) to fully calibrated hits (Trk::RIO_OnTrack)
      further use in track fits.

      This implementation is the technology-independent master tool
      which identifies the detector where the hit comes from (e.g.
      PixelCluster)
      and calls the appropriate tool to create e.g. PixelClusterOnTrack.
      The use of detector-specific tools is configured via job options.

      Both this tool and the detector-specific tools need a track
      hypothesis to make the conversion from Trk::PrepRawData to
      Trk:: RIO_OnTrack.
      This needs to be provided by the local pattern recognition
      or the track fitting tool.

      @author Wolfgang Liebig <http://consult.cern.ch/xwho/people/54608>
   */

  class RIO_OnTrackCreator final : public extends<AthAlgTool, IRIO_OnTrackCreator> {
   public:
    ///////////////////////////////////////////////////////////////////
    // Public methods:
    ///////////////////////////////////////////////////////////////////

    //! standard AlgTool constructor
    using base_class::base_class;
    //! virtual destructor
    virtual ~RIO_OnTrackCreator();

    //!get specific ROT tools and the AtlasIdHelper
    virtual StatusCode initialize() override;

    //! the master method for going from RIO to ROT.
    RIO_OnTrack* correct(const PrepRawData& rio,
                         const TrackParameters& pars,
                         const EventContext& ctx) const override final;

   private:
    //! Detector-specific helper tool, performing the actual calibration corrections for every InDet::PixelCluster
    ToolHandle<IRIO_OnTrackCreator> m_pixClusCor{this, "ToolPixelCluster", ""};
    //! Detector-specific helper tool, performing the actual calibration
    //! corrections for every InDet::SCT_Cluster
    ToolHandle<IRIO_OnTrackCreator> m_sctClusCor{this, "ToolSCT_Cluster", ""};
    //! Detector-specific helper tool, performing the actual calibration
    //! corrections for every InDet::TRT::DriftCircle
    ToolHandle<IRIO_OnTrackCreator> m_trt_Cor{this, "ToolTRT_DriftCircle", ""};
    //! Detector-specific helper tool, performing the actual calibration
    //! corrections for every Muon::MdtPrepData
    ToolHandle<IRIO_OnTrackCreator> m_muonDriftCircleCor{this, "ToolMuonDriftCircle", ""};
    //! Detector-specific helper tool, performing the actual calibration
    //! corrections for the remaining muon detector technologies: RPC, TGC, CSC,
    //! MM, sTGC.
    ToolHandle<IRIO_OnTrackCreator> m_muonClusterCor{this, "ToolMuonCluster",""};

    Gaudi::Property<std::string>m_mode{this, "Mode" ,"all" };   //!< flag: can be 'all', 'indet' or 'muon'
    //emum for the flag
    enum struct Mode {
      all = 0,
      indet = 1,
      muon = 2,
      invalid = 3
    };
    Mode m_enumMode = Mode::all;
  };

} // end of namespace

#endif // TRKTOOLS_RIOONTRACKCREATOR_H
