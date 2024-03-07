#!/usr/bin/env python3

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

import argparse
import ROOT
import re
import typing

def iterate(thisdir: ROOT.TDirectory, targetdir: ROOT.TDirectory, prefix: str, regex: typing.Pattern,
            excludeTrees: bool):
    """Recurse to copy all objects except those of regex"""
    for k in thisdir.GetListOfKeys():
        if k.GetClassName().startswith('TDirectory'):
            targetsubdirname = thisdir.GetPath()[len(prefix):] + '/' + k.GetName()
            if regex.fullmatch(targetsubdirname):
                print('Now skipping', targetsubdirname)
                continue
            targetsubdir = targetdir.mkdir(k.GetName())
            iterate(o := k.ReadObj(), targetsubdir, prefix, regex, excludeTrees)
            ROOT.SetOwnership(o, True)
        else:
            if k.GetClassName().startswith('TTree'):
                if not args.excludeTrees or k.GetName() == 'metadata':
                    targetdir.WriteObject(o := k.ReadObj().CloneTree(), k.GetName())
                    ROOT.SetOwnership(o, True)
            else:
                targetdir.WriteObject(o := k.ReadObj(), k.GetName())
                ROOT.SetOwnership(o, True)

if __name__ == '__main__':

    parser = argparse.ArgumentParser()
    parser.add_argument('infile')
    parser.add_argument('outfile')
    parser.add_argument('--excludeRegex', default='^run_\d+/lb_\d+')
    parser.add_argument('--excludeTrees', default=True)

    args = parser.parse_args()

    infile = ROOT.TFile.Open(args.infile, "READ")
    outfile = ROOT.TFile.Open(args.outfile, "RECREATE")
    regex = re.compile(args.excludeRegex)
    iterate(infile, outfile, infile.GetPath(), regex, args.excludeTrees)
    outfile.Write()
