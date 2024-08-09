/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFULLINFO_MDTTUBECALIBCONTAINER_H
#define MUONFULLINFO_MDTTUBECALIBCONTAINER_H

#include <vector>

#include "MdtCalibData/MdtTubeCalibContainer.h"

namespace MuonCalib {

    /** Holds single-tube full calibration information of one chamber */
    class MdtTubeFullInfoContainer: public MdtTubeCalibContainer {
    public:
        struct SingleTubeFullInfo {
            int statistics{0};  //!< number of hits
            float chi2Tdc{0.f};   //!< chisquared of the fit to the tdc spectrum
            float t0Err{0.f};     //!< error on t0 from the fit to the tdc spectrum
            float tMax{0.f};      //!< tmax from the fit to the tdc spectrum
            float tMaxErr{0.f};   //!< error on tmax from the fit to the tdc spectrum
            float noise{0.f};     //!< noise level from the fit to the tdc spectrum
            float noiseErr{0.f};  //!< error on noise from the fit to the tdc spectrum
            float slope{0.f};     //!< slope of the tdc spectrum near t0
            float slopeErr{0.f};  //!< error on the slope of the tdc spectrum near t0
            SingleTubeFullInfo() = default;
        };
        /** nMl = number of multilayres, nLayers = number of layers in multilayer (3 or 4); nTubes = number of tubes in one layer */
        MdtTubeFullInfoContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                                 const Identifier& moduleID);

        /** return calibration constants of a single tube */
        const SingleTubeFullInfo* getFullInfo(const Identifier& tubeId) const;

        /** set the calibration constants of a single tube */
        bool setFullInfo(const Identifier& tubeId, SingleTubeFullInfo&& val);

        /** set the name of the implementation used to fill this class */
        void setImplementation(const std::string& impl);

        /** return the name of this class */
        const std::string& name() const;

        /** return the name of the implementation filling this class */
        const std::string& implementation() const;
    private:
        std::vector<SingleTubeFullInfo> m_info;
        std::string m_name{"MdtTubeFullInfoContainer"};
        std::string m_implementation{"implementation"};

    };

}  // namespace MuonCalib

#endif
