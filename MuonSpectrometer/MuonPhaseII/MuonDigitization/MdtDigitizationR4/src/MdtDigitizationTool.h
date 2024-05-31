/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MDT_DIGITIZATIONR4_MDTDIGITIZATIONTOOL_H
#define MDT_DIGITIZATIONR4_MDTDIGITIZATIONTOOL_H


#include "MuonDigitizationR4/MuonDigitizationTool.h"
#include "MDT_Digitization/IMDT_DigitizationTool.h"
#include "MuonDigitContainer/MdtDigitContainer.h"
#include "MdtCalibData/MdtCalibDataContainer.h"
#include "MuonCondData/MdtCondDbData.h"

namespace MuonR4{
    class MdtDigitizationTool final: public MuonDigitizationTool {
        public:
            MdtDigitizationTool(const std::string& type, const std::string& name, const IInterface* pIID);

            StatusCode initialize() override final;
        protected:
            StatusCode digitize(const EventContext& ctx,
                                const TimedHits& hitsToDigit,
                                xAOD::MuonSimHitContainer* sdoContainer) const override final; 
        
 
        private:
            SG::WriteHandleKey<MdtDigitContainer> m_writeKey{this, "OutputObjectName", "MDT_Digits"};

            SG::ReadCondHandleKey<MuonCalib::MdtCalibDataContainer> m_calibDbKey{this, "CalibDataKey", "MdtCalibConstants",
                                                                                 "Conditions object containing the calibrations"};


            SG::ReadCondHandleKey<MdtCondDbData> m_badTubeKey{this, "BadTubeKey", "MdtCondDbData", "Key of MdtCondDbData"};

            ToolHandle<IMDT_DigitizationTool> m_digiTool{this, "DigitizationTool", ""};

            Gaudi::Property<double> m_timeResTDC{this, "ResolutionTDC", 0.5, "TDC time resolution"};
            Gaudi::Property<double> m_timeResADC{this, "ResolutionADC", 0.5 * Gaudi::Units::ns, "ADC time resolution"};
            Gaudi::Property<double> m_deadTime{this, "DeadTime", 700., "MDT drift tube dead time"};

            using DigiCache = OutDigitCache_t<MdtDigitCollection>;

    };
}
#endif