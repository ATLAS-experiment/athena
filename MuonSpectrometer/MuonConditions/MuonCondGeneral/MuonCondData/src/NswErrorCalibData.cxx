#include "MuonCondData/NswErrorCalibData.h"
#include <sstream>
#include "GeoModelKernel/throwExcept.h"

using errorParametrizer = NswErrorCalibData::errorParametrizer;
using Input = NswErrorCalibData::Input;
namespace{
    inline double evalPoly(const double val, const std::vector<double>& pars) {
        double result{0.};
        for (size_t c = 0 ; c < pars.size(); ++c) {
            result+=pars[c] * std::pow(val, c);
        }
        return result;
    }
    inline double scaleErrorAndAddSyst(const double val, const std::vector<double>& pars) {
        return std::hypot(pars[0], pars[1]*val);   
    }
        
}

/**********************************************************
 **********************************************************               
 **
 **             Resolution model table
 **
 ************************************************************
 ************************************************************/
errorParametrizer NswErrorCalibData::getParametrizer(const std::string& funcName) {
    if (funcName == "tanThetaPolynomial") {
        return [](const Input & input, const std::vector<double>& pars){
            return evalPoly(std::tan(input.locTheta), pars);
        };
    } else if (funcName == "thetaPolynomial") {
        return [](const Input & input, const std::vector<double>& pars){
            return evalPoly(input.locTheta, pars);
        };
    } else if (funcName == "scaleErrorAndAddSyst"){
        return [](const Input& input, const std::vector<double>& pars){
            return scaleErrorAndAddSyst(input.clusterError, pars);
        };
    }
    /// Return a surprise box if the function is unknown
    return [funcName](const Input&, const std::vector<double>& ) {
        std::stringstream except_str{};
        except_str<<"NswErrorCalibData::parametrizer() - The function '"<<funcName<<"' is unknown.";
        except_str<<"Please check"<<__FILE__<<" for the set of valid function names. ";
        THROW_EXCEPTION(except_str.str());
        return 0.;
    };
}


bool operator<(const NswErrorCalibData::ErrorConstants& a, const NswErrorCalibData::ErrorIdentifier& b) {
    if (a.author() != b.clusAlgAuthor) return a.author() < b.clusAlgAuthor;
    return a.maxStrip() < b.strip;
}
bool operator<(const NswErrorCalibData::ErrorIdentifier& a, const NswErrorCalibData::ErrorConstants& b) {
    if (a.clusAlgAuthor != b.author()) return a.clusAlgAuthor < b.author();
    return a.strip < b.minStrip();
}
bool NswErrorCalibData::ErrorConstants::operator<(const ErrorConstants& other) const {
    if (m_clusAlgAuthor != other.m_clusAlgAuthor) {
        return m_clusAlgAuthor < other.m_clusAlgAuthor;
    }
    return m_stripMax < other.m_stripMin;
}

NswErrorCalibData::ErrorConstants::ErrorConstants(const std::string& funcName, uint8_t author,
                                             uint16_t minStrip, uint16_t maxStrip,
                                             std::vector<double>&& pars):
        m_evalFunc{getParametrizer(funcName)},
        m_clusAlgAuthor (author),
        m_stripMin (minStrip),
        m_stripMax (maxStrip),
        m_pars (std::move(pars))
{
}

uint16_t NswErrorCalibData::ErrorConstants::minStrip() const { return m_stripMin; }
uint16_t NswErrorCalibData::ErrorConstants::maxStrip() const { return m_stripMax; }
uint8_t NswErrorCalibData::ErrorConstants::author() const { return m_clusAlgAuthor; }
double NswErrorCalibData::ErrorConstants::clusterUncertainty(const Input& clustInfo) const {
    return m_evalFunc(clustInfo, m_pars);
}
/***********************************************************
 *  
 *  Implementation of the NswErrorCalibData conditions object
 * 
 **********************************************************/

NswErrorCalibData::NswErrorCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc):
    AthMessaging{"NswErrorCalibData"},
    m_idHelperSvc{idHelperSvc} {

}
double NswErrorCalibData::clusterUncertainty(const Input& clustInfo) const {
    ErrorMap::const_iterator nswLayerItr = m_database.find(m_idHelperSvc->gasGapId(clustInfo.stripId));
    if (nswLayerItr == m_database.end()) {
        ATH_MSG_WARNING("There's no error calibration available for gasGap "
                      << m_idHelperSvc->toStringGasGap(clustInfo.stripId)<<".");
        return -1.;    
    }
    ErrorIdentifier errorId{};
    if (m_idHelperSvc->isMM(clustInfo.stripId)) {
        errorId.strip = m_idHelperSvc->mmIdHelper().channel(clustInfo.stripId);
    } else if (m_idHelperSvc->issTgc(clustInfo.stripId)) {
        errorId.strip = m_idHelperSvc->stgcIdHelper().channel(clustInfo.stripId);
    }
    errorId.clusAlgAuthor = clustInfo.clusterAuthor;
    const ErrorConstantsSet& errorsInLay{nswLayerItr->second};
    const ErrorConstantsSet::const_iterator layConstItr = errorsInLay.find(errorId);
    if (layConstItr != errorsInLay.end()) {
        const double uncert = layConstItr->clusterUncertainty(clustInfo);
        if (uncert <= 0.) {
            ATH_MSG_WARNING("Uncertainty of channel "<<m_idHelperSvc->toString(clustInfo.stripId)
                          <<" is smaller than zero ("<<uncert<<").  theta: "<<clustInfo.locTheta
                          <<", eta: "<<(-std::log(std::tan(clustInfo.locTheta/2)))
                          <<", phi: "<<clustInfo.locPhi<<", cluster size: "<<clustInfo.clusterSize 
                          << ", author " << static_cast<uint>(clustInfo.clusterAuthor)
                          << ", pars " << layConstItr->pars());
        }
        return uncert;
    }
    ATH_MSG_WARNING("No calibration constants were stored for channel "<<m_idHelperSvc->toString(clustInfo.stripId)
                  <<", cluster Author: "<<static_cast<int>(clustInfo.clusterAuthor));
    return 0.;
}
StatusCode NswErrorCalibData::storeConstants(const Identifier& gasGapId,
                                             ErrorConstants&& newConstants) {    
    /// Check that min strip is actually smaller than max strip
    if (newConstants.minStrip() > newConstants.maxStrip()) {
        ATH_MSG_ERROR("The constants for gas gap"<<m_idHelperSvc->toStringGasGap(gasGapId)
                    <<" have an invalid strip range"<<newConstants.minStrip()<<" to "<<newConstants.maxStrip());
        return StatusCode::FAILURE;
    }    
    ErrorConstantsSet& constants{m_database[gasGapId]};
    if (!constants.insert(std::move(newConstants)).second) {
        ATH_MSG_ERROR("Failed to save error calibration constants for "<<m_idHelperSvc->toStringGasGap(gasGapId));
        return StatusCode::FAILURE;
    }
    return StatusCode::SUCCESS;
}
