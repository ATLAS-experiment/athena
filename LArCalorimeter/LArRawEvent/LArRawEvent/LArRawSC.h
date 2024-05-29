/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARRAWSC_H
#define LARRAWSC_H
#include <vector>
#include <utility> //for std::move

#include "Identifier/HWIdentifier.h"

/**
   @class LArRawSC
   @brief Liquid Argon SuperCell raw data

   represents the output of the LATOME
*/

class LArRawSC  final{

 private:

  /** @brief Online Identifier */
  HWIdentifier m_hardwareID{};

  /** @brief Latome channel */
  short m_chan = 0;

  /** @brief LATOME source Id*/
  unsigned int m_sourceId = 0U;

  /** @brief vector of energies */
  std::vector < int > m_energies;

  /** @brief vector of bcids */
  std::vector < unsigned short > m_BCId;

  /** @brief vector of saturation flags */
  std::vector < bool > m_satur;

  /** @brief did pass tau selection */
  std::vector < bool > m_passTauSelection;

  /** @brief vector of taus*energies */
  std::vector< int > m_tauEnergies;

  /** @brief OFCa Overflow */
  bool m_ofcaOverflow{};

  /** @brief OFCb Overflow */
  bool m_ofcbOverflow{};

  /** @brief pedestal Overflow */
  bool m_pedOverflow{};

 public:
  /** @brief constructor
      @param[in] channel_value  Online identifier
      @param[in] gain_value  Gain
      @param[in] sample_value Reference of vector with ADC samples
  */
  LArRawSC(const HWIdentifier& channel_value, const short chan,
           const unsigned int sourceId, const std::vector<int>& energies,
           const std::vector<unsigned short>& bcids, std::vector<bool>& satur)
      : m_hardwareID(channel_value),
        m_chan(chan),
        m_sourceId(sourceId),
        m_energies(energies),
        m_BCId(bcids),
        m_satur(satur),
        m_ofcaOverflow(false),
        m_ofcbOverflow(false),
        m_pedOverflow(false) {}

  LArRawSC(const HWIdentifier& channel_value, const short chan,
           const unsigned int sourceId, std::vector<int>&& energies,
           std::vector<unsigned short>&& bcids, std::vector<bool>&& satur)
      : m_hardwareID(channel_value),
        m_chan(chan),
        m_sourceId(sourceId),
        m_energies(std::move(energies)),
        m_BCId(std::move(bcids)),
        m_satur(std::move(satur)),
        m_ofcaOverflow(false),
        m_ofcbOverflow(false),
        m_pedOverflow(false) {}

  /** @return HWIdentifier */
  const HWIdentifier& hardwareID() const { return m_hardwareID; }

  /** @return channel */
  short chan() const { return m_chan; }

  /** @return source Id value */
  unsigned int SourceId() const { return m_sourceId; }

  /** @return number of samples */
  int nsamples() const { return m_energies.size(); }

  /** @return a reference to a stl vector containing the energies values */
  const std::vector<int>& energies() const { return m_energies; }

  /** @return a reference to a stl vector containing the bcid values */
  const std::vector<unsigned short>& bcids() const { return m_BCId; }

  /** @return a reference to a stl vector containing the saturation flags */
  const std::vector<bool>& satur() const { return m_satur; }

  /** @return a reference to a stl vector containing the energies*taus */
  const std::vector<int>& tauEnergies() const { return m_tauEnergies; }

  /** @return true if the channel passes the tau selection */
  const std::vector<bool>& passTauSelection() const {
    return m_passTauSelection; }

    /** @return OFCa Overflow */
    bool ofcaOverflow() const { return m_ofcaOverflow; }

    /** @return OFCb Overflow */
    bool ofcbOverflow() const { return m_ofcbOverflow; }

    /** @return pedestal Overflow */
    bool pedOverflow() const { return m_pedOverflow; }


    /** @brief Set identifier */
    void setHardwareId(const HWIdentifier id);

    /** @brief Set channel number */
    void setChannel(const unsigned chan);

    /** @brief Set source id */
    void setSourceId(const unsigned sourceId);

    /** @brief Set energies .
        @param[in] samples  vector of energies
    */
    void setEnergies( const std::vector < int >&& energies);

    /** @brief Set bcids.
        @param[in] samples  vector of bcids
    */
    void setBCIds( const std::vector < unsigned short >&& bcids);

    /** @brief Set energies*taus .
        @param[in] samples  vector of energies*taus
    */
    void setTauEnergies( const std::vector < int >&& tauEnergies);

    /** @set true if passes the tau selection */
    void setPassTauSelection( const std::vector < bool >&& pass);


    /** @set saturation flags*/
    void setSaturation( const std::vector < bool >&& satur);

    /** @set OFCa Overflow */
    void setOFCaOverflow(bool overflow){ m_ofcaOverflow=overflow; }

    /** @set OFCb Overflow */
    void setOFCbOverflow(bool overflow){ m_ofcbOverflow=overflow; }

    /** @set pedestal Overflow */
    void setPedOverflow(bool overflow){ m_pedOverflow=overflow; }

    /** @brief default constructor for persistency */
    LArRawSC() = default;
    /** @brief default copy/move assignement */
    LArRawSC(const LArRawSC&) = default;
    LArRawSC(LArRawSC&&) = default;
    LArRawSC& operator=(const LArRawSC&) = default;
    LArRawSC& operator=(LArRawSC&&) = default;
    /** @brief Destructor */
    ~LArRawSC() = default;


};

#endif //LArRawSC_H

