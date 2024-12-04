/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#include "MuonCalibMath/BaseFunctionFitter.h"
#include "GeoModelKernel/throwExcept.h"
#include <cmath>

namespace MuonCalib {
    BaseFunctionFitter::BaseFunctionFitter() {
        init();
    } 
    BaseFunctionFitter::BaseFunctionFitter(const unsigned nb_coefficients) {
        init(nb_coefficients);
    }
    void BaseFunctionFitter::init() { init(5); }

    void BaseFunctionFitter::init(const unsigned nb_coefficients) {
        m_A = Amg::MatrixX(nb_coefficients,nb_coefficients);
        m_A.setZero();
        m_b = Amg::VectorX(nb_coefficients);
        m_b.setZero();
        m_alpha = Amg::VectorX(nb_coefficients);
        m_alpha.setZero();
        m_nb_coefficients = nb_coefficients;
    }
    int BaseFunctionFitter::number_of_coefficients() const {
        return m_nb_coefficients;
    }
    const Amg::VectorX& BaseFunctionFitter::coefficients() const {
        return m_alpha;
    }
    void BaseFunctionFitter::set_number_of_coefficients(const unsigned nb_coefficients) {
        init(nb_coefficients);
    }
    void BaseFunctionFitter::fit_parameters(const std::vector<SamplePoint> & sample_point,
                                            const unsigned int first_point,
                                            const unsigned int last_point,
                                            const BaseFunction& base_function) {
        if (first_point<1 || first_point>sample_point.size()) {
            THROW_EXCEPTION("BaseFunctionFitter::fit_parameters() - ERROR: Illegal first point "
						    <<first_point<<", must be >=1 and <="<<sample_point.size());
        }
        if (last_point<first_point || last_point>sample_point.size()) {
            THROW_EXCEPTION("BaseFunctionFitter::fit_parameters() - ERROR: Illegal last point "<<last_point
			              <<", must be >="<<first_point<<" and <="<<sample_point.size());
         }
        // clear the objects //
        init(m_nb_coefficients);

        for (unsigned j=0; j<m_nb_coefficients; ++j) {
            for (unsigned p=j; p<m_nb_coefficients; ++p) {
              for (unsigned k=first_point-1; k<last_point; ++k) {
                m_A.fillSymmetric(j,p,m_A(j,p)+base_function.value(j,sample_point[k].x1())
                                              *base_function.value(p,sample_point[k].x1()) /
                          std::pow(sample_point[k].error(), 2));
              }
            }
            for (unsigned k=first_point-1; k<last_point; k++) {
                 m_b[j] = m_b[j]+sample_point[k].x2()* base_function.value(j, sample_point[k].x1()) /
                          std::pow(sample_point[k].error(), 2);
            }
        }
        // perform the minimization //
        m_alpha = m_A.inverse()*m_b;
    }
}
