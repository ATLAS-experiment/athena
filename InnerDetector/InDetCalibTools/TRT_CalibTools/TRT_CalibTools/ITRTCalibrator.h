/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef TRT_CALIBTOOLS__ITRTCALIBRATOR_H
#define TRT_CALIBTOOLS__ITRTCALIBRATOR_H
/********************************************************************

NAME:     ITRTCalibrator
PACKAGE:  TRT_CalibTools

AUTHORS:  Johan Lundquist
CREATED:  March 2009

PURPOSE:  Fit R-t relation constants using histograms produced by a previous accumulation job

********************************************************************/

#include "CxxUtils/checker_macros.h"
#include "GaudiKernel/IAlgTool.h"

namespace Trk{
  class Track;
}
namespace TRT{
class TrackInfo;
}

static const InterfaceID IID_ITRTCalibrator("ITRTCalibrator", 1, 0);

// This tool is only called once in a job reconstructing only one event
class ATLAS_NOT_THREAD_SAFE ITRTCalibrator : virtual public IAlgTool {
    public:
        virtual bool fill(const Trk::Track* aTrack, TRT::TrackInfo* output) = 0;
        // uses thread-unsafe methods from Calibrator.
        virtual bool calibrate ATLAS_NOT_THREAD_SAFE () = 0;
        static const InterfaceID& interfaceID();
};

inline const InterfaceID& ITRTCalibrator::interfaceID()
{
	return IID_ITRTCalibrator;
}

#endif // TRT_CALIBTOOLS__ITRTCALIBRATOR_H

