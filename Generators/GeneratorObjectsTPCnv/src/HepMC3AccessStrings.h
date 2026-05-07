/*
  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GENERATOROBJECTSTPCNV_MC3ACCESSSTRINGS_H 
#define GENERATOROBJECTSTPCNV_MC3ACCESSSTRINGS_H  


namespace GeneratorObjectsTPCnv{
  #ifdef HEPMC3
    const std::string barcodesStr{"barcodes"};
    const std::string signalProcessIdStr{"signal_process_id"};
    const std::string mpiStr{"mpi"};
    const std::string eventScaleStr{"event_scale"};
    const std::string filterWeightStr{"filterWeight"};
    const std::string alphaQcdStr{"alphaQCD"};
    const std::string alphaQedStr{"alphaQED"};
    const std::string randomStatesStr{"random_states"};
    const std::string filterHtStr{"filterHT"};
    const std::string filterMetStr{"filterMET"};
 #endif
}
#endif