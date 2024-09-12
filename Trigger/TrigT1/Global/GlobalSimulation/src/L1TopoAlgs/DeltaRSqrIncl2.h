/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_DELTARSQRINCL2_H
#define GLOBALSIM_DELTARSQRINCL2_H

// based on DeltaRSqrIncl2 of L1TopoSimulation

// Decision algorithm to accept pairs of TOBs with DeltaR requirements

#include <string>
#include <vector>

class StatusCode;

namespace GlobalSim {
  class GenericTOBArray;
  class Decision;
  
  class DeltaRSqrIncl2 {
  public:

    DeltaRSqrIncl2(const std::string & name,
		   unsigned int MaxTOB1,
		   unsigned int MaxTOB2,
		   const std::vector<unsigned int>& MinET1,
		   const std::vector<unsigned int>& MinET2,
		   const std::vector<unsigned int>& DeltaRMin,
		   const std::vector<unsigned int>& DeltaRMax,
		   unsigned int NumResultBits);
    
    virtual ~DeltaRSqrIncl2() = default;

    StatusCode
    run(const GenericTOBArray& in0,
	const GenericTOBArray& in1,
	std::vector<GenericTOBArray>& output,
	Decision&);
  
    
    // expose variables to be monitored

    unsigned int numResultBits() const;
    const std::vector<double>& deltaRSqPassByBit(std::size_t bit) const;
    const std::vector<double>& deltaRSqFailByBit(std::size_t bit) const;

    std::string toString() const;

  private:

    std::string m_name{"DeltaRSqrIncl2_object"};
    unsigned int m_MaxTOB1{0u};
    unsigned int m_MaxTOB2{0u};

    std::vector<unsigned int> m_MinET1{};
    std::vector<unsigned int> m_MinET2{};

    std::vector<unsigned int> m_DeltaRMin{};
    std::vector<unsigned int> m_DeltaRMax{};

    unsigned int m_NumResultBits{0};

    // variables to be monitored
    // maintain a vector of values for each output bit
    std::vector<std::vector<double>> m_DeltaRSqPass{
      m_NumResultBits,
      std::vector<double>{}
    };
     
    std::vector<std::vector<double>> m_DeltaRSqFail{
      m_NumResultBits,
      std::vector<double>{}
    };

  };
   
}

#endif
