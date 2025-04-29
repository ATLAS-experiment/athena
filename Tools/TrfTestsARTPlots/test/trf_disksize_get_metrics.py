import ROOT
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import re
import awkward as ak
import uproot
import os
import warnings

class Units(object):
    kb = 1024.0
    Mb = 1024.0 * 1024.0
    Gb = 1024.0 * 1024.0 * 1024.0

# uproot branch heandler
def _remove_not_interpretable(branch):
    if isinstance(
        branch.interpretation, uproot.interpretation.identify.uproot.AsGrouped
    ):
        for name, interpretation in branch.interpretation.subbranches.items():
            if isinstance(
                interpretation, uproot.interpretation.identify.UnknownInterpretation
            ):
                return False
    if isinstance(
        branch.interpretation, uproot.interpretation.identify.UnknownInterpretation
    ):
        return False

    try:
        _ = branch.interpretation.awkward_form(None)
    except uproot.interpretation.objects.CannotBeAwkward:
        return False
    else:
        return True

# convert branch type into more readable format
def parse_type(name, typeName, ifparent):
    pattern = re.compile(r"Aux.*Base|Base.*Aux", re.IGNORECASE)
    if pattern.search(typeName):
        return "aux.store"
    if pattern.search(name):
        return "aux.store"
    if typeName == "BASE":
        return "aux.store"
    pattern = re.compile(
        r"vector\s*<\s*vector\s*<\s*ElementLink\s*<\s*DataVector\s*<.*?>\s*>\s*>"
    )
    if pattern.search(typeName):
        return r"nested vector(ElementLinks)"
    pattern = re.compile(r"\b\w*_v1\b", re.IGNORECASE)
    if pattern.search(typeName):
        return "interface"
    pattern = re.compile(r"vector\s*<\s*vector\s*<(.*?)>\s*>", re.IGNORECASE)
    if pattern.search(typeName):
        return re.sub(
            r"vector\s*<\s*vector\s*<(.*?)>\s*>", r"nested vector(\1)", typeName
        )
    pattern = re.compile(
        r"vector\s*<\s*pair\s*<\s*(.*?)\s*,\s*(.*?)\s*>\s*>", re.IGNORECASE
    )
    if pattern.search(typeName):
        return re.sub(
            r"vector\s*<\s*pair\s*<\s*(.*?)\s*,\s*(.*?)\s*>\s*>",
            r"vector pair(\1, \2)",
            typeName,
        )
    return 0

# get metrics dataframe 
def calculate_metrics(tree, up_tree):
    global nEntries
    nEntries = tree.GetEntries()  

    data = {
        "branch": [],
        "typeName": [],
        "items": [],
        "vars": [],
        "diskSize": [],
        "NEvents": [],
        "totalbytes": [],
        "subbranches": [],
    }
    # return total branch entries using uproot  
    def vars_length(branch):
        if branch not in up_tree.keys():
            return 1
        try:
            up_branch = (
                up_tree[branch].array(library="ak")
                if _remove_not_interpretable(up_tree[branch])
                else None
            )
        except (
            ValueError,
            uproot.interpretation.objects.CannotBeForth,
            uproot.deserialization.DeserializationError,
        ) as e:
            print(f"Skipping branch {branch} due to error: {e}")
            return 1
        except Exception as e:
            print(f"Unexpected error in branch {branch}: {e}")
            return 1

        if up_branch is None or len(up_branch) == 0:
            return 1
        elif up_branch.ndim == 1:
            return len(up_branch) / nEntries
        else:
            return ak.sum(ak.num(up_branch)) / nEntries
        
    # extract size info for branches  
    def process_branch(branch, parent_name=""):
        branch_name = branch.GetName()
        full_name = branch_name
        branches_list = branch.GetListOfBranches()
        leaves_list = branch.GetListOfLeaves()

        typeName = None
        if parent_name and leaves_list and leaves_list.At(0):
            typeName = leaves_list.At(0).GetTypeName()
        else:
            typeName = branch.GetClassName()

        if not typeName and leaves_list and leaves_list.At(0):
            typeName = leaves_list.At(0).GetTypeName()

        vars_count = len(branches_list)
        tp = parse_type(branch_name, typeName, 0)
        if tp != 0:
            typeName = tp

        subbranches_list = [sub_branch.GetName() for sub_branch in branches_list]

        if branch.GetZipBytes() > 0:
            data["branch"].append(full_name)
            data["NEvents"].append(nEntries)
            data["totalbytes"].append(branch.GetTotBytes())
            data["diskSize"].append(branch.GetZipBytes() / Units.kb)
            data["typeName"].append(typeName)
            data["vars"].append(vars_count)
            data["subbranches"].append(subbranches_list)
            data["items"].append(vars_length(branch_name)) if up_tree!=1 else data["items"].append("n/a")

        # Process sub-branches
        for sub_branch in branches_list:
            process_branch(sub_branch, full_name)

    for branch in tree.GetListOfBranches():
        process_branch(branch)

    return pd.DataFrame(data)

# add containers info, artificially adds DynAux into the container, as side effect the container exists for the nonAux branches such as McChanels. To be fixed
def add_container_info(data):
 auxvarptn = re.compile( r"Aux(?:Dyn)?(?:\.|:)" )
 for row in data.itertuples(index=True, name="Row"):
   m = auxvarptn.search(row.branch)
   contname=row.branch
   if m:
      contname = row.branch[:m.start()]
   contname=f"{contname}_container"
   if contname not in data["branch"].str.strip().values:
      new_cont = pd.DataFrame([{
                "branch": contname,
                "typeName": "singleton",
                "vars": 1,
                "diskSize": row.diskSize,
                "NEvents": row.NEvents,
                "totalbytes": row.totalbytes,
                "subbranches": [row.branch],
                "items": row.items
            }])
      data=pd.concat([data, new_cont], ignore_index=True)
   else:
       data.loc[data['branch'] == contname, ["vars", "diskSize", "totalbytes"]] += [1, row.diskSize, row.totalbytes] 
       data.loc[data['branch'] == contname, "subbranches"] = data.loc[data['branch'] == contname, "subbranches"].apply(lambda x: x + [row.branch] if isinstance(x, list) else [row.branch])
   if row.items=="n/a":
        data.loc[data['branch'] == contname, "typeName"] = "n/a"
   elif pd.to_numeric(row.items, errors="coerce") > 1:
        data.loc[data['branch'] == contname, "typeName"] = "collection"
 return data

# process physlite trees to dataframes
def get_df(file_name,isuproot):

    root_file = ROOT.TFile.Open(file_name)
    if not root_file or root_file.IsZombie():
        print(f"Cannot open file: {file_name}")
        return
    
    filename = os.path.basename(file_name)
    save_path = f"physlite_disksize/{filename}"
    os.makedirs(save_path, exist_ok=True)
    file_size = os.path.getsize(file_name)/Units.kb
    # Write info to output file
    with open(f"physlite_disksize/{filename}.out", "w") as out:
        out.write(f"Size: {file_size}\n")   
    up_file = uproot.open(file_name) if isuproot else 1
    tree_names = ["CollectionTree", "POOLCollectionTree", "POOLContainer", "MetaData"]
    output_files = ["EventData.csv.gz", "EventTag.csv.gz", "DataHeader.csv.gz", "MetaData.csv.gz"]

    for tree_name, output_file in zip(tree_names, output_files):
        tree = root_file.Get(tree_name)
        up_tree = up_file[tree_name] if isuproot else 1

        if not tree or not up_tree:
            print(f"Skipping {tree_name}, not found in file")
            continue

        df = calculate_metrics(tree, up_tree)
        df = add_container_info(df)
        df = df.sort_values(by="diskSize")
        df.to_csv(os.path.join(save_path, output_file), compression="gzip", index=False)

    root_file.Close()


def main():
    import argparse
    parser = argparse.ArgumentParser()
    parser.add_argument("--inputFile", type=str)
    parser.add_argument("--isuproot", type=bool)# uproot may take quite time if large file, only provides info for overall entries in the branch

    args = parser.parse_args()
    get_df(args.inputFile,args.isuproot)


main()
