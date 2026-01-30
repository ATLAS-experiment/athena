/*
 *   Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
 */

#ifndef TRIGGEPPERF_GEPERATIOALG_H
#define TRIGGEPPERF_GEPERATIOALG_H

#include "AthenaBaseComps/AthReentrantAlgorithm.h"
#include "StoreGate/ReadHandleKey.h"
#include "StoreGate/WriteDecorHandleKey.h"
#include "xAODTrigger/eFexEMRoIContainer.h"
#include "xAODTrigger/eFexTauRoIContainer.h"
#include "TrigGepPerf/GepCellMap.h"

namespace Gep {

template <typename T>
class GepEratioAlg : public AthReentrantAlgorithm {
public:
    GepEratioAlg(const std::string& name, ISvcLocator* pSvcLocator);

    virtual StatusCode initialize() override;
    virtual StatusCode execute(const EventContext& ctx) const override;

private:
    SG::ReadHandleKey<DataVector<T>> m_seedsKey{this, "SeedsKey", "L1_eEMRoI", "Input eFex seeds"};
    SG::ReadHandleKey<Gep::GepCellMap> m_gepCellsKey{this, "gepCellMapKey", "GepCells", "Input calo cells map"};

    SG::WriteDecorHandleKey<DataVector<T>> m_eratioKey{
        this, "OutputEratioDecorKey", "Eratio", "Output variable decorated with Eratio result"};
};

}

#include "GepEratioAlg.ixx"



class GepEMEratioAlg: public Gep::GepEratioAlg<xAOD::eFexEMRoI>
{
public:
  GepEMEratioAlg(const std::string& name, ISvcLocator* pSvcLocator) : Gep::GepEratioAlg<xAOD::eFexEMRoI>(name, pSvcLocator) {}
};

class GepTauEratioAlg: public Gep::GepEratioAlg<xAOD::eFexTauRoI>
{
public:
  GepTauEratioAlg(const std::string& name, ISvcLocator* pSvcLocator) : Gep::GepEratioAlg<xAOD::eFexTauRoI>(name, pSvcLocator) {}
};

#endif
