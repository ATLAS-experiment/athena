/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONALIGNERRORTOOL_ALIGNMENTERRORTOOL_H
#define MUONALIGNERRORTOOL_ALIGNMENTERRORTOOL_H

#include <iosfwd>
#include <string>
#include <boost/regex.hpp>

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "GaudiKernel/ToolHandle.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "MuonAlignErrorBase/AlignmentDeviation.h"
#include "MuonCalibITools/IIdToFixedIdTool.h"
#include "MuonAlignmentData/MuonAlignmentErrorData.h"  // for accessing info from the DB
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "TrkToolInterfaces/ITrkAlignmentDeviationTool.h"

namespace Trk {
    class RIO_OnTrack;
}

namespace MuonAlign {

    class AlignmentErrorTool : public Trk::ITrkAlignmentDeviationTool, virtual public AthAlgTool {
    public:

     AlignmentErrorTool(const std::string&, const std::string&,
                        const IInterface*);
      ~AlignmentErrorTool() override = default;

      StatusCode initialize() override;
      void makeAlignmentDeviations(
         const Trk::Track& track,
         std::vector<Trk::AlignmentDeviation*>& deviations) const override;

    private:
        ToolHandle<MuonCalib::IIdToFixedIdTool> m_idTool{this, "idTool", "MuonCalib::IdToFixedIdTool"};
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        // Struct for per-Station Deviations Information //
        struct deviationSummary_t {
            // SYSTEMATIC UNCERTAINTIES
            double translation{0.0};
            double rotation{0.0};
            // RULE
            boost::regex stationName{""};
            boost::regex multilayer{""};
            // SET OF HITS SATISFYING THE RULE
            std::vector<const Trk::RIO_OnTrack*> hits{};
            // USEFUL NUMBERS
            Amg::Vector3D sumP{Amg::Vector3D::Zero()};
            Amg::Vector3D sumU{Amg::Vector3D::Zero()};
            Amg::Vector3D sumV{Amg::Vector3D::Zero()};
            double sumW2{0.0};
        };

        SG::ReadCondHandleKey<MuonAlignmentErrorData> m_readKey{this, "ReadKey", "MuonAlignmentErrorData", "Key of MuonAlignmentErrorData"};
    };
}  // namespace MuonAlign

#endif
