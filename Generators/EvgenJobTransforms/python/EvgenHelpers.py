#  Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

import os
import re
import gzip
import shutil
import tarfile
from enum import Enum

from AthenaCommon.Logging import logging

evgenLog = logging.getLogger("Gen_tf")


def _mk_symlink(srcfile, dstfile):
    """Helper function to make symlinks."""
    if dstfile:
        if os.path.exists(dstfile) and not os.path.samefile(dstfile, srcfile):
            os.remove(dstfile)
        if not os.path.exists(dstfile):
            evgenLog.info(f"Symlinking {srcfile} to {dstfile}")
            print (f"Symlinking {srcfile} to {dstfile}")
            os.symlink(srcfile, dstfile)
        else:
            evgenLog.debug(f"Symlinking: {dstfile} is already the same as {srcfile}")


def _count_lhe_events(lhe_file):
    """Helper function to count LHE events in a file.
    Support for plain text, gz, tar.gz, tgz files.
    Use chunked reading to avoid memory issues with large files.
    """
    def _count_in_stream(stream):
        count_ev = 0
        for chunk in iter(lambda: stream.read(1024 * 1024), b""):
            count_ev += chunk.count(b"/event")
        return count_ev

    if lhe_file.endswith((".tar.gz", ".tgz", ".tar")):
        count_ev = 0
        with tarfile.open(lhe_file, "r:*") as tar:
            for member in tar:
                if not member.isfile():
                    continue
                extracted = tar.extractfile(member)
                if extracted is None:
                    continue
                with extracted:
                    count_ev += _count_in_stream(extracted)
        return count_ev

    if lhe_file.endswith(".gz"):
        with gzip.open(lhe_file, "rb") as f:
            return _count_in_stream(f)

    with open(lhe_file, "rb") as f:
        return _count_in_stream(f)


def _find_unique_file(pattern):
    """Helper functions for finding input file"""
    import glob
    files = glob.glob(pattern)
    # Check that there is exactly 1 match
    if not files:
        raise RuntimeError(f"No {pattern} file found")
    elif len(files) > 1:
        raise RuntimeError(f"More than one {pattern} file found")
    return files[0]


def _prepare_lhe_file(input_file, output_file):
    """
    Helper function to prepare LHE file for shower.
    If the requested output file is uncompressed, 
    make a symlink to the input file.
    If the requested output file is compressed, 
    compress the input file to the output file..
    """
    if not output_file.endswith(".gz"):
        _mk_symlink(input_file, output_file)
        return

    if os.path.lexists(output_file):
        os.remove(output_file)
    evgenLog.info("Compressing %s to %s", input_file, output_file)
    with open(input_file, "rb") as source:
        with gzip.open(output_file, "wb") as destination:
            shutil.copyfileobj(source, destination)


def _merge_lhe_files(listOfFiles, outputFile):
    """
    This function merges a list of input LHE files into one output file.
    The header is taken from the first file, but the number of events is
    updated to equal the total number of events in all input files.
    """
    if os.path.exists(outputFile):
        print("outputFile", outputFile, "already exists. Will rename to", outputFile + ".OLD")
        os.rename(outputFile, outputFile + ".OLD")

    total_events = 0
    for file in listOfFiles:
        total_events += _count_lhe_events(file)

    wrote_header = False
    # Produce a compressed merged file when avoidExtracting is True
    output_opener = gzip.open if outputFile.endswith(".gz") else open
    with output_opener(outputFile, "wt") as output:
        for file in listOfFiles:
            inHeader = True
            header = ""
            print("*** Starting file", file)
            with open(file, "r") as infile:
                for line in infile:
                    # Reading first event signals that we are done with all header information.
                    if "<event" in line and inHeader:
                        inHeader = False
                        if not wrote_header:
                            wrote_header = True
                            output.write(header)
                        output.write(line)
                    # Each input file ends with "</LesHouchesEvents>". We only write it once at the end.
                    elif not inHeader and "</LesHouchesEvents>" not in line:
                        output.write(line)

                    if inHeader:
                        # Format for storing number of events differs in MG and Powheg.
                        if "nevents" in line:
                            # MG5 format is "n = nevents".
                            parts = line.split("=")
                            if parts:
                                line = line.replace(parts[0], str(total_events), 1)
                        elif "numevts" in line:
                            # Powheg format is "numevts n".
                            parts = line.split()
                            if len(parts) > 1:
                                line = line.replace(parts[1], str(total_events), 1)
                        header += line

        output.write("</LesHouchesEvents>\n")


def _handle_input_files(generators, flags):
    """Helper for handling input files"""
    from GeneratorConfig.GenConfigHelpers import gens_lhef
    is_lhe_input = gens_lhef(generators)

    # Name of event files produced by various generators.
    events_file_map = {
        "Alpgen": "alpgen.unw_events",
        "Protos": "protos.events",
        "ProtosLHEF": "protoslhef.events",
        "BeamHaloGenerator": "beamhalogen.events",
        "HepMCAscii": "events.hepmc",
        "ReadMcAscii": "events.hepmc",
    }
    eventsFile = None
    for gen_name, out_file in events_file_map.items():
        if gen_name in generators:
            eventsFile = out_file
            break
    if eventsFile is None:
        if is_lhe_input:
            eventsFile = "events.lhe.gz" if flags.Generator.avoidExtracting else "events.lhe"
        else:
            raise RuntimeError(f"Unknown type of ME generator: {generators}")

    genInputFiles = [f.strip() for f in flags.Generator.inputGeneratorFile.split(",") if f.strip()]
    if not genInputFiles:
        raise RuntimeError("Generator.inputGeneratorFile is empty while input handling is requested")

    def _input_root(path, keep_suffix_after_underscore=False):
        fname = os.path.basename(path)
        if any(ext in fname for ext in (".tar.", ".tgz", ".gz")):
            fname = re.split(r"\.tar\.|\.tgz|\.gz", fname, maxsplit=1)[0]
        if fname.endswith(".events"):
            fname = fname[:-7]
        parts = fname.split("._", 1)
        if keep_suffix_after_underscore and len(parts) > 1:
            return parts[0] + "._" + parts[1].split(".", 1)[0]
        return parts[0]

    # If there is a single file, make a symlink. If multiple files, merge them into one output eventsFile.
    if len(genInputFiles) == 1:
        inputroot = _input_root(genInputFiles[0], keep_suffix_after_underscore=False)
        realEventsFile = _find_unique_file(f"*{inputroot}.*ev*ts")
        if is_lhe_input:
            # Compress or symlink the extracted input according to the flag.
            _prepare_lhe_file(realEventsFile, eventsFile)
            return _count_lhe_events(eventsFile)
        _mk_symlink(realEventsFile, eventsFile)
        return None

    allFiles = []
    for file in genInputFiles:
        # Since we can have multiple files from the same task, include more of the filename
        # to make the lookup unique in the plain-file case.
        inputroot = _input_root(file, keep_suffix_after_underscore=True)
        evgenLog.info("inputroot = %s", inputroot)
        realEventsFile = _find_unique_file(f"*{inputroot}.*ev*ts")
        # The only input format where merging is permitted is LHE.
        with open(realEventsFile, "r") as f:
            first_line = f.readline()
            if "LesHouches" not in first_line:
                raise RuntimeError(f"{realEventsFile} is NOT a LesHouches file")
        allFiles.append(realEventsFile)
    _merge_lhe_files(allFiles, eventsFile)

    return _count_lhe_events(eventsFile)


def _read_production_parameters(jo_dir):
    """
    Read the required production settings from the compulsory 
    production_parameters.yaml in the jO directory.
    """
    import yaml

    path = os.path.join(jo_dir, "production_parameters.yaml")
    try:
        with open(path, encoding="utf-8") as file:
            yaml_content = yaml.safe_load(file)
    except FileNotFoundError:
        raise RuntimeError(
            f"Required production parameters file not found: {path}. "
            "Put production_parameters.yaml beside the jO."
        )
    except (OSError, yaml.YAMLError) as exc:
        raise RuntimeError(f"Cannot read production parameters from {path}: {exc}")

    if not isinstance(yaml_content, dict) or "job_parameters" not in yaml_content:
        raise RuntimeError(f"{path} must contain a job_parameters list")

    job_parameters = yaml_content["job_parameters"]
    if (not isinstance(job_parameters, list) or len(job_parameters) != 1
            or not isinstance(job_parameters[0], dict)):
        raise RuntimeError(f"job_parameters in {path} must be a list containing one parameter mapping")

    parameters = dict(job_parameters[0])
    if "nEventsPerJob" not in parameters:
        raise RuntimeError(f"Required parameter nEventsPerJob is missing from job_parameters in {path}")
    parameters.setdefault("inputFilesPerJob", 0)

    for name, minimum in (("nEventsPerJob", 1), ("inputFilesPerJob", 0)):
        value = parameters[name]
        if type(value) is not int or value < minimum:
            raise RuntimeError(f"{name} in {path} must be an integer >= {minimum}")

    return {name: parameters[name] for name in ("nEventsPerJob", "inputFilesPerJob")}


def _validate_sample_properties(sample, flags):
    """Helper function to validate and set sample properties"""
    # Required fields with lightweight, explicit validators.
    required_rules = {
        "keywords": lambda v: isinstance(v, list) and len(v) > 0,
        "contact": lambda v: isinstance(v, list) and len(v) > 0,
    }
    for field, validator in required_rules.items():
        value = getattr(sample, field, None)
        if not validator(value):
            raise RuntimeError(f"self.{field} should be set in Sample(EvgenConfig)")

    input_files_per_job = flags.Generator.inputFilesPerJob
    me_generator = getattr(sample, "MEgenerator", None)

    if input_files_per_job > 0 and not me_generator:
        raise RuntimeError("self.MEgenerator should be set when inputFilesPerJob > 0 in production_parameters.yaml")
    if input_files_per_job == 0 and me_generator:
        raise RuntimeError("self.MEgenerator should be empty when inputFilesPerJob == 0 in production_parameters.yaml")


def _is_txt_only_run(flags):
    """Helper function to determine if this is LHE-only generation with no showering)"""
    has_txt = bool(flags.Output.TXTFileName)
    has_evnt = bool(flags.Output.EVNTFileName)
    has_yoda =bool(flags.Generator.outputYODAFile)
    
    return has_txt and not has_evnt and not has_yoda


def _get_nevents(flags, safety):
    """Helper function to determing number of events to be generated
    based on MaxEvents or nEventsPerJob and a user-provided safety
    factor to allow for failures in showering stage."""
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


class BeamType(Enum):
    """Helper class for storing the information
    on beam type: (Z, A, atomic mass in u).
    Masses are taken from AME2020 atomic mass evaluation
    (DOI: 10.1088/1674-1137/abddaf).
    """
    
    PROTON = (1, 1, 1.00782503190)   # 1H
    OXYGEN = (8, 16, 15.9949146193)  # 16O
    NEON = (10, 20, 19.9924401753)   # 20Ne
    XENON = (54, 129, 128.904780857) # 129Xe
    LEAD = (82, 208, 207.976652)     # 208Pb
 
    def __init__(self, Z, A, atomic_mass):
        self.Z = Z
        self.A = A
        self.atomic_mass = atomic_mass # For STARlight

