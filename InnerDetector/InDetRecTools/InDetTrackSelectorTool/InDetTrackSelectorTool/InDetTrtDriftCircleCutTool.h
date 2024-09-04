/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef InDetTrackSelectorTool_InDetTrtDriftCircleCutTool_H
#define InDetTrackSelectorTool_InDetTrtDriftCircleCutTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "InDetRecToolInterfaces/ITrtDriftCircleCutTool.h"
#include "TRT_ConditionsData/ActiveFraction.h"
#include "StoreGate/ReadCondHandleKey.h"

/**
 * A tool to be used for trt segment pre-selection
 *
 * Thomas Koffas <Thomas.Koffas@cern.ch>
 * April 2009
 */


namespace InDet{

  class InDetTrtDriftCircleCutTool : virtual public ITrtDriftCircleCutTool, public AthAlgTool  
    {
      
    public:
  
      virtual StatusCode initialize() override;
      
      InDetTrtDriftCircleCutTool(const std::string& t, const std::string& n, const IInterface*  p);
      
      virtual ~InDetTrtDriftCircleCutTool();
      
      /** @brief Minimum number of drift circles using the track scoring tool */
      virtual int minNumberDCs(const Trk::TrackParameters*) const override;
      
    private:
      
      SG::ReadCondHandleKey<TRTCond::ActiveFraction> m_strawReadKey
      {this,"ActiveReadKey","ActiveFraction","ActiveFraction in-key"};
      /** Properties for track selection:all cuts are ANDed */
      IntegerProperty m_minOffset
	{this, "MinOffsetDCs", 0, "Minimum number of TRT drit circles required"};
      BooleanProperty m_param
	{this, "UseNewParameterization", false,
	 "Use the new or the old parameterization"};
      BooleanProperty m_useTRT
	{this, "UseActiveFractionSvc", true,
	 "Use the TRT active fraction services to correct for dead straws"};

    }; //end of class definitions
} //end of namespace

#endif
