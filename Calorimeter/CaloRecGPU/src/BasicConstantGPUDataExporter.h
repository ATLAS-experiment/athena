//
// Copyright (C) 2002-2023 CERN for the benefit of the ATLAS collaboration
//
// Dear emacs, this is -*- c++ -*-
//

#ifndef CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H
#define CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H

#include "AthenaBaseComps/AthAlgTool.h"
#include "CaloRecGPU/CaloClusterGPUTransformers.h"
#include "StoreGate/ReadCondHandleKey.h"
#include "CaloRecGPU/CaloGPUTimed.h"
#include "CaloConditions/CaloNoise.h"
#include "CaloDetDescr/CaloDetDescrManager.h"


/**
 * @class BasicConstantGPUDataExporter
 * @author Nuno Fernandes <nuno.dos.santos.fernandes@cern.ch>
 * @date 29 May 2022
 * @brief Standard tool to export calorimeter geometry and cell noise to GPU.
 *
 * For the time being, this must be run on first event, so that the noise tool exists and so on.
 * Hopefully we can find a way around this in the future.
 */

class BasicConstantGPUDataExporter :
  public AthAlgTool, virtual public ICaloClusterGPUConstantTransformer, public CaloGPUTimed
{
 public:

  BasicConstantGPUDataExporter(const std::string & type, const std::string & name, const IInterface * parent);

  virtual StatusCode initialize() override;

  virtual StatusCode convert (CaloRecGPU::ConstantDataHolder & constant_data, const bool override_keep_CPU_info) const override;

  virtual StatusCode convert (const EventContext & ctx, CaloRecGPU::ConstantDataHolder & constant_data, const bool override_keep_CPU_info) const override;

  virtual StatusCode finalize() override;

  virtual ~BasicConstantGPUDataExporter() = default;

 private:

  /** @brief If @p true, do not delete the CPU version of the GPU-friendly data representation.
   *  Defaults to @p false.
   *
   */

  Gaudi::Property<bool> m_keepCPUData {this, "KeepCPUData", true, "Keep CPU version of GPU data format"};

  /** @brief Key of the CaloNoise Conditions data object. Typical values
      are '"electronicNoise', 'pileupNoise', or '"totalNoise' (default) */

  SG::ReadCondHandleKey<CaloNoise> m_noiseCDOKey{this, "CaloNoiseKey", "totalNoise", "SG Key of CaloNoise data object"};

  /**
   * @brief Key for the CaloDetDescrManager in the Condition Store
   */
  SG::ReadCondHandleKey<CaloDetDescrManager> m_caloMgrKey{this, "CaloDetDescrManager", "CaloDetDescrManager",
                                                          "SG Key for CaloDetDescrManager in the Condition Store"};


  bool m_hasBeenInitialized;

};

#endif //CALORECGPU_BASICCONSTANTGPUDATAEXPORTER_H
