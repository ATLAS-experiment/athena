/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

///////////////////////////////////////////////////////////////////
// Interface for MuonClusterOnTrack production
// (for CSC and RPC technologies)
///////////////////////////////////////////////////////////////////

#ifndef MuonClusterOnTrackCreator_H
#define MuonClusterOnTrackCreator_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "GaudiKernel/ServiceHandle.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "MuonRIO_OnTrack/MuonClusterOnTrack.h"
#include "MuonRecToolInterfaces/IMuonClusterOnTrackCreator.h"
#include "TrkParameters/TrackParameters.h"
#include "NSWCalibTools/INSWCalibTool.h"
#include "MMClusterization/IMMClusterBuilderTool.h"
#include "TrkPrepRawData/PrepRawDataCLASS_DEF.h"

/** @class MuonClusterOnTrackCreator
    This tool creates MuonClusterOnTrack objects using a given
    global postion prediction.
*/

namespace Muon {

    class MuonClusterOnTrackCreator : public AthAlgTool, virtual public IMuonClusterOnTrackCreator {
        // /////////////////////////////////////////////////////////////////
        // Public methods:
        // /////////////////////////////////////////////////////////////////

    public:
        MuonClusterOnTrackCreator(const std::string&, const std::string&, const IInterface*);
        virtual ~MuonClusterOnTrackCreator() = default;
        virtual StatusCode initialize() override;

        /** @brief Create new Muon::MuonClusterOnTrack from a Trk::PrepRawData and a predicted Trk::TrackParameter.
            @param RIO Trk::PrepRawData object to be calibrated
            @param GP  Predicted intersect position of the muon with the measurement plane
            @return a pointer to a new Muon::MuonClusterOnTrack object, zero if calibration failed.
            The ownership of the new Muon::MuonClusterOnTrack is passed to the client calling the tool
        */
        virtual MuonClusterOnTrack* createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP) const override;

        /** @brief Create new Muon::MuonClusterOnTrack from a Trk::PrepRawData and a prediction of the global position and direction.
            It is only implemented for the CSCs, for RPC and TGC Trk::PrepRawData the result is the same as for the routine without the
           direction.
            @param RIO Trk::PrepRawData object to be calibrated
            @param GP  Predicted intersect position of the muon with the measurement plane
            @param GD  Predicted direction at the intersect position of the muon with the measurement plane
            @return a pointer to a new Muon::MuonClusterOnTrack object, zero if calibration failed.
            The ownership of the new Muon::MuonClusterOnTrack is passed to the client calling the tool
        */
        virtual  MuonClusterOnTrack* createRIO_OnTrack(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP,
                                                       const Amg::Vector3D& GD) const override;

        /** @brief Create new Muon::MuonClusterOnTrack from a Trk::PrepRawData and the predicted Trk::TrackParameter at the measurement
           surface.
            @param RIO Trk::PrepRawData object to be calibrated
            @param TP  Predicted Trk::TrackParameter at the measurement surface
            @return a pointer to a new Muon::MuonClusterOnTrack object, zero if calibration failed.
            The ownership of the new Muon::MuonClusterOnTrack is passed to the client calling the tool
        */
        virtual MuonClusterOnTrack* correct(const Trk::PrepRawData& RIO, const Trk::TrackParameters& TP) const override;


        /** @brief Create new Muon::MuonClusterOnTrack from a Trk::PrepRawData and a prediction of the global position and direction.
            @param RIO Trk::PrepRawData object to be calibrated
            @param GP  Predicted intersect position of the muon with the measurement plane
            @param GD  Predicted direction at the intersect position of the muon with the measurement plane
            @return a pointer to a new Muon::MuonClusterOnTrack object, zero if calibration failed.
            The ownership of the new Muon::MuonClusterOnTrack is passed to the client calling the tool
        */
        virtual MuonClusterOnTrack* correct(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const override;
        
     
    private:
        ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

        ToolHandle<Muon::INSWCalibTool> m_calibToolNSW{this, "NSWCalibTool", ""};
        ToolHandle<Muon::IMMClusterBuilderTool> m_clusterBuilderToolMM{this, "MMClusterBuilder", "Muon::SimpleMMClusterBuilderTool/SimpleMMClusterBuilderTool"};

        MuonClusterOnTrack* calibratedClusterMMG(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const;
        MuonClusterOnTrack* calibratedClusterSTG(const Trk::PrepRawData& RIO, const Amg::Vector3D& GP, const Amg::Vector3D& GD) const;
        
        Gaudi::Property<bool> m_doFixedErrorTgcEta{this, "DoFixedErrorTgcEta", false};
        Gaudi::Property<bool> m_doFixedErrorRpcEta{this, "DoFixedErrorRpcEta", false};
        Gaudi::Property<bool> m_doFixedErrorTgcPhi{this, "DoFixedErrorTgcPhi", false};
        Gaudi::Property<bool> m_doFixedErrorRpcPhi{this, "DoFixedErrorRpcPhi", false};
        Gaudi::Property<double> m_fixedErrorTgcEta{this, "FixedErrorTgcEta", 5.};
        Gaudi::Property<double> m_fixedErrorRpcEta{this, "FixedErrorRpcEta", 5.};
        Gaudi::Property<double> m_fixedErrorTgcPhi{this, "FixedErrorTgcPhi", 5.};
        Gaudi::Property<double> m_fixedErrorRpcPhi{this, "FixedErrorRpcPhi", 5.};
    };
}  // namespace Muon
#endif  // MuonClusterOnTrackCreator_H
