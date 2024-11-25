# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
# @author: Zhaoyuan.Cui@cern.ch
# @date: Nov. 22, 2024
# @brief: Customized flags for FPGA data preparation pipeline

def addFPGADataPrepFlags():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()
    
    flags.addFlag("FPGADataPrep.RunPassThrough", False)
    flags.addFlag("FPGADataPrep.PassThrough.RunSoftware", True)
    flags.addFlag("FPGADataPrep.PassThrough.ClusterOnly", False)
    
    return flags