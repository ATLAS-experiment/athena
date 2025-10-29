/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_ERATIO_H
#define GLOBALSIM_ERATIO_H


// ERatio implementation: an algorithm to detect pi0s in the
// neighborhood of an EFEX RoI using LAr strip data.
// ERatio looks for a significant secondary energy peak in the
// LAr strips located close to an EFex RoI

#include "../IO/LArStripNeighborhoodContainer.h"

#include <string>
#include <vector>

class StatusCode;

namespace GlobalSim {
  
  class ERatio {
  public:

    ERatio(const std::string & name,
	   double thresh=50.,
	   double maxERCut=2.5);
    
    virtual ~ERatio() = default;

    StatusCode
    run(const LArStripNeighborhood& in, bool& result);
    
    std::string toString() const;

  private:

    std::string m_name{"ERatio_object"};
    double m_minDeltaE{50.}; // minimum signicant strip energy difference
    double m_maxERCut{2.5}; // max value for ERatio to be tagged as pi0

    double secondaryPeakEnergy_forw(const StripDataVector&, std::size_t);
    double secondaryPeakEnergy_back(const StripDataVector&, std::size_t);

    bool eratioCut(double p_peakE, double s_peakE);

    
  };   
}

std::ostream& operator<< (std::ostream&, const GlobalSim::ERatio&);


#endif
