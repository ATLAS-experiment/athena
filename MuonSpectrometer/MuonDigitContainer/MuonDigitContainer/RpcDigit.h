/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

// RpcDigit.h

#ifndef RpcDigitUH
#define RpcDigitUH

// RPC digitization. Holds a channel ID.

#include <iosfwd>
#include <limits>
#include "MuonDigitContainer/MuonDigit.h"

class RpcDigit : public MuonDigit {

private:  // data

  /** @brief Arrival time of the primary signal at the primary readout */
  float m_time{0.f};
  /** @brief Arrival time of the secondary signal at the secondary readout */
  float m_secTime{0.f};
  float m_ToT{-1.f};

public:  // functions

  // Default constructor.
  RpcDigit()=default;

  /** @brief: Constructor used for legacy digits
   *  @param id: Fired channel
   *  @param time: Time of arrival
  */
  RpcDigit(const Identifier& id,
           float time);
  /** @brief Full constructor
   *  @param id: Fired channel
   *  @param time: Time of arrival at the primary readout
   *  @param secTime: Time of arrival at the secondary readout (BI-RPC)
   *  @param ToT: TIme over threshold (BI-RPCs)
  */
  RpcDigit(const Identifier& id, 
           float time, 
           float secTime, 
           float ToT);

  /** @brief Return the primary time of arrival */
  float time() const { return m_time; }
  /** @brief Return the time of arrival at the second strip readout (BI-RPC)*/
  float secTime () const {return m_secTime; }
  /** @brief Time over threshold */
  float ToT() const { return m_ToT; }

};

#endif
