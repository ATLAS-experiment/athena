# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

from AthenaConfiguration.ComponentAccumulator import ComponentAccumulator
from GeneratorConfig.Sequences import EvgenSequence, EvgenSequenceFactory
from GeneratorConfig.GeneratorInfoSvcConfig import GeneratorInfoSvcCfg

import os

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


def MadGraph_LHE_Cfg(
    flags,
    process_definition,
    *,
    safety=None,
    settings=None,
    pdf_setting=None,
    devices=None,
    catch_errors=None,
    lhe_version=None,
    saveProcDir=None,
    plugin=None,
    keepJpegs=None,
    usePMGSettings=None,
):
    """Fragment for configuring a standalone (LHE-only) generation step.

    This creates starts from MadGraphBaseCfg and creates a MGC instance
    that is later used to call the MadGraphUtil functions that steer
    the event generation.

    All arguments after * are keyword-only to avoid confusion 
    between MadGraphControl settings and CA configuration options.

    process_definition is required, the rest is optional.
    """

    from MadGraphControl.MGC import MGControl
    import MadGraphControl.MadGraphUtils as MadGraphUtils

    if isinstance(pdf_setting, MadGraphPDFSets):
        pdf_setting = get_pdf_set(pdf_setting)

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

    run_card_settings = {} if settings is None else dict(settings)

    # Get nEvents
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
    MadGraphUtils.modify_run_card(
        process_dir=process_dir,
        flags=flags,
        settings=run_card_settings,
        pdf_setting=cfg["pdf_setting"],
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

    # Create a symlink to the produced output with the name 
    # requested by the user.
    requested_output = flags.Output.TXTFileName
    if requested_output and not os.path.exists(requested_output):
        if os.path.lexists(requested_output):
            os.remove(requested_output)
        candidates = []
        if produced_output:
            candidates.append(produced_output)
        root, _ = os.path.splitext(requested_output)
        candidates.extend([f"{root}.events", "events.events"])
        for candidate in candidates:
            if candidate and os.path.exists(candidate) and candidate != requested_output:
                os.symlink(os.path.abspath(candidate), requested_output)
                break

    return ca
