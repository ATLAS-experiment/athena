// Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#ifndef FPGATRACKSIMACTSTRACKINSPECTIONTOOL_H
#define FPGATRACKSIMACTSTRACKINSPECTIONTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "ActsEvent/TrackContainer.h"


#include <deque>
#include <memory>
#include <map>
#include <vector>
#include <string>

namespace FPGATrackSim {
  class ActsTrackInspectionTool : public AthAlgTool {
  public:

    ActsTrackInspectionTool(const std::string&, const std::string&, const IInterface*);
    virtual ~ActsTrackInspectionTool() = default;

    virtual StatusCode initialize() override;

    struct FpgaActsTrack {
      struct Measurement {
        unsigned long identifier;
        std::string type;  // "Pixel" or "Strip"
        struct globalCoordinates {
          double x = 0;
          double y = 0;
          double z = 0;
        } coordinates = {};
        // Acts flags
        bool measurementFlag = false;
        bool outlierFlag = false;
        bool holeFlag = false;
        bool materialFlag = false;
        bool sharedHitFlag = false;
      };
      Acts::BoundVector parameters = {};
      std::deque<std::unique_ptr<Measurement>> trackMeasurements;
      double chi2 = 0.0;
      unsigned int ndof = 0u;
    };

    // Typedef within the class
    using fpgaActsEventTracks = std::vector<std::unique_ptr<FpgaActsTrack>>;

    fpgaActsEventTracks getActsTracks (const ActsTrk::TrackContainer& tracksContainer) const;
    std::string getPrintoutActsEventTracks(const fpgaActsEventTracks& tracks) const;
    std::string getPrintoutStatistics(const std::map<std::string, std::map<uint32_t,std::vector<uint32_t>> >& tracksForAllEvents) const;
  private:
    mutable std::vector<std::unique_ptr<FpgaActsTrack>> m_actsTracks ATLAS_THREAD_SAFE;
  };
}

//typedef for all acts tracks of one event
typedef FPGATrackSim::ActsTrackInspectionTool::fpgaActsEventTracks FPGATrackSimActsEventTracks;

#endif