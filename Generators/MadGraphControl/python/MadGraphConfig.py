# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg

import os
import time

from MadGraphControl.MadGraphPDFSettings import MadGraphPDFSets, get_pdf_set


def _get_nevents(flags, safety):
    """Helper function to determing number of events to be generated
    in MadGraph, based on MaxEvents or nEventsPerJob and a user-provided 
    safety factor (the latter defaults to 1.1 to allow for failures 
    in showering stage)."""
    try:
        sf = float(safety)
    except (TypeError, ValueError) as exc:
        raise RuntimeError(f"safety must be numeric, got {safety}.") from exc
    if sf <= 0:
        raise RuntimeError(f"safety must be > 0, got {safety}.")

    base_events = (
        flags.Exec.MaxEvents
        if flags.Exec.MaxEvents > 0
        else flags.Generator.nEventsPerJob
    )

    return int(base_events * sf)

def _prepare_lhe_for_shower(produced_output, lhe_file):
    # The supported lhe file formats are .lhe, .lhe.gz, .tar.gz, and .tgz.
    # .tar.gz and .tgz are tarballs that contain a single .lhe file
    # .gz files can be read directly by Pythia so we don't want to unzip them.
    if (produced_output and produced_output.endswith(".gz")
            and not produced_output.endswith((".tar.gz", ".tgz"))):
        compressed_lhe_file = (
            lhe_file if lhe_file.endswith(".gz") else f"{lhe_file}.gz"
        )
        if _symlink_first_existing(
                compressed_lhe_file, [produced_output], overwrite=True):
            return
        raise RuntimeError(
            "Could not prepare compressed LHE file for showering. "
            f"Expected: {produced_output}"
        )

    primary_output = None

    if produced_output:
        if produced_output.endswith(".tar.gz"):
            root = produced_output[:-7]
        elif produced_output.endswith(".tgz"):
            root = produced_output[:-4]
        elif produced_output.endswith(".gz"):

            root = produced_output[:-3]

        else:
            root, _ = os.path.splitext(produced_output)
        #primary_output = f"{root}.events"
        primary_output = f"{root}"
    
    # If the transform requested a specific TXT output name, symlink the 
    # produced output to the filename that the transform expects
    candidates = [candidate 
                  for candidate in (primary_output, 
                                    "tmp_LHE_events.events", 
                                    "events.events") 
                  if candidate]

    if _symlink_first_existing(lhe_file, candidates, overwrite=True):
        return


    raise RuntimeError(
        "Could not prepare LHE file for showering. "
        f"Expected one of: {', '.join(candidates)}"
    )


def _symlink_first_existing(link_name, candidates, overwrite=False):
    """
    Helper function to symlink the first existing file in candidates to link_name.
    """
    print("nnnnnnnn")
    print(link_name)
    print(candidates)
    if os.path.exists(link_name) and not overwrite:
        return True

    for candidate in candidates:
        if not candidate or not os.path.exists(candidate):
            print(os.path.exists(candidate))
            continue
        if os.path.abspath(candidate) == os.path.abspath(link_name):
            print(os.path.abspath(link_name))
            return True
        if os.path.lexists(link_name):
            print("qqqqqqq")

            os.remove(link_name)
        os.symlink(os.path.abspath(candidate), link_name)
        return True

    return False


def MadGraphBaseCfg(flags, **kwargs):
    """Base MadGraph CA fragment. It returns a CA object 
    that contains the generator metadata and registers 
    default values for steering the MGC object 
    (to be created by the top-level config)."""
    from MadGraphControl.MGC import (
        MADGRAPH_CATCH_ERRORS,
        MADGRAPH_DEVICES,
        MADGRAPH_PDFSETTING,
    )

    # Default values for MGC. Use MGC defaults for now, but these
    # can be declared here in the future.
    defaults = {
        "safety": 1.1,
        "pdf_setting": MADGRAPH_PDFSETTING,
        "devices": MADGRAPH_DEVICES,
        "catch_errors": MADGRAPH_CATCH_ERRORS,
        "lhe_version": 3,
        "saveProcDir": False,
        "keepJpegs": False,
        "usePMGSettings": False,
    }

    # Create a dictionary with default settings.
    # This can be used in top-level configs.
    cfg = {**defaults, **{k: v for k, v in kwargs.items() if v is not None}}

    # Create the CA object adding the generator metadata
    ca = ComponentAccumulator(EvgenSequenceFactory(EvgenSequence.Generator))
    ca.merge(
        GeneratorInfoSvcCfg(flags, Generators=["MadGraph"]),
        sequenceName=EvgenSequence.Generator.value,
    )
    
    return ca, cfg


def MadGraphCfg(
    flags,
    process_definition,
    *,
    safety=None,
    run_card_settings=None,
    param_card_settings=None,
    pdf_setting=None,
    devices=None,
    catch_errors=None,
    lhe_version=None,
    saveProcDir=None,
    plugin=None,
    keepJpegs=None,
    usePMGSettings=None,
    prepare_lhe_for_shower=False,
    lhe_file="events.lhe",
):
    """
    Fragment for configuring a LHE generation step.

    This starts from MadGraphBaseCfg and creates a MGC instance
    that is later used to call the MadGraphUtils functions that steer
    the event generation.

    All arguments after * are keyword-only to avoid confusion 
    between MadGraphControl settings and CA configuration options.
    Set prepare_lhe_for_shower=True when the same job should feed the
    produced LHE file into a shower generator.

    process_definition is required, the rest is optional.

    run_card_settings maps run_card.dat settings to their requested values.
    param_card_settings maps param_card.dat settings to dictionaries of
    parameter indices and values.

    If prepare_lhe_for_shower is True, the produced LHE file will be 
    symlinked to lhe_file (default: events.lhe) 
    for later use in the showering step.
    """

    from MadGraphControl.MGC import MGControl
    import MadGraphControl.MadGraphUtils as MadGraphUtils

    if isinstance(pdf_setting, MadGraphPDFSets):
        pdf_setting = get_pdf_set(pdf_setting)

    # TODO: implement deduplication of settings as done in Pythia8Config
    ca, cfg = MadGraphBaseCfg(
        flags,
        safety=safety,
        pdf_setting=pdf_setting,
        devices=devices,
        catch_errors=catch_errors,
        lhe_version=lhe_version,
        saveProcDir=saveProcDir,
        keepJpegs=keepJpegs,
        usePMGSettings=usePMGSettings,
    )

    run_card_settings = {} if run_card_settings is None else dict(run_card_settings)
    param_card_settings = {} if param_card_settings is None else dict(param_card_settings)

    # Overwrite the number of events in the run_card_settings with the value
    # determined from the flags and safety factor.
    run_card_settings["nevents"] = _get_nevents(flags, cfg["safety"])

    # Create the MGC instance
    mgc = MGControl(
        process=process_definition,
        plugin=plugin,
        keepJpegs=cfg["keepJpegs"],
        usePMGSettings=cfg["usePMGSettings"],
        pdf_setting=cfg["pdf_setting"],
        devices=cfg["devices"],
        catch_errors=cfg["catch_errors"],
    )

    # Bind the MGC instance to the MadGraphUtils module, 
    # so that it can be used in the calls to the MadGraphUtils functions 
    # This is not following the CA logic completely, but it avoids 
    # having to pass the MGC instance through multiple function calls.
    MadGraphUtils.my_MGC_instance = mgc

    # Create the process directory
    process_dir = mgc.process_dir

    # Modify the run_card settings in the process directory before generating events.
    mgc.runCardDict.update(run_card_settings)

    # Modify the parameter_card settings in the process directory before generating events.
    mgc.paramCard.modify_paramCardDict(
        params=param_card_settings
    )

    # Generate events
    MadGraphUtils.generate(process_dir=process_dir, flags=flags, pdf_setting=cfg["pdf_setting"])
    produced_output = MadGraphUtils.arrange_output(
        process_dir=process_dir,
        flags=flags,
        lhe_version=cfg["lhe_version"],
        saveProcDir=cfg["saveProcDir"],
        pdf_setting=cfg["pdf_setting"],
    )

    # If requested, prepare the produced LHE file for showering
    # by symlinking it to the filename that pythia expects, 
    # by default "events.lhe".


    # If the transform requested a specific TXT output name, symlink the 
    # produced output to the filename that the transform expects
    # (only if the file does not exist).
    requested_output = flags.Output.TXTFileName
    if requested_output and not os.path.exists(requested_output):
        root, _ = os.path.splitext(requested_output)
        candidates = [candidate for candidate in (produced_output, f"{root}.events", "events.events") if candidate]
        _symlink_first_existing(requested_output, candidates, overwrite=True)
    
    if prepare_lhe_for_shower:
        _prepare_lhe_for_shower(produced_output, lhe_file)
    return ca
