/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONFIT_MDTTUBECALIBCONTAINER_H
#define MUONFIT_MDTTUBECALIBCONTAINER_H

#include <vector>
#include <array>

#include "MdtCalibData/MdtTubeCalibContainer.h"

namespace MuonCalib {

    /** Holds single-tube full calibration information of one chamber */
    class MdtTubeFitContainer : public MdtTubeCalibContainer {
    public:
        struct SingleTubeFit {
            //!< number of hits used for the fit
            int statistics{0};
            //!< number of hits in the tube (differs from statistics in case of grouped fit
            int n_hits{0}; 
             //! number of hits above adc cut               
            int n_hits_above_adc_cut{0}; 
            //!< chisquared of the fit to the tdc spectrum
            float chi2Tdc{0.f}; 
             //! for MTT0 chi2 of trailing edge fit            
            float chi2TdcEnd{0.f};         
            std::array<float, 8> par{};
            std::array<float, 36> cov{};
            std::array<float, 4> adc_par{};
            std::array<float, 4> adc_err{};
            float adc_chi2{0.f};
            std::string group_by{"UNKNOWN"};
            SingleTubeFit() = default;
        };

        /** nMl = number of multilayres, nLayers = number of layers in multilayer (3 or 4); nTubes = number of tubes in one layer */
        MdtTubeFitContainer(const Muon::IMuonIdHelperSvc* idHelperSvc,
                            const Identifier& moduleID); 
        
        /** return calibration constants of a single tube */
        const SingleTubeFit* getFit(const Identifier& tubeId) const;
        SingleTubeFit* getFit(const Identifier& tubeId);

        /** set the calibration constants of a single tube */
        bool setFit(SingleTubeFit&& val, const Identifier& tubeId, MsgStream& log);

        /** set the name of the implementation used to fill this class */
        void setImplementation(const std::string& impl);
        /** return the name of this class */
        const std::string& name() const;
        /** return the name of the implementation filling this class */
        const std::string& implementation() const;

        void setGroupBy(const std::string& group_by);
     
        std::string GroupBy() const;
    private:
        std::vector<SingleTubeFit> m_info{};
        std::string m_name{"MdtTubeFitContainer"};
        std::string m_implementation{"implementation"};
        std::string m_group_by {"UNKNOWN"};
    };

}  // namespace MuonCalib

#endif
