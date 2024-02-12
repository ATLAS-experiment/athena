/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_CALIBALGS_TRTCALIBRATIONMGR_H
#define TRT_CALIBALGS_TRTCALIBRATIONMGR_H

#include "TRT_CalibTools/ITRTCalibrator.h"
#include <string>
#include <vector>
#include "AthenaBaseComps/AthAlgorithm.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "AthenaKernel/IAthenaOutputStreamTool.h"
#include "StoreGate/DataHandle.h"
#include "xAODEventInfo/EventInfo.h"
#include "xAODTracking/VertexContainer.h"
#include "TrkTrack/TrackCollection.h"
#include "TrkTrack/Track.h"
#include "StoreGate/ReadHandleKey.h"
#include "TRT_ConditionsData/RtRelationMultChanContainer.h"
#include "TRT_ConditionsData/StrawT0MultChanContainer.h"
#include "CxxUtils/checker_macros.h"


namespace TRT{
  class TrackInfo;
}

namespace Trk{
 class ITrackSelectorTool;
 class ITrackFitter;
}
class IFillAlignTrkInfo;
class IAccumulator;
class IFitTool;


/**

      @class TRTCalibrationMgr

      This algorithm controls the flow of the TRT calibration.

Documentation being updated (Jan 2009)

The calculation of t0 from the timeresidual histogram is made in the
following way: First it is checked where the maximum bin is located.
If its absolute value is bigger then 5 ns NO fit is made, but the
histogram mean value is used for Dt0 instead (this was just taken from
previous versions and I have never really questioned the reason for
it). If the highest bin is lower than 5 ns a normal gaussian fit is
made and Dt0 is set to the mean of that. The new t0 is then the old t0
+ Dt0.


      @author Chafik, Johan, Alex

*/

// TRTCalibrator is called by TRTCalibrationMgr and is not thread-safe.
// But the job calling TRTCalibrator reconstructs only one event, in order to get access to Athena tools. This job never run multi-threaded.
class ATLAS_NOT_THREAD_SAFE TRTCalibrationMgr : public AthAlgorithm
{

public:
    TRTCalibrationMgr(const std::string &name, ISvcLocator *pSvcLocator);
    ~TRTCalibrationMgr(void);

    typedef TRTCond::RtRelationMultChanContainer RtRelationContainer;
    typedef TRTCond::StrawT0MultChanContainer StrawT0Container;

    virtual StatusCode initialize(void) override;
    virtual StatusCode execute(void) override;
    virtual StatusCode finalize(void) override;
    StatusCode streamOutCalibObjects();

private:

    // Tools for the algorithm
    ToolHandleArray<IFillAlignTrkInfo> m_TrackInfoTools ;
    PublicToolHandleArray<ITRTCalibrator> m_TRTCalibTools {this, "TRTCalibrator", {}, ""};
    ToolHandleArray<IFitTool> m_FitTools;
    ToolHandle<Trk::ITrackFitter> m_trackFitter;
    ToolHandle<Trk::ITrackSelectorTool> m_trackSelector; //!< Tool handle to the Trk::ITrackSelectorTool
    ToolHandle<IAthenaOutputStreamTool> m_streamer; //!< OutputStreamTool

    // Gaudi properties
    Gaudi::Property<bool> m_dorefit       {this, "DoRefit"       , true , "Does a Re-Fit"};
    Gaudi::Property<bool> m_docalibrate   {this, "DoCalibrate"   , false, "Does the calibration"};
    Gaudi::Property<bool> m_writeConstants{this, "WriteConstants", false, "Write out the calibration constants"};

    int m_ntrk = 0;
    
    unsigned int m_max_ntrk;
    std::string m_par_rtcontainerkey; //"/TRT/Calib/RT"
    std::string m_par_t0containerkey; //"/TRT/Calib/T0"

    // ReadHandleKeys
    SG::ReadHandleKey<xAOD::VertexContainer> m_verticesKey   {this, "VerticesKey"     , "PrimaryVertices"    , "RHK for primary vertices"  };
    SG::ReadHandleKey<xAOD::EventInfo>       m_EventInfoKey  {this, "EventInfoKey"    , "EventInfo"          , "RHK for xAOD::EventInfo"   };
    SG::ReadHandleKey<TrackCollection>       m_TrkCollection {this, "TrkCollectionKey", "CombinedInDetTracks", "RHKs for track collections"};
};

#endif // TRT_CALIBALGS_TRTCALIBRATIONMGR_H
