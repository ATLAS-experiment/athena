# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

import ROOT

file = ROOT.TFile.Open("myExampleStream.pool.root")

# CollectionTree is the default TTree name for Event Data
tree = file.Get("CollectionTree")
tree.GetEntry(0)

# Find the branches containing HiveDataObj
brNames = [
    b.GetName()
    for b in tree.GetListOfBranches()
    if b.GetName().startswith("HiveDataObj")
]

print(" ---- ".join(["   Event"] + [name[12:] for name in brNames]))
# Loop over all rows (events) - get objects from the HiveDataObj branches with "getattr(tree,b)"
lineformat = "{:>8}" * (len(brNames) + 1)
for evt in range(tree.GetEntries()):
    tree.GetEntry(evt)
    print(lineformat.format(evt + 1, *[getattr(tree, b).val() for b in brNames]))
