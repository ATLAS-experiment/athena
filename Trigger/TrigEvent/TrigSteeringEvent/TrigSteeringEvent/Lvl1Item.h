/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/

/**********************************************************************************
 * @Project: HLT
 * @Package: TrigSteeringEvent
 * @class  : Lvl1Result
 *
 * @brief container for compact version of the results of all three trigger levels
 *
 * @author Nicolas Berger  <Nicolas.Berger@cern.ch>  - CERN
 * @author Till Eifert     <Till.Eifert@cern.ch>     - U. of Geneva, Switzerland
 * @author Ricardo Goncalo <Jose.Goncalo@cern.ch>    - Royal Holloway, U. of London
 **********************************************************************************/

#ifndef TRIGSTEERINGEVENT_LVL1ITEM_H
#define TRIGSTEERINGEVENT_LVL1ITEM_H

#include "xAODCore/CLASS_DEF.h"

#include <string>


namespace LVL1CTP {

  /**
   * \brief
   *
   */

  class Lvl1Item {

  public:

    Lvl1Item( const std::string& n, unsigned int hash, bool passBP = false,
              bool passAP = true, bool passAV = true, float factor = 1) :
      m_name(n), m_hashId(hash), m_passBP(passBP), m_passAP(passAP), m_passAV(passAV),
      m_prescaleFactor(factor) {}

    const std::string&  name()   const { return m_name; }
    unsigned int        hashId() const { return m_hashId; }

    bool isPassedBeforePrescale() const    { return m_passBP; }
    bool isPassedAfterPrescale()  const    { return m_passAP; }
    bool isPassedAfterVeto()      const    { return m_passAV; }

    bool isPassed()      const    { return isPassedAfterVeto(); }

    bool isPrescaled() const { return isPassedBeforePrescale() && ! isPassedAfterPrescale(); }
    bool isVeto() const      { return isPassedAfterPrescale()  && ! isPassedAfterVeto();     }

    float prescaleFactor() const { return m_prescaleFactor; }

    void setName  (const std::string& name) { m_name = name; }
    void setHashId(unsigned int hash) { m_hashId = hash; }
    void setPrescaleFactor(float factor) { m_prescaleFactor = factor; }

  private:

    std::string m_name;
    unsigned int m_hashId;
    bool m_passBP, m_passAP, m_passAV;
    float m_prescaleFactor;
  };

} // end of namespace

#endif
