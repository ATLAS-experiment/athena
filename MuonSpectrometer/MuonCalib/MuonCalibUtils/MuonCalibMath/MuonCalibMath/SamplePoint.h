/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/
#ifndef MuonCalibMath_SamplePoint_H
#define MuonCalibMath_SamplePoint_H

#include <array>
namespace MuonCalib { 

/**
@class SamplePoint
  This class provides a sample point for the BaseFunctionFitter.
*/

  class SamplePoint {
    private:
        // sample point triplet //
        double m_x1{0.}; //!< x1 coordinate of the sample point
        double m_x2{0.}; //!< x2 coordinate of the sample point
        double m_sigma{0.}; //!< error of the x2 coordinate of the sample point
  public:
    // Constructors
    SamplePoint() = default;
    /** constructor,
        triplet[0] = x1 coordinate of the sample point,
        triplet[1] = x2 coordinate of the sample point,
        triplet[2] = error of the x2 coordinate of the sample point */
    SamplePoint(const std::array<double, 3>& triplet):
        m_x1{triplet[0]}, m_x2{triplet[1]}, m_sigma{triplet[2]}{}

    /** constructor,
        mx1 = x1 coordinate of the sample point,
        mx2 = x2 coordinate of the sample point,
        msigma = error of the x2 coordinate of the sample point */
    SamplePoint(const double  mx1, 
                const double  mx2,
		            const double  msigma):
        m_x1{mx1}, m_x2{mx2}, m_sigma{msigma} {}
    // Methods
    // get-methods //
    //!< get the x1 coordinate of the sample point
    inline double x1() const { return m_x1; }
    //!< get the x2 coordinate of the sample point
    inline double x2() const { return m_x2; }
    //!< get the error on the x2 coordinate of the sample point
    inline double error() const { return m_sigma; }
    // set-methods //
    /** set the sample point,
        x[0] = x1 coordinate of the sample point,
        x[1] = x2 coordinate of the sample point,
        x[2] = error of the x2 coordinate of the sample point */
    inline void set_triplet(const std::array<double, 3>& values){
       m_x1 = values[0];
       m_x2 = values[1];
       m_sigma = values[2];
    } 
    //!< set the x1 coordinate of the sample point to mx1
    inline void set_x1(const double  mx1){m_x1 = mx1; }
    //!< set the x2 coordinate of the sample point to mx2 
    inline void set_x2(const double  mx2){m_x2 = mx2; }
     //!< set the error of the x2 coordinate sample point to merror
    inline void set_error(const double  merror) { m_sigma = merror; }
  };
}
#endif
