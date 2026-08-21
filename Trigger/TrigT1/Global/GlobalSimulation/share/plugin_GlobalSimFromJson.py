from GlobalSimulation.GlobalSimJsonConfig import GlobalSimJsonCfg
from TrigValTools.TrigValSteering.Common import find_file_in_path

def setup(flags):
    if flags.Concurrency.NumThreads==0:
        flags.Concurrency.NumThreads=1

#jsonpath = find_file_in_path("L0GlobalTestMenuv1_Khoo.json", 'DATAPATH')
jsonpath = find_file_in_path("GlobalSimulation/L0GlobalTestMenuv1_AllChainsCfg.json", 'DATAPATH')

force_TOBs = [
    # 'LArCellMux'
    "gFexRhoTOBs",
]

ignore_prereqs = [
    # 'LArCellMux'
]

cfg.merge( GlobalSimJsonCfg(flags, jsonpath, force_TOBs, ignore_prereqs) )
