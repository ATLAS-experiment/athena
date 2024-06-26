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

  /** @brief Arrival time of the signal at the readout */
  float m_time{0.f};
  /** @brief Arrival time of the secondary signal at the secondary readout */
  float m_ToT{-1.f};
  /** @brief Is the strip readout at opposite side */
  bool m_stripSide{false};

public:  // functions

  // Default constructor.
  RpcDigit()=default;

  /** @brief Full constructor
   *  @param id: Fired channel
   *  @param time: Time of arrival at the primary readout
   *  @param stripSide: Time of arrival at the secondary readout (BI-RPC)
   *  @param ToT: TIme over threshold (BI-RPCs)
  */
  RpcDigit(const Identifier& id, 
           float time, 
           float ToT  = -1.f,
           bool stripSide = false);

  /** @brief Return the primary time of arrival */
  float time() const { return m_time; }
  /** @brief Return the time of arrival at the second strip readout (BI-RPC)*/
  bool stripSide() const {return m_stripSide; }
  /** @brief Time over threshold */
  float ToT() const { return m_ToT; }

};

#endif
