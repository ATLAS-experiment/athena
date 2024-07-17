/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef MUONR4__HOUGHSEGMENT__H
#define MUONR4__HOUGHSEGMENT__H
#include <vector>

#include "MuonSpacePoint/MuonSpacePointContainer.h"
#include "xAODMeasurementBase/UncalibratedMeasurement.h"
#include "AthenaKernel/CLASS_DEF.h"
#include "MuonReadoutGeometryR4/MuonChamber.h"

namespace MuonR4{

    /// @brief Placeholder for what will later be the muon segment EDM representation. 
    /// For now, just a plain storage for the dummy fit result, to test the 
    /// implementation of residuals 
    class MuonSegment{
        public: 
            MuonSegment(double y0, double x0, double tanTheta, double tanPhi, const std::vector<const xAOD::UncalibratedMeasurement*> & measurements):
                m_y0(y0), m_x0(x0), m_tanTheta(tanTheta), m_tanPhi(tanPhi), m_measurements(measurements){

            }  
            // setters 
            void setChi2(double chi2){m_chi2 = chi2;} 
            void setX0(double x0){m_x0 = x0;}
            void setY0(double y0){m_y0 = y0;}
            void setTanTheta(double tanTheta){m_tanTheta = tanTheta;}
            void setTanPhi(double tanPhi){m_tanPhi = tanPhi;}
            void setMeasurements(const std::vector<const xAOD::UncalibratedMeasurement*> & measurements){m_measurements = measurements;}
            void setChi2PerMeasurement(size_t iMeasurement, double chi2){
                m_chi2PerMeasurement.resize(std::max(iMeasurement, m_measurements.size())); 
                m_chi2PerMeasurement.at(iMeasurement) = chi2; 
            }void setChi2PerMeasurement(const std::vector<double> & chi2vals){
                m_chi2PerMeasurement = chi2vals; 
            }
            // temporary validation info - will be removed in the final segment iteration
            void setChamber(const MuonGMR4::MuonChamber* ch){m_chamber=ch;}
            void setParentSeed(const MuonR4::HoughSegmentSeed* parent){m_parent=parent;}

            // getters 
            double chi2() const {return m_chi2;}
            double x0() const {return m_x0;}
            double y0() const {return m_y0;}
            double tanTheta() const {return m_tanTheta;}
            double tanPhi() const {return m_tanPhi;}
            std::vector<const xAOD::UncalibratedMeasurement*> measurements() const {return m_measurements;}
            std::vector<double> chi2PerMeasurement() const{return m_chi2PerMeasurement;} 
            // temporary validation info - will be removed in the final segment iteration
            const MuonGMR4::MuonChamber* chamber()const {return m_chamber;}
            const MuonR4::HoughSegmentSeed* parent()const {return m_parent;}


        private: 
            double m_y0{0};
            double m_x0{0};
            double m_tanTheta{0};
            double m_tanPhi{0};
            double m_chi2{0};
            std::vector<const xAOD::UncalibratedMeasurement*> m_measurements{};
            std::vector<double> m_chi2PerMeasurement{};
            const MuonGMR4::MuonChamber* m_chamber{nullptr}; 
            const MuonR4::HoughSegmentSeed* m_parent{nullptr}; 
    };
    // placeholder - later will be xAOD EDM 
    using MuonSegmentContainer = std::vector<MuonSegment> ; 
}
CLASS_DEF( MuonR4::MuonSegmentContainer , 1185056226 , 1 )

#endif
