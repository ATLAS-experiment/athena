# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration
# @author: Zhaoyuan.Cui@cern.ch
# @date: Nov. 22, 2024
# @brief: Customized flags for FPGA data preparation pipeline

def addFPGADataPrepFlags(flags):


    flags.addFlag("FPGADataPrep.xclbin", "/eos/project/a/atlas-eftracking/FPGA_compilation/FPGA_compilation_hw/F110/kernels.hw.xclbin")
    flags.addFlag("FPGADataPrep.bdfID", "0000:c3:00.1")

    flags.addFlag("FPGADataPrep.DoActs", True)
    flags.addFlag("FPGADataPrep.FPGA.RunPixelClustering", True)
    flags.addFlag("FPGADataPrep.FPGA.RunSpacePoint", True)
    flags.addFlag("FPGADataPrep.FPGA.UseTV", False)
    
    flags.addFlag("FPGADataPrep.RunPassThrough", False)
    flags.addFlag("FPGADataPrep.PassThrough.RunSoftware", True)
    flags.addFlag("FPGADataPrep.PassThrough.ClusterOnly", False)
    flags.addFlag("FPGADataPrep.PassThrough.MaxClusterNum", 500000)
    flags.addFlag("FPGADataPrep.PassThrough.MaxSpacePointNum", 500000)
    flags.addFlag("FPGADataPrep.DoEmulation", False)
    flags.addFlag("FPGADataPrep.ForTiming", False)
    flags.addFlag("FPGADataPrep.doF110", False)

    
    return flags

def addClusterMakerFlags(flags):
    flags.addFlag("ClusterMaker.DoBulkCopy", False)
    
    return flags
