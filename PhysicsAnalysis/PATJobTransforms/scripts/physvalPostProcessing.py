#!/usr/bin/env python3

# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT
import os
import argparse
import logging
import PATJobTransforms.physvalPostProcessingTools as physvalPostProcessingTools
from PATJobTransforms.physvalPostProcessingTools import (
    ConfigManager,
    EfficiencyComputation,
    ProjectionComputation,
    RebinningOperation,
    HistogramAdjustment,
    HistogramAddition,
    HistogramBlackList,
    ResolutionComputation,
    ROCCurveComputation
)

logging.basicConfig(level=logging.INFO)

from AthenaCommon.Utils.unixtools import find_datafile

try:
    electron_config = find_datafile("PATJobTransforms/physvalPostProcessingElectronConfig.yaml")
    photon_config = find_datafile("PATJobTransforms/physvalPostProcessingPhotonConfig.yaml")
    btag_config = find_datafile("PATJobTransforms/physvalPostProcessingBTagConfig.yaml")
except (ImportError, FileNotFoundError) as e:
    print(f"Error: {e}. Attempting fallback paths.")
    electron_config = find_datafile("../share/physvalPostProcessingElectronConfig.yaml")
    photon_config = find_datafile("../share/physvalPostProcessingPhotonConfig.yaml")
    btag_config = find_datafile("../share/physvalPostProcessingBTagConfig.yaml")
except Exception as e:
    print(f"Unexpected error: {e}. Please ensure the configuration files are available at the correct paths.")
    raise

def create_output_file(input_file, output_file):
    try:
        ROOT.TFile.Cp(input_file, output_file)
        output_root = ROOT.TFile.Open(output_file, "UPDATE")
        if not output_root or output_root.IsZombie():
            logging.error(f"Could not open {output_file}")
            return None
        # logging.info(f"Opened {output_file} file for updating.")
        return output_root
    except Exception as e:
        logging.error(f"Failed to create output file: {e}")
        return None

def process_domain(root_file, domain_config, domain):
    logging.info(f"For domain: '{domain}'")

    for rebin in domain_config.get("rebinning", []): RebinningOperation.from_yaml(rebin)(root_file)
    actions = []
    if "resolutions" in domain_config:
        for res in domain_config["resolutions"]: actions.append(lambda res=res: ResolutionComputation.from_yaml(res)(root_file))
    if "projections" in domain_config:
        for proj in domain_config["projections"]: actions.append(lambda proj=proj: ProjectionComputation.from_yaml(proj)(root_file))
    if "efficiencies" in domain_config:
        for eff in domain_config["efficiencies"]: actions.append(lambda eff=eff: EfficiencyComputation.from_yaml(eff)(root_file))
    if "adding_histograms" in domain_config:
        for add in domain_config["adding_histograms"]: actions.append(lambda add=add: HistogramAddition.from_yaml(add)(root_file))
    if "adjustments" in domain_config:
        for adj in domain_config["adjustments"]: actions.append(lambda adj=adj: HistogramAdjustment.from_yaml(adj)(root_file))
    if "roc_curves" in domain_config:
        for roc in domain_config["roc_curves"]: actions.append(lambda roc=roc: ROCCurveComputation.from_yaml(roc)(root_file))
    for action in actions: action()
    for list in domain_config.get("blacklist", []): HistogramBlackList.from_yaml(list)(root_file)

    for rebin in domain_config.get("rebinning_later", []): RebinningOperation.from_yaml(rebin)(root_file)
    
    logging.info(f"Finished post processing for domain: '{domain}'")

def main():
    parser = argparse.ArgumentParser(description="NTUPLE PHYSVAL Histogram Post-processing Tool")
    parser.add_argument("files", nargs="*", help="Input ROOT file and optionally output file name")
    parser.add_argument("--domain", type=str, help="Specific domain to process (e.g., Electron, Photon, BTag)")
    parser.add_argument("--crash_on_error", action="store_true", help="Crash on first error instead of just warning")
    args = parser.parse_args()

    physvalPostProcessingTools.crash_on_error = args.crash_on_error

    if len(args.files) == 1:
        input_file = args.files[0]
        output_file = os.path.splitext(input_file)[0] + "_after_post_process.root"
    elif len(args.files) == 2:
        input_file, output_file = args.files
    else:
        logging.error("You must provide 1 or 2 positional arguments: <input_file> [output_file]")
        exit(1)

    config_manager = ConfigManager()
    config_manager.add_configuration("Electron", electron_config)
    config_manager.add_configuration("Photon", photon_config)
    config_manager.add_configuration("BTag", btag_config)

    logging.info(f"Input: {input_file}")
    logging.info(f"Output: {output_file}")

    root_file = create_output_file(input_file, output_file)
    if not root_file:
        exit(1)

    logging.info("Post Processing started...")
    if args.domain:
        process_domain(root_file, config_manager.get_config(args.domain), args.domain)  # Processing specified domain
    else:
        for domain, domain_config in config_manager.get_all_configs():
            process_domain(root_file, domain_config, domain)  # Processing all domains

    try:
        root_file.Close()
        logging.info(f"Postprocessing complete. Output saved to: {output_file}")
    except Exception as e:
        logging.error(f"Failed to close ROOT file: {e}")
        exit(1)

if __name__ == "__main__":
    main()
