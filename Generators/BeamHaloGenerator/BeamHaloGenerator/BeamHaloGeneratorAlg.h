/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/

#ifndef BEAMHALOGENERATORALG_H
#define BEAMHALOGENERATORALG_H

#include "GaudiKernel/ServiceHandle.h"
#include "GeneratorModules/GenModule.h"
#include "GaudiKernel/ITHistSvc.h"
#include "TH1F.h"
#include "GeneratorModules/GenData.h"

#include <string>
#include <memory>

class BeamHaloGenerator;


/** @class BeamHaloGeneratorAlg

    @author W. H. Bell <W.Bell@cern.ch>
    
    @brief A GenModule algorithm to produce beam halo HepMC records
    from input ASCII files produced with MARS or FLUKA beam background
    simulations.
*/
class BeamHaloGeneratorAlg : public GenModule {
public:
  BeamHaloGeneratorAlg(const std::string& name, ISvcLocator *svcLocator);
  virtual ~BeamHaloGeneratorAlg() = default;
  virtual StatusCode genInitialize();
  virtual StatusCode genFinalize();

  /** Read one event from the selected input and convert it into
      GenEvent format.  If the end of file is reached the status code
      returned will not be success. */
  virtual StatusCode callGenerator();

  /** Fill the GenEvent pointer with the contents of the GenEvent
      cache.  The GenEvent cache is filled by calling
      callGenerator() */
  virtual StatusCode fillEvt(HepMC::GenEvent* evt);

private:

  /** Input file type and therefore associated beam halo generator
      that should be used. */
  StringProperty m_inputTypeStr{this, "inputType", "MARS-NM"};
  
  /** Input file name */
  StringProperty m_inputFile{this, "inputFile", "bgi-b2l1.1"};
  
  /** The position of the interface plane in mm. */
  DoubleProperty m_interfacePlane{this, "interfacePlane", 20850.0}; // (mm)
  
  /** Flag for flipping event */
  BooleanProperty m_enableFlip{this, "enableFlip", false};
  
  /** Flip probability */
  FloatProperty m_flipProbability{this, "flipProbability", 0.0};
  
  /** Flag to enable or disable sampling. */
  BooleanProperty m_enableSampling{this, "enableSampling", false};
  
  /** The name of the binary buffer file, needed for sampling from a
      converted file. */
  StringProperty m_bufferFileName{this, "bufferFileName", "BinaryBuffer.bin"};
  
  /** A vector of strings defining generator settings. */
  StringArrayProperty m_generatorSettings{this, "generatorSettings", {}, "A set of cuts to be applied to generated particles."};
  
  /** A flag to allow monitoring plots to be turned on or off. */
  BooleanProperty m_doMonitoringPlots{this, "doMonitoringPlots", false};
  
  /** A pointer to the THist service for validation plots. */
  ServiceHandle<ITHistSvc> m_tHistSvc{this, "THistSvc", "THistSvc"};
  
  /** Name of the random number stream */
  StringProperty m_randomStream{this, "randomStream", "BeamHalo"};
  
  /** A pointer to the beam halo generator */
  BeamHaloGenerator *m_beamHaloGenerator{};

  /** An empty GenEvent to cache the generate output between
      callGenerator and fillEvt. */
  HepMC::GenEvent m_evt{};

  enum validationPlotsEnum {
    PRI_R,
    PRI_Z,
    PRI_Z_TCT, 
    SP_R_ALL,
    SP_E_ALL,
    SP_PZ_ALL,
    SP_PT_ALL,
    SP_R_PROTONS,
    SP_E_PROTONS,
    SP_PZ_PROTONS,
    SP_PT_PROTONS,
    SP_R_MUONS,
    SP_E_MUONS,
    SP_PZ_MUONS,
    SP_PT_MUONS,
    NPLOTS};

  /** An array of TH1F pointers for validation plots */
  TH1F *m_validationPlots[NPLOTS];
  std::shared_ptr<GenData> m_gendata{nullptr};

};

#endif
