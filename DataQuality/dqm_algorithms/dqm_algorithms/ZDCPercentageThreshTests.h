/*
        Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

 /*! \file ZDCPercentageThreshTests.h file declares the dqm_algorithms::ZDCPercentageThreshTests class.
  * \author Yuhan Guo
 */

#ifndef DQM_ALGORITHMS_ZDCPERCENTAGETHRESHTESTS_H
#define DQM_ALGORITHMS_ZDCPERCENTAGETHRESHTESTS_H
                      
                      

#include "dqm_core/Algorithm.h"
#include <string>
#include <iosfwd>
#include <TH1.h>


namespace dqm_algorithms {

class ZDCPercentageThreshTests : public dqm_core::Algorithm {
public:

    ZDCPercentageThreshTests( const std::string & name );
  
    virtual ~ZDCPercentageThreshTests();
    virtual dqm_core::Algorithm*  clone();
    virtual dqm_core::Result*     execute( const std::string& name, const TObject& data, const dqm_core::AlgorithmConfig& config );
    double calculatePercentage(const TH1 * hist, dqm_core::Result * result);
    using dqm_core::Algorithm::printDescription;
    virtual void                  printDescription(std::ostream& out);

private:
    std::string  m_name;
    double m_minstat;
    double m_thresh;
    double m_greenTh;
    double m_redTh;
    int m_binNum;


};

} //namespace dqm_algorithms

#endif // DQM_ALGORITHMS_ZDCPERCENTAGETHRESHTESTS_H