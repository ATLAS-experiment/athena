/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MUONSPACEPOINTCALIBRATOR_SPACEPOINTCALIBRATOR_H
#define MUONSPACEPOINTCALIBRATOR_SPACEPOINTCALIBRATOR_H

#include "MuonRecToolInterfacesR4/ISpacePointCalibrator.h"


#include "AthenaBaseComps/AthAlgTool.h"


#include "MuonSpacePoint/SpacePoint.h"
#include "MuonSpacePoint/CalibratedSpacePoint.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"

#include "MdtCalibInterfaces/IMdtCalibrationTool.h"
#include "ActsGeometryInterfaces/ActsGeometryContext.h"
#include "MuonReadoutGeometryR4/MuonDetectorManager.h"

#include "GaudiKernel/PhysicalConstants.h"
namespace MuonR4{
    /*** @brief Implementation of the space point calibrator interface */
    class SpacePointCalibrator : public extends<AthAlgTool, ISpacePointCalibrator> {
        public:
            SpacePointCalibrator(const std::string& type, 
                                const std::string &name, 
                                const IInterface* parent);

            StatusCode initialize() override final;

            CalibSpacePointPtr calibrate(const EventContext& ctx,
                                         const SpacePoint* spacePoint,
                                         const Amg::Vector3D& seedPosInChamb,
                                         const Amg::Vector3D& seedDirInChamb,
                                         const double timeDelay) const override final;
            
            CalibSpacePointPtr calibrate(const EventContext& ctx,
                                         const CalibratedSpacePoint& spacePoint,
                                         const Amg::Vector3D& seedPosInChamb,
                                         const Amg::Vector3D& seedDirInChamb,
                                         const double timeDelay) const override final;
            
            
            CalibSpacePointVec calibrate(const EventContext& ctx,
                                         const std::vector<const SpacePoint*>& spacePoints,
                                         const Amg::Vector3D& seedPosInChamb,
                                         const Amg::Vector3D& seedDirInChamb,
                                         const double timeDelay) const override final;

            CalibSpacePointVec calibrate(const EventContext& ctx,
                                         CalibSpacePointVec&& spacePoints,
                                         const Amg::Vector3D& seedPosInChamb,
                                         const Amg::Vector3D& seedDirInChamb,
                                         const double timeDelay) const override final;

            double driftVelocity(const EventContext& ctx,
                                 const CalibratedSpacePoint& spacePoint) const override final;
            double driftAcceleration(const EventContext& ctx,
                                     const CalibratedSpacePoint& spacePoint) const override final;
        private:
            /// access to the ACTS geometry context 
            SG::ReadHandleKey<ActsGeometryContext> m_geoCtxKey{this, "AlignmentKey", "ActsAlignment", "cond handle key"}; 

            ServiceHandle<Muon::IMuonIdHelperSvc> m_idHelperSvc{this, "MuonIdHelperSvc", "Muon::MuonIdHelperSvc/MuonIdHelperSvc"};

            ToolHandle<IMdtCalibrationTool> m_mdtCalibrationTool{this, "MdtCalibrationTool", ""};

            const MuonGMR4::MuonDetectorManager* m_detMgr{nullptr};

            /** Assumed propagation velocity of the muon through the detector. Needs to be replaced by the
             *  proper time estimate once the calibrator is exposed to the Acts propagator
            */
            Gaudi::Property<double> m_muonPropSpeed{this, "PropagationSpeed", 1./ Gaudi::Units::c_light };

            /** How fast does an electron signal travel along an rpc strip  */
            Gaudi::Property<double> m_rpcSignalVelocity {this, "rpcSignalVelocity", 0.5 * Gaudi::Units::c_light,
                                                         "Propagation speed of the signal inside the rpc strip"}; 
            /*** Resolution of the rpc time measurement  */
            Gaudi::Property<double> m_rpcTimeResolution{this, "rpcTimeResolution", 0.6 * Gaudi::Units::nanosecond,
                                                          "Estimated time resolution of the strip readout"};
    };

}
#endif
