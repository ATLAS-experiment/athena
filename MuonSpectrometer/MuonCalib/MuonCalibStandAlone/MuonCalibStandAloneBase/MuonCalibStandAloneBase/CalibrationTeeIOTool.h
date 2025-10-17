/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef CalibrationTeeIOTool_H
#define CalibrationTeeIOTool_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ToolHandle.h"
#include "MuonCalibStandAloneBase/ICalibrationIOTool.h"
#include <map>
#include <string>

namespace MuonCalib {

    /** @class CalibrationTeeIOTool
    Wites to two calibtaion IO Tools. Reads from the first one.
    */

    class CalibrationTeeIOTool : public AthAlgTool, virtual public ICalibrationIOTool {
    public:
        /** constructor*/
        CalibrationTeeIOTool(const std::string &t, const std::string &n, const IInterface *p);
        /** initialize function */
        StatusCode initialize() override;
        /** interface functions */
        StatusCode WriteT0(const MdtTubeFitContainer *t0_output, const NtupleStationId &station_id, int iov_start, int iov_end) override;
        StatusCode WriteRt(const RtCalibrationOutput *rt_relation, std::shared_ptr<const IRtResolution> resolution,
                           const NtupleStationId &station_id, int iov_start, int iov_end, bool real_rt, bool real_resolution) override;
        StatusCode LoadT0(std::map<NtupleStationId, MdtStationT0Container *> &t0s, int iov_id) override;
        StatusCode LoadRt(std::map<NtupleStationId, IRtRelation *> &rts, std::map<NtupleStationId, IRtResolution *> &res,
                          int iov_id) override;

    private:
        //! two calibration IO Tools
        ToolHandle<ICalibrationIOTool> m_tool1{this, "IOTool1", "MuonCalib::CalibrationDummyIOTool"};
        ToolHandle<ICalibrationIOTool> m_tool2{this, "IOTool2", "MuonCalib::CalibrationDummyIOTool"};
    };

}  // namespace MuonCalib
#endif
