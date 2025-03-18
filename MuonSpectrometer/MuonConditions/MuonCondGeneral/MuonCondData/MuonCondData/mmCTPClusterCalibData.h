/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONCONDDATA_MMCTPCLUSTERCALIBDATA_H
#define MUONCONDDATA_MMCTPCLUSTERCALIBDATA_H


// Athena includes
#include "AthenaKernel/CondCont.h" 
#include "AthenaKernel/BaseInfo.h" 
#include "AthenaBaseComps/AthMessaging.h"
#include "MuonIdHelpers/IMuonIdHelperSvc.h"
#include "GeoPrimitives/GeoPrimitives.h"
#include "GeoModelUtilities/TransientConstSharedPtr.h"

namespace Muon{
class mmCTPClusterCalibData: public AthMessaging {
    public:

    class CTPParameters{
        public:
            CTPParameters() = default;
            CTPParameters(std::array<double, 2>&& pars);
            CTPParameters(CTPParameters&& other) noexcept : m_pars(std::move(other.m_pars)) {}
            CTPParameters& operator=(CTPParameters&& other) = default;

            const std::array<double, 2>& pars() const { return m_pars; }

        private:       
            std::array<double, 2> m_pars{0., 0.};

    };

        mmCTPClusterCalibData(const Muon::IMuonIdHelperSvc* idHelperSvc);
         ~mmCTPClusterCalibData() = default;

        StatusCode storeConstants(const Identifier& gasGapId, CTPParameters&& newConstants);
        double getCTPCorrectedDriftVelocity(const Identifier& identifier, const double theta) const;
    
    private:
        const Muon::IMuonIdHelperSvc* m_idHelperSvc{nullptr};
        using parameterMap = std::unordered_map<Identifier, CTPParameters>; 
        parameterMap m_database{};
};

} // end for Muon namespace


CLASS_DEF( Muon::mmCTPClusterCalibData , 211195114 , 1 );
CONDCONT_DEF( Muon::mmCTPClusterCalibData , 242286916 );

#endif
