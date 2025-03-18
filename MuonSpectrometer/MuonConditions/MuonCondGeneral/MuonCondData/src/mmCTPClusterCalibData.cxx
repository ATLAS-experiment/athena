#include "MuonCondData/mmCTPClusterCalibData.h"

#include "GaudiKernel/SystemOfUnits.h"



namespace Muon{

mmCTPClusterCalibData::mmCTPClusterCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"mmCTPClusterCalibData"},
    m_idHelperSvc{idHelperSvc} {}

mmCTPClusterCalibData::CTPParameters::CTPParameters(std::array<double, 2>&& pars): m_pars (std::move(pars)){}

StatusCode mmCTPClusterCalibData::storeConstants(const Identifier& gasGapIdentifier,
                                             CTPParameters&& newConstants) {    

    if (m_database.find(gasGapIdentifier) != m_database.end()) {
        ATH_MSG_ERROR("The drift velocity calibration constants for gas gap "<<m_idHelperSvc->toStringGasGap(gasGapIdentifier) <<" already exist. Overwriting is not allowed");
        return StatusCode::FAILURE;
    }

    //Retrieve the calibration constants, need at least 2 parameters for currently implemented parametrisation
    if (newConstants.pars().size() < 2) {
        ATH_MSG_ERROR("The error calibration constants for stName:" << m_idHelperSvc->mmIdHelper().stationName(gasGapIdentifier) << 
                                                           " stEta: " << m_idHelperSvc->mmIdHelper().stationEta(gasGapIdentifier)  << 
                                                           " stPhi: " << m_idHelperSvc->mmIdHelper().stationPhi(gasGapIdentifier) << 
                                                           " ml: "    << m_idHelperSvc->mmIdHelper().multilayer(gasGapIdentifier) << 
                                                           " gg: "    << m_idHelperSvc->mmIdHelper().gasGap(gasGapIdentifier)  <<
                                                           " are incomplete. 2 parameters are required!");

        return StatusCode::FAILURE;
    }


    m_database[gasGapIdentifier] = std::move(newConstants);

    return StatusCode::SUCCESS;
}

double mmCTPClusterCalibData::getCTPCorrectedDriftVelocity(const Identifier& gasGapIdentifier, const double theta) const {
    //Identifier: There is no pcb segmentation for these corrections, stored with pcb = 1 as default! Therefore expects pcb = 1 
    //Theta: Expected in degrees
    //Drift velocity: will be fully taken from the parametrisation from calibration file 

    //If not present in the map return original value
    if( m_database.find(gasGapIdentifier) == m_database.end()) {
        ATH_MSG_ERROR("There's no drift velocity calibration available for gasGap " << m_idHelperSvc->toStringGasGap(gasGapIdentifier)<< " size of the calib map is: " << m_database.size() );
    }

    ATH_MSG_VERBOSE( "Retriving drift velocity for stName" << m_idHelperSvc->toStringGasGap(gasGapIdentifier) );

    //Conversions of incident angle
    double trf_theta_in_degrees = (theta > 90) ? (180 - theta) : theta;
    double trf_theta_in_radians = trf_theta_in_degrees * Gaudi::Units::deg;
    double tan_theta            = std::tan(trf_theta_in_radians);

    //Parametrisation was derived as delta_residual = x_true - x_charge_weighted = (time_true - time_charge_weighted) * v_drift * tan(theta)
    //The fit used is a linear fit from data obtained for VMM 17 to VMM 95. Data outside this fit range were excluted due to problems with the magnetic field at the inner and outer PCBs, see figure 11.8 on page 135 of https://cds.cern.ch/record/2839930/files/CERN-THESIS-2021-354.pdf. Therefore, it is expected, that x^2 and x^3 are 0.
    //Parametrisation from Stefanie Gotz and Fabian Vogel, for more details see:
    //https://indico.cern.ch/event/1501492/contributions/6321472/attachments/3013528/5314031/SpatialResolution14.pdf#page=5
    //https://indico.cern.ch/event/1458120/contributions/6138857/attachments/2942828/5170893/MuonWeekOctober24FV1.pdf
    double vDrift = m_database.at(gasGapIdentifier).pars()[0] + ((m_database.at(gasGapIdentifier).pars()[1])*trf_theta_in_degrees); 
    
    //Remove tantheta and always return positive drift velocity!

    vDrift =  (tan_theta != 0 ) ?   std::fabs(vDrift/tan_theta) : std::fabs(vDrift);


    ATH_MSG_VERBOSE( "New drift velocity: " << vDrift  << " for theta: " << trf_theta_in_degrees << " degrees" );

    return vDrift;
}

}//end for Muon namespace
