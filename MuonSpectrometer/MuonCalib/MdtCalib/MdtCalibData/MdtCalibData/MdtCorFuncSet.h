/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MDTCORFUNCSET_H
#define MDTCORFUNCSET_H

#include "MdtCalibData/IMdtBFieldCorFunc.h"
#include "MdtCalibData/IMdtBackgroundCorFunc.h"
#include "MdtCalibData/IMdtSlewCorFunc.h"
#include "MdtCalibData/IMdtTempCorFunc.h"

#include <memory>
namespace MuonCalib {
     /** Class which holds all correction functions for a given region.
        The segmentation can differ from the rt calibration region */
    class MdtCorFuncSet {
    public:
        MdtCorFuncSet() = default;

        MdtCorFuncSet(std::unique_ptr<IMdtSlewCorFunc> && s, 
                      std::unique_ptr<IMdtBFieldCorFunc>&& bf, 
                      std::unique_ptr<IMdtTempCorFunc>&& t, 
                      std::unique_ptr<IMdtBackgroundCorFunc>&& bg);

        ~MdtCorFuncSet() = default;

        const IMdtSlewCorFunc* slewing() const { return m_slewing.get(); }
        const IMdtBFieldCorFunc* bField() const { return m_bField.get(); }
        const IMdtTempCorFunc* temperature() const { return m_temperature.get(); }
        const IMdtBackgroundCorFunc* background() const { return m_background.get(); }

        void setSlewing(std::unique_ptr<IMdtSlewCorFunc>&& slew);
        void setBField(std::unique_ptr<IMdtBFieldCorFunc>&& bField);
        void setTemperature(std::unique_ptr<IMdtTempCorFunc>&& temperature);
        void background(std::unique_ptr<IMdtBackgroundCorFunc>&& background);

    private:

        std::unique_ptr<IMdtSlewCorFunc> m_slewing{};
        std::unique_ptr<IMdtBFieldCorFunc> m_bField{};
        std::unique_ptr<IMdtTempCorFunc> m_temperature{};
        std::unique_ptr<IMdtBackgroundCorFunc> m_background{};
    };

}  // namespace MuonCalib

#endif
