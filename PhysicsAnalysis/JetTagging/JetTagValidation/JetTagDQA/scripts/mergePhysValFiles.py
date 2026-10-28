#!/usr/bin/env python
# Copyright (C) 2002-2026 CERN for the benefit of the ATLAS collaboration

# ----------------------------------------------------------------------
# stand-alone script to merge specific directories of NTUP_PHYSVAL files
# author: philipp.mogg@cern.ch
# 16 May 2016
# updates: judith.hoefer@cern.ch
# Nov 2020
# updates: sorting driven by the JetTagDQA naming convention (AFT-866)
# Oct 2026
# ----------------------------------------------------------------------

import argparse
import glob
import os

import ROOT

# the jet containers monitored by JetTagDQA
JETCONTAINERS = [
    "AntiKt4EMTopoJets",
    "AntiKt4EMPFlowJets",
    "AntiKtVR30Rmax4Rmin02PV0TrackJets",
    "AntiKt10UFOCSSKSoftDropBeta100Zcut10Jets",
]

# truth classes a histogram can be restricted to, as the trailing part of its name
TRUTH_CLASSES = [
    "b",
    "c",
    "u",
    "muon",
    "hbb",
    "hcc",
    "htautauhad",
    "top",
    "Wqq",
    "qcdbb",
    "qcdbx",
    "qcdcx",
    "qcdll",
]

# variables the tagger working point histograms are matched against
MATCHED_VARS = ["pt_ttbar", "pt_Zprime", "SV1Lxy", "Lxy"]

# groups that are split further into one sub-folder per selection
SPLIT_GROUPS = ["jet", "tracks", "sv1", "muon", "tagger"]

# sub-folder for the histograms of a group without a selection
INCLUSIVE = "inclusive"

# sub-folder for everything else inside a group
REST = "other"

# where histograms that do not follow the naming convention end up
UNSORTED = "other_histograms/histos"


def split_prefix(name):
    """Split a histogram name into its BTag_<container>_ prefix and the remaining stem."""
    for container in JETCONTAINERS:
        prefix = "BTag_" + container + "_"
        if name.startswith(prefix):
            return prefix, name[len(prefix):]
    return None, name


def categorise(stem):
    """Return the group and selection sub-folder a histogram stem belongs to."""
    tokens = stem.split("_")
    if tokens[0] == "tagger" and len(tokens) > 1:
        group, rest = "tagger_" + tokens[1], tokens[2:]
    else:
        group, rest = tokens[0], tokens[1:]
    if group.split("_")[0] not in SPLIT_GROUPS:
        return group, ""
    for var in MATCHED_VARS:
        if stem.endswith("_matched_" + var):
            return group, "_" + var
    if rest and rest[-1] in TRUTH_CLASSES:
        return group, "_" + rest[-1]
    return group, INCLUSIVE if group.split("_")[0] != "tagger" else REST


class Merger(object):
    """Add up the histograms of several NTUP_PHYSVAL files into a sorted output file."""

    def __init__(self, inputs, output):
        self.files = [ROOT.TFile.Open(path) for path in inputs]
        self.output = ROOT.TFile(output, "recreate")
        self.dirs = {}
        self.errors = []

    def get_dir(self, path):
        if path not in self.dirs:
            # mkdir creates the whole path but returns its top directory, so look the leaf up
            self.output.mkdir(path)
            self.dirs[path] = self.output.GetDirectory(path)
        return self.dirs[path]

    def target(self, path, name):
        """Return the output directory of one histogram."""
        prefix, stem = split_prefix(name)
        if prefix is None:
            return self.get_dir(path + "/" + UNSORTED)
        group, selection = categorise(stem)
        subpath = "/".join(part for part in [path, group, selection] if part)
        return self.get_dir(subpath)

    def merge_histogram(self, path, name):
        total = None
        for stream in self.files:
            histo = stream.Get(path + "/" + name)
            if not histo:
                self.errors.append("ERROR: Cannot find {}/{} in {}. Omitting.".format(path, name, stream.GetName()))
                continue
            if total is None:
                total = histo.Clone()
            else:
                total.Add(histo)
        if total is None:
            return
        self.target(path, name).WriteTObject(total, name)

    def merge_folder(self, path):
        print("Merging folder " + path)
        source = self.files[0].Get(path)
        if not source:
            self.errors.append("ERROR: Cannot find directory " + path + ". Omitting.")
            return
        self.get_dir(path)
        for key in source.GetListOfKeys():
            classname = ROOT.TClass.GetClass(key.GetClassName())
            if classname.InheritsFrom(ROOT.TH1.Class()):
                self.merge_histogram(path, key.GetName())
            elif classname.InheritsFrom(ROOT.TDirectory.Class()):
                self.merge_folder(path + "/" + key.GetName())

    def close(self):
        self.output.Close()
        for stream in self.files:
            stream.Close()


def collect_inputs(entries, pattern):
    """Expand the input arguments into the list of files to merge."""
    files = []
    for entry in entries:
        path = os.path.abspath(entry)
        if os.path.isdir(path):
            files.extend(sorted(glob.glob(os.path.join(path, pattern))))
        else:
            files.append(path)
    return files


def main():
    parser = argparse.ArgumentParser(description="Merge specific folder(s) in root files.")
    parser.add_argument(
        "-i",
        "--input",
        nargs="+",
        help="folder(s) holding the samples, or the files themselves (default: ./)",
        default=[os.getcwd()],
    )
    parser.add_argument(
        "-o", "--output", help="path for the output (default: ./merge.root)", default=os.getcwd() + "/merge.root"
    )
    parser.add_argument(
        "-d",
        "--dir",
        nargs="+",
        help="ROOT directory to be merged, multiple arguments are possible (default: Summary)",
        default=["Summary"],
    )
    parser.add_argument("-p", "--pattern", help='pattern of files to merge (default: "*PHYSVAL*")', default="*PHYSVAL*")
    args = parser.parse_args()

    output_file = os.path.abspath(args.output)
    inputs = collect_inputs(args.input, args.pattern)
    if not inputs:
        raise SystemExit("No input file found in " + " ".join(args.input))

    print("Target file: " + output_file)
    for path in inputs:
        print("Found input file: " + path)
        if os.path.exists(output_file) and os.path.samefile(output_file, path):
            raise SystemExit("Please make sure that the output file is not part of the input files! Stopping.")

    ROOT.TH1.AddDirectory(False)
    merger = Merger(inputs, output_file)
    for folder in args.dir:
        merger.merge_folder(folder)
    merger.close()

    if merger.errors:
        print("Summary of all errors:")
        for error in merger.errors:
            print(error)


if __name__ == "__main__":
    main()
