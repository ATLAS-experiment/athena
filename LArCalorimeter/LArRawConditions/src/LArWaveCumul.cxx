/*
  Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
*/


#include "LArRawConditions/LArWaveCumul.h"
#include <math.h>


void LArWaveCumul::addEvent( int delay , int step ,
			     const std::vector<double>& Samples )
{
  const unsigned int nSwave = getSize() ;
  const unsigned int nSevt  = Samples.size() ;
  for ( unsigned int i=0 ; i<nSevt ; i++ ) {
    unsigned int k=(i+1)*step-(delay+1);
    if (k<nSwave) {
      double evt  = Samples[i] ;
      int    N    = m_triggers[k] ;
      double amp2 = m_amplitudes[k]*m_amplitudes[k];
      double sum2 = (N-1)*m_errors[k]*m_errors[k] + N*amp2;

      double sum  = m_amplitudes[k] * N ;
      m_amplitudes[k] = ( sum + evt ) / (N+1) ;
      double arg = N? ( ( sum2 + evt*evt ) - (N+1)*m_amplitudes[k]*m_amplitudes[k] )/N: 0.;

      // Can happen due to underflow if m_errors is zero and evt is
      // exactly the same as m_amplitudes.
      if (arg < 0) arg = 0;
      m_errors[k]     = sqrt(arg);
      m_triggers[k]   = N + 1 ;

    }
  }
}



void LArWaveCumul::addAccumulatedEvent( int delay , int step ,
			                const std::vector<double>& SamplesSum ,
			                const std::vector<double>& Samples2Sum , 
			                unsigned nTriggers )
{
  if (!nTriggers) return;  // there should be related warnings displayed by the LArLATOMEDecoder
  const unsigned int nSwave = getSize() ;
  const unsigned int nSevt  = SamplesSum.size() ;
  
  for ( unsigned int i=0 ; i<nSevt ; i++ ) {

    unsigned int k=(i+1)*step-(delay+1);
    
    if (k<nSwave ) {

      int    N1    = m_triggers[k] ;
      double sum1  = m_amplitudes[k] * N1 ;
      double sum12 = (N1-1)*m_errors[k]*m_errors[k] + N1*m_amplitudes[k]*m_amplitudes[k] ;

      int    N2    = nTriggers ;
      double sum2  = SamplesSum[i] ;
      double sum22 = Samples2Sum[i] ;
      
      m_triggers[k]   = N1+N2 ;
      m_amplitudes[k] = (sum1+sum2)/m_triggers[k] ;
      m_errors[k]     = std::sqrt(std::max(sum12+sum22-m_amplitudes[k]*m_amplitudes[k]*m_triggers[k], 0.)/(m_triggers[k]-1) ) ;

    }
  }
}
