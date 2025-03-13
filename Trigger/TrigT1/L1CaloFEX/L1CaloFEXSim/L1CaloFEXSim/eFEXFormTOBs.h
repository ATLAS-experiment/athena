/*
  Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
*/

//***********************************************************************
//                                 eFEXFormTOBs.h
//                                 --------------
//     begin                       : 30 04 2021
//     email                       : nicholas.andrew.luongo@cern.ch
//***********************************************************************

#ifndef eFEXFORMTOBS_H
#define eFEXFORMTOBS_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "L1CaloFEXToolInterfaces/IeFEXFormTOBs.h"

namespace LVL1 {

  //Doxygen class description below:
  /** The eFEXFormTOBs class provides functions for creating TOBs for eFEX objects
  */

  class eFEXFormTOBs : public AthAlgTool, virtual public IeFEXFormTOBs {

  public:
    /** Constructors */
    eFEXFormTOBs(const std::string& type, const std::string& name, const IInterface* parent);

    /** standard Athena-Algorithm method */
    virtual StatusCode initialize() override;
    /** Destructor */
    virtual ~eFEXFormTOBs();

    
    virtual uint32_t formTauTOBWord(int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) const override;
    virtual std::vector<uint32_t> formTauxTOBWords(int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) const override;

    virtual uint32_t formEmTOBWord(int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) const override;
    virtual std::vector<uint32_t> formEmxTOBWords(int, int, int, int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int) const override;

    /** Internal data */
  private:
    const unsigned int m_tobETshift = 2;
    const unsigned int m_fpgaShift = 30;
    const unsigned int m_etaShift = 27;
    const unsigned int m_phiShift = 24;
    const unsigned int m_rhadShift = 22;
    const unsigned int m_wstotShift = 20;
    const unsigned int m_retaShift = 18;
    const unsigned int m_seedShift = 16;
    const unsigned int m_undShift = 15;
    const unsigned int m_seedMaxShift = 14;
    const unsigned int m_shelfShift = 24;
    const unsigned int m_efexShift = 20;
    const unsigned int m_taurhadShift = 20;
    const unsigned int m_taurcoreShift = 18;
    const unsigned int m_algoVersionShift= 12;

  };

} // end of namespace

CLASS_DEF( LVL1::eFEXFormTOBs , 261506707 , 1 )

#endif
