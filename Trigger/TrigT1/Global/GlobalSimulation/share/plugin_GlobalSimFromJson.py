from GlobalSimulation.GlobalSimJsonConfig import GlobalSimJsonCfg
from TrigValTools.TrigValSteering.Common import find_file_in_path

def setup(flags):
    if flags.Concurrency.NumThreads==0:
        flags.Concurrency.NumThreads=1

    # turn off everything else from the steering script
    flags.DQ.doMonitoring=False
    flags.Trigger.enableL1CaloPhase1=False

jsonpath = find_file_in_path("GlobalSimulation/L0GlobalTestMenuv1_Khoo.json", 'DATAPATH')

# List entries below are intentionally commented as examples for running code
# that I do not want tested in the CI at present.

force_TOBs = [
    # "GlobalSim_JET1Jets",
]

txt_inputs = {
    # 'GlobalSim_CellTowers': ('topoc_pu_type','/eos/atlas/atlascerngroupdisk/trig-gbl/online/ValidationGate/input_data/hex/mc_events/JET1/myfile_tree.hex.txt'),
}

txt_outputs = {
    # 'GlobalSim_JET1Jets': ('main_output_type','JET1_out_hex.txt'),
}

cfg.merge( GlobalSimJsonCfg(
    flags, jsonpath,
    ignore_menu_items=True,
    force_config_TOBs=force_TOBs,
    txt_inputs=txt_inputs,
    txt_outputs=txt_outputs
) )
