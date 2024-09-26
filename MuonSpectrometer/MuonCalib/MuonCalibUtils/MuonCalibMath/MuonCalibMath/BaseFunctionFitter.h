/*
  Copyright (C) 2002-2017 CERN for the benefit of the ATLAS collaboration
*/



#ifndef BaseFunctionFitterHXX
#define BaseFunctionFitterHXX

//////////////////
// HEADER FILES //
//////////////////

// CLHEP //
//#include "CLHEP/config/CLHEP.h"
#include "CLHEP/Units/SystemOfUnits.h"
#include "CLHEP/Units/PhysicalConstants.h"

#include "EventPrimitives/EventPrimitives.h"

// standard C++ libraries //
#include <iostream>
#include <iomanip>
#include <fstream>

// STL //
#include <vector>

// MDT calibration utilities //
#include "MuonCalibMath/BaseFunction.h"
#include "MuonCalibMath/SamplePoint.h"

namespace MuonCalib {

    /**
      @class BaseFunctionFitter
            This class performs a fit of a linear combination of base functions to 
            a set of sample points. */
    class BaseFunctionFitter {

      private:
          // auxiliary minimization objects //
          int m_nb_coefficients{0}; //!< number of coefficients
          Amg::MatrixX m_A; //!< coefficient matrix for the fit
          Amg::VectorX m_alpha; //!< coefficients of the base functions after the fit
          Amg::VectorX m_b; //!< m_A*m_alpha = m_b;

          // private methods //
          void init(); //!< default initialization method
          /** initialization method: 
            the number of fit parameters (coefficients) is set to nb_coefficients */
          void init(const unsigned nb_coefficients); 

      public:
        // Constructors
        /** default constructor, the number of fit parameters will be set to 5 */
        BaseFunctionFitter();
        /** constructor, the number of fit parameters is set to nb_coefficients */
        BaseFunctionFitter(const unsigned nb_coefficients);
        /** get the number of fit parameters (coefficients) of the base functions 
	          to be fitted */
        int number_of_coefficients() const;
         /** get the coefficients determined by the fit to the sample points */ 
        const Amg::VectorX& coefficients() const;
        /** set the number of fit parameters (coefficients) of the base
            functions to nb_coefficients */
        void set_number_of_coefficients(const unsigned nb_coefficients);
        /** perform a fit of the base functions (base_function) to the sample
    	     points as given in "sample_point" starting at the sample point first_point
    	     and stopping at the point last_point, 
    	     1 <= first_point < last_point <= size of the sample_point vector;
    	     the method returns true, if the fit failed */
        bool fit_parameters(const std::vector<SamplePoint> & sample_point,
			                      const unsigned int first_point,
			                      const unsigned int last_point,
			                      BaseFunction * base_function);
    };
}
#endif
