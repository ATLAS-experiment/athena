/*
  Copyright (C) 2002-2021 CERN for the benefit of the ATLAS collaboration
*/

#ifndef LARHV_HECHVMANAGER_H
#define LARHV_HECHVMANAGER_H

#include "LArHV/HECHVDescriptor.h"
#include "LArHV/HECHVModule.h"
#include "Identifier/HWIdentifier.h"
#include <memory>
#include <functional>

#if !(defined(SIMULATIONBASE) || defined(GENERATIONBASE))
class LArHVIdMapping;
#endif

class CondAttrListCollection;
struct HECHVPayload;

/**
 * @class HECHVManager
 *
 * @brief  This class provides direct access to information on the HV
 * electrodes within the barrels.  The information may be accessed either
 * directly or iteratively.  Direct access is provided by the getHVModule()
 * method.  Iterative access
 * is by looping over valid side, eta, phi, and sector indices to
 * retrieve a HV module.  From the high voltage modules one
 * can obtain a list of electrodes (iteratively or directly).
 *
 * The manager owns the pointers to the HV Modules.
 */

class HECHVManager
{
 public:
  class HECHVData
  {
  public:
    static constexpr double INVALID = -99999;
    class Payload;
    HECHVData();
    HECHVData (std::unique_ptr<Payload> payload);
    HECHVData& operator= (HECHVData&& other) noexcept;
    ~HECHVData();
    bool hvOn (const HECHVSubgap& subgap) const;
    double voltage (const HECHVSubgap& subgap) const;
    double current (const HECHVSubgap& subgap) const;
    int  hvLineNo  (const HECHVSubgap& subgap) const;
  private:
    static int index (const HECHVSubgap& subgap) ;
    std::unique_ptr<Payload> m_payload;
  };

  HECHVManager();
  ~HECHVManager();

  const HECHVDescriptor& getDescriptor() const;

  // Begin/End side index (0=negative and 1= positive)
  static unsigned int beginSideIndex() ;
  static unsigned int endSideIndex() ;

  static unsigned int beginPhiIndex() ;
  static unsigned int endPhiIndex() ;

  static unsigned int beginSamplingIndex() ;
  static unsigned int endSamplingIndex() ;

  const HECHVModule& getHVModule(unsigned int iSide
				 , unsigned int iPhi
				 , unsigned int iSampling) const;

  // Get the database payload --- for use by simulation only
  // (doesn't account for conditions changes)
  HECHVData getDataSim() const;

#if !(defined(SIMULATIONBASE) || defined(GENERATIONBASE))
  HECHVData getData (const LArHVIdMapping& hvIdMapping,
                     const std::vector<const CondAttrListCollection*>& attrLists) const;
  // Get hvLine for a subgap
  int hvLineNo(const HECHVSubgap& subgap
               , const LArHVIdMapping* hvIdMapping
               , HWIdentifier *hvlId=nullptr) const;
#endif

 private:
  using idfunc_t = std::function<std::vector<HWIdentifier>(HWIdentifier)>;
  HECHVData getData (const idfunc_t& idfunc,
                     const std::vector<const CondAttrListCollection*>& attrLists) const;

  HECHVManager(const HECHVManager& right) = delete;
  HECHVManager& operator=(const HECHVManager& right) = delete;

  class Clockwork;
  std::unique_ptr<const Clockwork> m_c;
};

#endif
