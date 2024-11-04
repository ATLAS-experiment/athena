/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// TRTTrackHoleSearchTool.h
// author: Ryan D. Reece <ryan.reece@cern.ch>
// created: Jan 2010


#ifndef TRT_TrackHoleSearch_TRTTrackHoleSearchTool_h
#define TRT_TrackHoleSearch_TRTTrackHoleSearchTool_h

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "GaudiKernel/ServiceHandle.h"
#include "TrkToolInterfaces/ITrackHoleSearchTool.h"
#include "TrkParameters/TrackParameters.h"
#include "TrkExInterfaces/IExtrapolator.h"
#include "InDetConditionsSummaryService/IInDetConditionsSvc.h"

#include "CLHEP/Units/SystemOfUnits.h"

#include <atomic>
#include <string>
#include <vector>
#include <fstream>

namespace Trk
{
	class CylinderSurface;
	class Track;
	class Surface;
}

class TRT_ID;

class TRTTrackHoleSearchTool : public Trk::ITrackHoleSearchTool, public AthAlgTool
{
 public:
	TRTTrackHoleSearchTool(const std::string& type, const std::string& name, const IInterface* parent);

	StatusCode initialize();
	StatusCode finalize();

	/** Input : track, partHyp
	    Output: Changes in information
	    This method fills the fields relevant to the hole counts in the vector information. These fields should be initialised to 0. 
	    The relevant indices are specified by the enumeration in Tracking/TrkEvent/TrkTrackSummary.
	    If problems occur, the information counters are reset to -1 flagging them as not set.
	    The parthyp argument is relevant for the extrapolation steps in the hole search.
	*/
	void countHoles(const Trk::Track& track, 
	                std::vector<int>& information ,
	                const Trk::ParticleHypothesis partHyp = Trk::pion) const;
    
	/** Input : track, parthyp
	    Return: A DataVector containing pointers to TrackStateOnSurfaces which each represent an identified hole on the track.
	    The parthyp argument is relevant for the extrapolation steps in the hole search.
	    Attention: This is a factory, ownership of the return vector is passed to the calling method.
	*/
	const Trk::TrackStates* getHolesOnTrack(const Trk::Track& track, 
	                                                                  const Trk::ParticleHypothesis partHyp = Trk::pion) const;
    
	/** Input : track, parthyp
	    Return: A pointer to a new Trk::Track which containes the information of the input track plus the tsos of the identified holes
	    The parthyp argument is relevant for the extrapolation steps in the hole search.
	    Attention: This is a factory, ownership of the return track is passed to the calling method.
	*/
	const Trk::Track* getTrackWithHoles(const Trk::Track& track, 
	                                    const Trk::ParticleHypothesis partHyp = Trk::pion) const;
        
    
	/** Input : track, parthyp
	    Return: A pointer to a new Trk::Track which containes the information of the input track plus the tsos of the identified holes or outliers
	    The parthyp argument is relevant for the extrapolation steps in the hole search.
	    Attention: This is a factory, ownership of the return track is passed to the calling method.
	*/
	const Trk::Track* getTrackWithHolesAndOutliers(const Trk::Track& track, 
	                                               const Trk::ParticleHypothesis partHyp = Trk::pion) const;
    
          
 private:
        // configurables
        //----------------------------------
        ToolHandle<Trk::IExtrapolator> m_extrapolator
          {this, "extrapolator", "Trk::Extrapolator"};
        ServiceHandle<IInDetConditionsSvc> m_conditions_svc
          {this, "conditions_svc", "TRT_ConditionsSummarySvc"};
        BooleanProperty m_use_conditions_svc{this, "use_conditions_svc", true};
        // barrel 1075.0, EC 1010.0
        FloatProperty m_outer_radius{this, "outer_radius", 1075.0*CLHEP::mm};
        // barrel 715.0, EC 2715.0
        FloatProperty m_max_z{this, "max_z", 2715.0*CLHEP::mm};
        IntegerProperty m_max_trailing_holes{this, "max_trailing_holes", 1}; // only used if not end_at_last_trt_hit
        BooleanProperty m_begin_at_first_trt_hit{this, "begin_at_first_trt_hit", false}; // if not, extrapolate from last Si hit
        BooleanProperty m_end_at_last_trt_hit{this, "end_at_last_trt_hit", false}; // if not, continue hole search to the edge of the TRT
        BooleanProperty m_bcheck{this, "bcheck", false};
        BooleanProperty m_do_dump_bad_straw_log{this, "do_dump_bad_straw_log", false};
        FloatProperty m_locR_cut{this, "locR_cut", -1.}; // 1.4*CLHEP::mm // negative means no cut
        FloatProperty m_locR_sigma_cut{this, "locR_sigma_cut", -1.};
        FloatProperty m_locZ_cut{this, "locZ_cut", 5.0*CLHEP::mm};
       
        // private data
        //----------------------------------
        mutable std::atomic_bool m_has_been_called{false};
        const TRT_ID* m_TRT_ID{nullptr};
        Trk::CylinderSurface* m_trt_outer_surf{nullptr};

        // private methods
        //----------------------------------
        int extrapolateBetweenHits(const Trk::TrackParameters* start_parameters,
                                   const Trk::Surface& end_surf,
                                   Trk::TrackStates* holes,
                                   const Trk::ParticleHypothesis partHyp = Trk::pion) const;

        void dump_bad_straw_log() const;

        Trk::TrackStates::const_iterator
        find_first_trt_hit(const Trk::TrackStates& track_states) const;

        Trk::TrackStates::const_iterator
        find_last_hit_before_trt(const Trk::TrackStates& track_states) const;

        const Trk::Track* addHolesToTrack(const Trk::Track& track,
                                          const Trk::TrackStates* holes) const;
};

#endif // TRT_TrackHoleSearch_TRTTrackHoleSearchTool_h
