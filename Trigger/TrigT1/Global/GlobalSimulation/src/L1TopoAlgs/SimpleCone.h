/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef GLOBALSIM_SIMPLECONE_H
#define GLOBALSIM_SIMPLECONE_H

// based on SimpleCone of L1TopoSimulation

#include <string>
#include <vector>

class StatusCode;

namespace GlobalSim {
  class GenericTOBArray;
  class Decision;

  class SimpleCone{
  public:
    
    SimpleCone(const std::string & name,
	       unsigned int InputWidth,
	       unsigned int MaxRSqr,
	       unsigned int MinET,
	       const std::vector<unsigned int>& MinSumET);

    virtual ~SimpleCone() = default;

    StatusCode
    run(GenericTOBArray const& input,
	std::vector<GenericTOBArray>& output,
	Decision&);
  
    
    std::string toString() const;

    const std::vector<double>& ETPassByBit(std::size_t bit) const;
    const std::vector<double>& ETFailByBit(std::size_t bit) const;

  private:

    std::string m_name{};
    
    unsigned int m_InputWidth {0};
    unsigned int m_MaxRSqr{0};
    unsigned int m_MinET {0};
    std::vector<unsigned int> m_MinSumET{};

    // monitored data
    std::vector<std::vector<double>> m_ETPassByBit {
      m_MinSumET.size(),
      std::vector<double>{}
    };
    
    std::vector<std::vector<double>> m_ETFailByBit {
      m_MinSumET.size(),
      std::vector<double>{}
    };

  };
   
}

#endif
