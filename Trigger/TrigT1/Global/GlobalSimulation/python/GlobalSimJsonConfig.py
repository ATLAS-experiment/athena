#!/usr/bin/env athena.py
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration
import json

from AthenaConfiguration.ComponentAccumulator import (ComponentAccumulator,)
from AthenaConfiguration.AccumulatorCache import AccumulatorCache
from AthenaConfiguration.ComponentFactory import CompFactory
from AthenaCommon.Constants import INFO
from AthenaCommon.Logging import log
import importlib
from pprint import pformat


def get_all_vector_keys(conf):
    key_props = set()
    vector_outputs = set()
    for pname,pdef in conf.getDefaultProperties().items():
        if pname.endswith('Key'):
            key_props.add(pname)
    for key in key_props:
        prop = getattr(conf,key)
        if prop.type().startswith('std::vector') and \
            prop.mode()=='W':
            vector_outputs.add(f'{prop.type()}#{prop.path}')
    return vector_outputs


# The alg is shared between all hypo tools in order to merge the TIP word
# For now, expect a dict extracted from json
# Might be a threshold class later
# The alg instantiation could also be pulled out into the upper level loop
# The kwargs can absorb any other info in the threshold, but it seems better to
# name all required properties explicitly
# No need to cache, we should just be doing a series of unique thresholds
def eEmEg1BDTMultCfg(flags,thrname,input,et_low,workingPoint,startbit,nbits,**kwargs):
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg('GlobalSimulationAlg')
    # This is for a working point with no eta-dependence
    # (Run 3 convention) Eta-dependent ones would require looping over eta ranges
    # with individual priorities
    tool = CompFactory.GlobalSim.eEmEg1BDTMultAlgTool(
        f'Egamma1BDTAlgTool_{thrname}',
        et_low=et_low,
        Eg1BDT=workingPoint['value'],
        Eg1BDT_op=workingPoint['compOp'],
        eEmEg1BDTTOBContainerKey=input['eEmEg1BDTTOBContainer'],
        TIPposition=startbit,
        TIPwidth=nbits,
        enable_dump=True,
    )
    alg.TIPwriters = [tool]
    cfg.addEventAlgo(alg)

    vector_outputs = get_all_vector_keys(tool)
    return cfg, vector_outputs


def eEmEg1MultCfg(flags,thrname,input,et_low,workingPoint,startbit,nbits,**kwargs):
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg('GlobalSimulationAlg')
    # This is for a working point with no eta-dependence
    # (Run 3 convention) Eta-dependent ones would require looping over eta ranges
    # with individual priorities
    tool = CompFactory.GlobalSim.eEmMultAlgTool(
        f'eEmMultAlgTool_{thrname}',
        et_low=et_low,
        rhad=workingPoint['rhad'],
        rhad_op=workingPoint['rhad_op'],
        reta=workingPoint['reta'],
        reta_op=workingPoint['reta_op'],
        wstot=workingPoint['wstot'],
        wstot_op=workingPoint['wstot_op'],
        eEmTOBs=input['eEmTOBContainer'],
        TIPposition=startbit,
        TIPwidth=nbits,
        enable_dump=True,
    )
    alg.TIPwriters = [tool]
    cfg.addEventAlgo(alg)

    vector_outputs = get_all_vector_keys(tool)
    return cfg, vector_outputs


def commonMultCfg(flags,thrname,input,et_low,startbit,nbits,**kwargs):
    cfg = ComponentAccumulator()
    alg = CompFactory.GlobalSim.GlobalSimulationAlg('GlobalSimulationAlg')
    tool =  CompFactory.GlobalSim.CommonMultAlgTool(
        f'commonMult_{thrname}',
        et_low = et_low,
        TIPposition=startbit,
        TIPwidth=nbits,
        # Assume this is unique, otherwise we need a custom configurator
        CommonTOBsKey = list(input.values())[0],
        enable_dump=True,
        )

    alg.TIPwriters = [tool]
    cfg.addEventAlgo(alg)

    vector_outputs = get_all_vector_keys(tool)
    return cfg, vector_outputs


# Define the mapping to non-generic config functions
threshold_type_configs = {
    'eEMBDT': eEmEg1BDTMultCfg,
    'eEMBeeDeeTee': eEmEg1BDTMultCfg,
    'eEMEg1': eEmEg1MultCfg,
}
def threshold_type_to_hypo_config(threshold):
    return threshold_type_configs.get(threshold,commonMultCfg)


# Forward configuration to a subtool
# One level of nesting here, make recursive if we need more
def forward_to_subtool(substr, subval, props):
    subname, subprop = substr.split('.')
    if '/' in subname:
        subtype, subname = subname.split('/')
    else:
        subtype = subname
    if subname not in props:
        props[subname] = CompFactory.getComp(f"GlobalSim::{subtype}")(
            subtype,
            **{subprop:subval}
        )
    else:
        props[subname][subprop] = subval

# Generic function to instantiate an Alg that does not need special config
# Currently assumes one instance per type
# If we wanted multiple instances, the type would have to be added to the menu
@AccumulatorCache
def GenericTOBProducerAlgCfg(flags, name, type, input, output, **params):
    cfg = ComponentAccumulator()

    props = {}
    for i in input:
        if '.' in i[0]:
            forward_to_subtool(i[0]+'Key',i[1],props)
        else:
            props[i[0]+'Key'] = i[1]
    for o in output:
        if '.' in o[0]:
            forward_to_subtool(o[0]+'Key',o[1],props)
        else:
            props[o[0]+'Key'] = o[1]
    
    for prop, val in params.items():
        if '.' in prop:
            forward_to_subtool(prop,val,props)
        else:
            props[prop] = val
    alg = CompFactory.getComp(f"GlobalSim::{type}")(
        name,
        **props)
    cfg.addEventAlgo(alg)

    # At the moment it doesn't seem like we want to write anything but it might be more
    # consistent if we just call the function anyway?
    vector_outputs = get_all_vector_keys(alg)
    return cfg, vector_outputs


# Generic function to instantiate an Alg that does not need special config
# Currently assumes one instance per type
# If we wanted multiple instances, the type would have to be added to the menu
@AccumulatorCache
def GenericTOBProducerAlgToolCfg(flags, name, type, input, output, **params):
    cfg = ComponentAccumulator()

    # Construct the tool and add it to a GlobalSimulationAlg
    props = {}
    for i in input:
        if '.' in i[0]:
            forward_to_subtool(i[0]+'Key',i[1],props)
        else:
            props[i[0]+'Key'] = i[1]
    for o in output:
        if '.' in o[0]:
            forward_to_subtool(o[0]+'Key',o[1],props)
        else:
            props[o[0]+'Key'] = o[1]

    for prop, val in params.items():
        if '.' in prop:
            forward_to_subtool(prop,val,props)
        else:
            props[prop] = val
    algtool = CompFactory.getComp(f"GlobalSim::{type}")(
        name,
        **props)
    alg = CompFactory.GlobalSim.GlobalSimulationAlg(
        name.replace("AlgTool","Alg"),
        globalsim_algs = [algtool]
        )
    cfg.addEventAlgo(alg)

    vector_outputs = get_all_vector_keys(algtool)
    return cfg, vector_outputs


# Ideally we use the generic configuration for everything, working
# from conventions in the json description, however some
# custom configurator functions may be necessary.
def TOBProvider_cfg_fn(name):
    if '.' in name:
        # Interpret as a python function in a specified module
        modulename, cfgname = name.rsplit('.',1)
        module = importlib.import_module(modulename)
        return lambda flags, **params: (getattr(module,cfgname)(flags, **params), set())
    elif name.endswith('AlgTool'):
        return GenericTOBProducerAlgToolCfg
    elif name.endswith('Alg'):
        return GenericTOBProducerAlgCfg
    else:
        return None
    

# Handle different ways of specifying input/output names for handle properties,
# converting the menu representation into a list of pairs
# - Defaults to assuming we set [TOBContainerName]Key = "[TOBContainerName]"
# - If the property name does not match the container name, then use the menu mapping
def expand_io(io):
    expanded = []
    if isinstance(io,str):
        expanded.append((io,io))
    else:
        if isinstance(io,list):
            expanded += [(el,el) for el in io]
        elif isinstance(io,dict):
            expanded += [(ttype,tname) for ttype,tname in io.items()]
        else:
            raise TypeError(f"Unhandled type {type(io)} for I/O definition")
    return tuple(expanded)


# Configure the components necessary to generate a list of required input TOBs
# By default, recurse through the prerequisites of these components to capture all requirements.
# Another way to do this would be to construct a DAG from the inputs/outputs,
# then iterate through the nodes and instantiate each component.
#
# No need to resolve objects created upstream of GSim
inputs_to_gsim = {
    'AllCalo',
    'GlobalLArCells',
    'L1_eEMRoI',
    'L1_eEMRoI_ReSim',
    'L1_jFexSRJetRoI',
    'L1_jFexSRJetRoI_ReSim',
    'L1_gFexRhoRoI',
}
def config_TOB_providers(flags, tobs_to_providers, input, recurse=True):
    TOB_CAs = []
    all_vector_outputs = set()

    upstream_deps = set()
    input_list = list(input)
    while len(input_list)>0:
        tob = input_list.pop()
        # For now, the GlobalLArCells are the common input for everything, which are produced upstream.
        # Specified explicitly in the menu to make it clear that the algorithm is at the start of Global
        if tob in inputs_to_gsim:
            continue
        assert tob in tobs_to_providers, f"Configuration for {tob} provider not specified in L0 menu!"
        provider = tobs_to_providers[tob]
        cfg_fn = TOBProvider_cfg_fn(provider['type'])
        assert cfg_fn is not None, f"No configuration available for {provider['type']}"
        params = dict(provider['parameters'])
        if '.' not in provider['type']:
            params.update(dict(
                name = provider['name'],
                type = provider['type'],
                input = provider['input'],
                output = provider['output'],
            ))
        TOB_cfg, vector_outputs = cfg_fn(flags,**params)
        TOB_CAs.append(TOB_cfg)
        all_vector_outputs |= vector_outputs
        log.debug(f"{provider['name']} needs:")
        log.debug(pformat(provider['input']))
        upstream_deps |= set(i[1] for i in provider['input'])

    # Organised this way, we do tiers of inputs,
    # prioritising those needed for the hypo first,
    # then iterating backwards
    # Mostly only for clarity in the sequence, but
    # useful if setting threads=0 (but why?)
    if recurse and upstream_deps:
        upstream_CAs, upstream_vecs = config_TOB_providers(flags, tobs_to_providers, upstream_deps, recurse=True)
        TOB_CAs += upstream_CAs
        all_vector_outputs |= upstream_vecs

    return TOB_CAs, all_vector_outputs


# Configure the TIPWriter that evaluates a single threshold, and the TOB providers that
# generate the necessary inputs to execute the TIPWriter
# The prerequisites should be shared with any TIPWriter processing the same threshold type
def L0HypoCfg(flags, threshold, hypo_specs, tobs_to_providers, triggerlines):

    cfg = ComponentAccumulator()

    # The menu describes the common properties of threshold types and specific instances
    # Extract all information for the current threshold instance
    hypo_dict = None
    for hypo_type, hypo_type_dict in hypo_specs.items():
        if threshold in hypo_type_dict['thresholds']:
            this_thr = hypo_type_dict['thresholds'][threshold]
            hypo_dict = dict(this_thr)
            hypo_dict['type'] = hypo_type
            hypo_dict['input'] = hypo_type_dict['input']
            log.info(f"  Menu contains specs for {threshold}: {this_thr}")
            if 'workingPoint' in hypo_dict:
                hypo_dict['workingPoint'] =  hypo_type_dict['workingPoints'][this_thr['workingPoint']]

            for line in triggerlines:
                if threshold == line['name']:
                    hypo_dict['startbit'] = line['startbit']
                    hypo_dict['nbits'] = line['nbits']
                    break
            break
    assert hypo_dict is not None, f"Threshold {threshold} not in L0 menu"
    assert 'startbit' in hypo_dict, f"Threshold {threshold} missing TIP position"

    log.debug(pformat(hypo_dict))

    # Put all the inputs in a supersequence to run before the single hypo alg
    cfg.addSequence(CompFactory.AthSequencer('GlobalSimulationInputs'))
    # This level mainly just makes clear which algs are needed for a given threshold
    # Many algs will be shared
    seqname = f"GlobalSimulationInputs_{hypo_dict['type']}"
    cfg.addSequence(CompFactory.AthSequencer(seqname),parentName='GlobalSimulationInputs')

    all_vector_outputs = set()
    # Grab the corresponding configuration function
    # This should interpret the threshold data to generate a single hypo instance
    hypo, vector_outputs = threshold_type_to_hypo_config(hypo_dict['type'])(flags,threshold,**hypo_dict)
    all_vector_outputs |= vector_outputs

    # Configure input algs for multiplicity
    #    Iterate backwards to their prerequisites
    #    Collect the globalsim_algs (TOB writers) for GlobalSimulationAlg
    # For aesthetic reasons (and for threads=0), collect the inputs and reverse order
    # Expand here to treat all formats in the json consistently
    inputs = expand_io(hypo_dict['input'])

    all_input_CAs, input_vector_outputs = config_TOB_providers(flags, tobs_to_providers, [i[1] for i in inputs])
    all_vector_outputs |= input_vector_outputs

    for ca in reversed(all_input_CAs):
        cfg.merge(ca,seqname)

    # Merge one GlobalSimulationAlg that runs all TIP writers
    cfg.merge(hypo)

    # Keep the hypo_dict around for diagnostics
    return cfg, all_vector_outputs, hypo_dict


def GlobalSimJsonCfg(
    flags,
    json_name,
    force_config_TOBs=[],
    ignore_prereqs=[],
    print_detailed_config=False
    ):
    menu = json.load(open(json_name))
    log.setLevel(INFO)

    gsim_alg_prefix = "GlobalSim_"

    cfg = ComponentAccumulator()
    # This is for placing any algorithms that produce inputs required by Global
    # that happen upstream of the hardware system
    cfg.addSequence(CompFactory.AthSequencer('GlobalSimPrereqs'))
    # Generate LAr Cells a la FEB2/LASP/MUX
    cfg.addSequence(CompFactory.AthSequencer('LArPreprocessing'), parentName='GlobalSimPrereqs')
    from GlobalSimulation.LArCellPreparationAlgConfig import LArCellPreparationAlgCfg
    cfg.merge( LArCellPreparationAlgCfg(
        flags,
        name = gsim_alg_prefix+'LArCellPreparationAlg',
        GlobalLArCellsKey='GlobalLArCells',
        ),
        sequenceName='LArPreprocessing'
    )

    cfg.addSequence(CompFactory.AthSequencer('GlobalSimulation'))

    all_vector_outputs = set()

    # Map output TOBs to their providers
    # Where necessary, the input property name minus 'Key', which hints at the type, is specified
    # in the json -- make a tuple with the type and name in all cases
    # This is probably more common for inputs...
    tobs_to_providers = {}
    for pname, provider in menu['algorithms'].items():
        log.debug(f"{pname} --> {provider}")
        output = expand_io(provider['output'])
        log.debug('Outputs:')
        log.debug(pformat(output))
        _provider = dict(provider)
        if '/' in pname:
            _provider['type'], _provider['name'] = pname.split('/',1)
        else:
            _provider['type'] = _provider['name'] = pname
        if gsim_alg_prefix:
            _provider['name'] = gsim_alg_prefix + _provider['name']
        _provider['input'] = expand_io(provider['input'])
        _provider['output'] = output
        _provider['parameters'] = {}
        _provider['parameters'].update(provider.get('parameters',{}))
        for otuple in output:
            assert otuple[1] not in tobs_to_providers, f"Duplicate provider for {otuple}"
            tobs_to_providers[otuple[1]] = _provider

    log.debug(pformat(tobs_to_providers))

    # Get L1 items (never mind CTP logic for now) and run configuration for each threshold
    # Let CA merging sort out all overlaps (add caching a la HLT later)
    active_thresholds = set()
    TIP_bits = 0
    for item, item_dict in menu['items'].items():
        for threshold in item_dict['thresholds']:
            active_thresholds.add(threshold)

    for threshold in active_thresholds:
        hypo_ca, vector_outputs, hypo_dict = L0HypoCfg(
            flags,
            threshold,
            hypo_specs = menu['hypotheses'],
            tobs_to_providers = tobs_to_providers,
            triggerlines = menu['connectors']['L0Global']['triggerlines']
        )
        cfg.merge(hypo_ca,'GlobalSimulation')
        all_vector_outputs |= vector_outputs

        # Check that the TIP bits don't overlap
        hypo_bits = sum([1<<i for i in range(hypo_dict['startbit'],hypo_dict['startbit']+hypo_dict['nbits'])])
        assert TIP_bits & hypo_bits == 0, f"TIP bits for {threshold} overlap other thresholds"
        TIP_bits = TIP_bits + hypo_bits

    # Explicitly requested TOBproviders
    for tob in force_config_TOBs:
        recurse = tob not in ignore_prereqs
        log.info(f"Explicitly configuring {tob} from menu {'with' if recurse else 'without'} prereqs")
        tob_cfgs, vector_outputs = config_TOB_providers(flags,tobs_to_providers,{tob},recurse)
        # Try to put in serial execution order, no guarantees
        for ca in reversed(tob_cfgs):
            cfg.merge(ca,'GlobalSimulation')
        all_vector_outputs |= vector_outputs

    # Record diagnostic data to output stream (auto-extract from algs?)
    from OutputStreamAthenaPool.OutputStreamConfig import addToAOD
    cfg.merge(addToAOD(flags,all_vector_outputs))

    if print_detailed_config:
        cfg.printConfig(summariseProps=True)

    return cfg


def main():
    from AthenaConfiguration.AllConfigFlags import initConfigFlags
    flags = initConfigFlags()

    # Standard argparse.ArgumentParser initialisation options can be provided here
    parser = flags.getArgumentParser(
        prog='GlobalSimJsonConfig',
        description='Prototype Global Simulation steering from json menu',
        epilog='Give us 25Gbps or give us ...'
    )
    parser.add_argument(
        '-j','--config-json',
        default='GlobalSimulation/L0GlobalTestMenuv1_Khoo.json',
        help='The input json config (L0 menu prototype)'
    )
    parser.add_argument(
        '--force-TOBs',
        default=[],
        nargs='+',
        help='List of TOBs to force on'
    )
    parser.add_argument(
        '--ignore-prereqs',
        default=[],
        nargs='+',
        help='List of forced TOBs for which prerequisites should be ignored'
    )

    args = flags.fillFromArgs(parser=parser)
    flags.lock()

    from AthenaConfiguration.MainServicesConfig import MainServicesCfg
    cfg = MainServicesCfg(flags)

    from AthenaConfiguration.Enums import Format
    if flags.Input.Format==Format.POOL:
        from AthenaPoolCnvSvc.PoolReadConfig import PoolReadCfg
        cfg.merge(PoolReadCfg(flags))
    elif flags.Input.Format==Format.BS:
        from ByteStreamCnvSvc.ByteStreamConfig import ByteStreamReadCfg
        cfg.merge(ByteStreamReadCfg(flags))

        from TrigConfigSvc.TrigConfigSvcCfg import L1ConfigSvcCfg
        cfg.merge(L1ConfigSvcCfg(flags))

        from L1CaloFEXByteStream.L1CaloFEXByteStreamConfig import eFexByteStreamToolCfg, jFexRoiByteStreamToolCfg, gFexByteStreamToolCfg
        decoderTools = []
        if flags.Trigger.L1.doeFex: decoderTools += [cfg.popToolsAndMerge(eFexByteStreamToolCfg(flags=flags,name='eFexBSDecoderTool',writeBS=False))]
        if flags.Trigger.L1.dojFex: decoderTools += [cfg.popToolsAndMerge(jFexRoiByteStreamToolCfg(flags=flags,name="jFexBSDecoderTool",writeBS=False))]
        if flags.Trigger.L1.dogFex: decoderTools += [cfg.popToolsAndMerge(gFexByteStreamToolCfg(flags=flags,name="gFexBSDecoderTool",writeBS=False))]
        cfg.addEventAlgo(CompFactory.L1TriggerByteStreamDecoderAlg(
            name="L1TriggerByteStreamDecoder",
            DecoderTools=decoderTools
        ))
    else:
        raise RuntimeError(f'Unrecognised input file format {flags.Input.Format} for {flags.Input.Files}')

    from TrigValTools.TrigValSteering.Common import find_file_in_path
    jsonpath = find_file_in_path(args.config_json, 'DATAPATH')
    cfg.merge( GlobalSimJsonCfg(flags, jsonpath, args.force_TOBs, args.ignore_prereqs, print_detailed_config=True) )

    cfg.run()



if __name__=="__main__":
    main()
