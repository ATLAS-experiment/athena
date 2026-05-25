/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

// MuonMeanMDTdADCFillerTool.h,  Header file for class MuonMeanMDTdADCFillerTool
///////////////////////////////////////////////////////////////////

#ifndef MUONCOMBINEDEVALUATIONTOOLS_MUONMEANMDTDADCFILLERTOOL_H
#define MUONCOMBINEDEVALUATIONTOOLS_MUONMEANMDTDADCFILLERTOOL_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonCombinedToolInterfaces/IMuonMeanMDTdADCFiller.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRecHelperTools/IMuonEDMHelperSvc.h"
#include "StoreGate/ReadHandleKey.h"
#include "xAODEventInfo/EventInfo.h"

namespace Rec {

    /** @class MuonMeanMDTdADCFillerTool
       @brief return mean Number of ADC counts for MDT tubes on the track
     */

    class MuonMeanMDTdADCFillerTool : public extends<AthAlgTool, IMuonMeanMDTdADCFiller> {
        ///////////////////////////////////////////////////////////////////
        // Public methods:
        ///////////////////////////////////////////////////////////////////
    public:
        // Copy constructor:

        /// Constructor with parameters:
        using base_class::base_class;
        virtual ~MuonMeanMDTdADCFillerTool() = default;

        StatusCode initialize() override;

        /** return mean Number of ADC counts for MDT tubes on the track of muon (method will simply step down to the relevant track)*/
        double meanMDTdADCFiller(const xAOD::Muon& muon) const override;

        /** return mean Number of ADC counts for MDT tubes on the track */
        double meanMDTdADCFiller(const Trk::Track& track) const override;

    private:
        ServiceHandle<Muon::IMuonEDMHelperSvc> m_edmHelperSvc{this, "edmHelper", "Muon::MuonEDMHelperSvc/MuonEDMHelperSvc",
                                                              "Handle to the service providing the IMuonEDMHelperSvc interface"};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};
        SG::ReadHandleKey<xAOD::EventInfo> m_eventInfo{this, "EventInfo", "EventInfo", "event info"};
    };

}  // namespace Rec
#endif  //> !MUONCOMBINEDEVALUATIONTOOLS_MUONMEANMDTDADCFILLERTOOL_H
