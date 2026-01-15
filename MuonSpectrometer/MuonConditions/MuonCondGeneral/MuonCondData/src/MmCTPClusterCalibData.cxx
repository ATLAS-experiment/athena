/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#include "MuonCondData/MmCTPClusterCalibData.h"
#include "GaudiKernel/SystemOfUnits.h"
namespace Muon{

MmCTPClusterCalibData::MmCTPClusterCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"MmCTPClusterCalibData"},
    m_idHelperSvc{idHelperSvc} {
    m_database.resize(m_idHelperSvc->mmIdHelper().detectorElement_hash_max() * 4);
}



std::uint32_t MmCTPClusterCalibData::convertHash(const Identifier& gasGapId) const {
    IdentifierHash detHash{};
    const MmIdHelper& idHelper{m_idHelperSvc->mmIdHelper()};
    if(idHelper.get_detectorElement_hash(gasGapId, detHash)){
        ATH_MSG_WARNING(__func__<<"() - "<<m_idHelperSvc->toString(gasGapId)<<" is not a valid Mm identifier");
        return -1;
    }
    return static_cast<unsigned>(detHash)*4 + (idHelper.gasGap(gasGapId) -1); 

}
StatusCode MmCTPClusterCalibData::storeConstants(const Identifier& gasGapIdentifier,
                                                 CTPParameters&& newConstants) {    

    auto& slot = m_database.at(convertHash(gasGapIdentifier));
    if (slot) {
        ATH_MSG_ERROR("The drift velocity calibration constants for gas gap "<<m_idHelperSvc->toStringGasGap(gasGapIdentifier) 
                    <<" already exist. Overwriting is not allowed");
        return StatusCode::FAILURE;
    }
    slot = std::make_unique<CTPParameters>(std::move(newConstants));
    return StatusCode::SUCCESS;
}

double MmCTPClusterCalibData::getCTPCorrectedDriftVelocity(const Identifier& gasGapIdentifier, const double theta) const {
    //Identifier: There is no pcb segmentation for these corrections, stored with pcb = 1 as default! Therefore expects pcb = 1 
    //Theta: Expected in degrees
    //Drift velocity: will be fully taken from the parametrisation from calibration file 

    //If not present in the map return original value
    const auto& calibPars = m_database.at(convertHash(gasGapIdentifier));
    if(!calibPars) {
        ATH_MSG_WARNING("There's no drift velocity calibration available for gasGap " 
                << m_idHelperSvc->toStringGasGap(gasGapIdentifier)<<".");
        return std::numeric_limits<double>::max();
    }

    ATH_MSG_VERBOSE( "Retrieving drift velocity for stName" << m_idHelperSvc->toStringGasGap(gasGapIdentifier) );

    //Conversions of incident angle
    double trf_theta_in_degrees = (theta > 90.) ? (180. - theta) : theta;
    double tan_theta            = std::tan(trf_theta_in_degrees*Gaudi::Units::deg);

    //Parametrisation was derived as delta_residual = x_true - x_charge_weighted = (time_true - time_charge_weighted) * v_drift * tan(theta)
    //The fit used is a linear fit from data obtained for VMM 17 to VMM 95. Data outside this fit range were excluted due to problems with the magnetic field at the inner and outer PCBs, see figure 11.8 on page 135 of https://cds.cern.ch/record/2839930/files/CERN-THESIS-2021-354.pdf. Therefore, it is expected, that x^2 and x^3 are 0.
    //Parametrisation from Stefanie Gotz and Fabian Vogel, for more details see:
    //https://indico.cern.ch/event/1501492/contributions/6321472/attachments/3013528/5314031/SpatialResolution14.pdf#page=5
    //https://indico.cern.ch/event/1458120/contributions/6138857/attachments/2942828/5170893/MuonWeekOctober24FV1.pdf
    double vDrift = (*calibPars)[0] + (*calibPars)[1] * trf_theta_in_degrees;
    //Remove tantheta and always return positive drift velocity!

    vDrift =  (tan_theta != 0 ) ? std::abs(vDrift/tan_theta) : std::abs(vDrift);


    ATH_MSG_VERBOSE( "New drift velocity: " << vDrift  << " for theta: " << trf_theta_in_degrees << " degrees" );

    return vDrift;
}

}//end for Muon namespace
