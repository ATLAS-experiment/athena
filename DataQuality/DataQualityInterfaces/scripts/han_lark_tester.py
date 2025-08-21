#!/usr/bin/env python
# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration

# More detailed syntax checking for han files


import lark, sys
from AthenaCommon.Utils.unixtools import find_datafile

class T(lark.Transformer):
    def __init__(self):
        super().__init__()
        self.library_set = set()

    def __default_token__(self,tok):
        print(tok)
        return tok
    def referenceblock(self, tok):
        locations = [_ for _ in tok if isinstance(_, lark.Tree) and _.data == 'location']
        if len(locations) > 1:
            raise ValueError(f'More than one location given for reference {tok[0].children[0]}')
        if len(locations) > 0:
            locs = locations[0].children[0].split(',')
        else:
            locs = [_.children[0] for _ in tok if isinstance(_, lark.Tree) and _.data == 'file']
        afspaths = [_ for _ in locs if _.startswith('/afs')]
        if any(afspaths):
            raise ValueError('A reference location for a production han configuration is given with an AFS path\n'
            f'Offending path is {",".join(afspaths)} of reference {tok[0].children[0]}')
        eospaths = [_ for _ in locs if _.startswith('/eos')]
        xrootdpaths = [_ for _ in locs if _.startswith('root://')]
        if len(eospaths) !=  len(xrootdpaths):
            raise ValueError(f'Backup xrootd locations must be given for references with EOS paths for reference {tok[0].children[0]}\n'
            f'Given EOS paths are {",".join(eospaths)}\n'
            f'Given xrootd paths are {",".join(xrootdpaths)}')
        return tok


class AlgorithmNameInterpreter(lark.visitors.Interpreter):
    def __init__(self):
        super().__init__()

    def algorithmblock(self, tree: lark.Tree):
        thisalgname = [_.children[0].value for _ in tree.children if isinstance(_, lark.Tree) and _.data == "algorithmname"][0]
        thisalgrealname = [_ for _ in tree.children if isinstance(_, lark.Tree) and _.data == "realname"]
        if not thisalgrealname:
            raise ValueError(f"no underlying algorithm specified for {thisalgname}")
        for node in tree.children:
            if node.data == 'algorithmblock':
                for subnode in node.children:
                    if subnode.data == 'algorithmname':
                        subnode.children[0].value = thisalgname + '/' + subnode.children[0].value
                if not [_ for _ in node.children if isinstance(_, lark.Tree) and _.data == "realname"]:
                    node.children.append(thisalgrealname[0])
                self.visit(node)

    def dirblock(self, tree: lark.Tree):
        thisdirname = [_.children[0].value for _ in tree.children if isinstance(_, lark.Tree) and _.data == "dirname"][0]
        for node in tree.children:
            if node.data == 'dirblock':
                for subnode in node.children:
                    if subnode.data == 'dirname':
                        subnode.children[0].value = thisdirname + '/' + subnode.children[0].value
                self.visit(node)
            elif node.data == 'histblock':
                for subnode in node.children:
                    if subnode.data == 'histname':
                        subnode.children[0].value = thisdirname + '/' + subnode.children[0].value


grammarfile = find_datafile('DataQualityInterfaces/han_def.lark')
if grammarfile is None:
    raise OSError('Unable to find file parser configuration file')
grammar = open(grammarfile).read()
transformer = T()
parser = lark.Lark(grammar, parser='lalr', lexer='contextual', transformer=transformer)


def algorithm_check(tree):
    import ROOT
    import cppyy

    ROOT.gSystem.Load('libdqm_core.so')
    cppyy.include("dqm_core/AlgorithmManager.h")

    cppyy.cppdef(r"""
#include <dqm_core/AlgorithmManager.h>
std::pair<std::vector<std::string>, std::vector<std::string>> get_lib_algs() {
std::vector<std::string> rv1;
std::vector<std::string> rv2;
for (const auto& p : dqm_core::AlgorithmManager::instance().getAlgorithmMap()) {
  rv1.push_back(p.first);
}
for (const auto& p : dqm_core::AlgorithmManager::instance().getSummaryMakerMap()) {
  rv2.push_back(p.first);
}
return std::move(std::make_pair(rv1, rv2));
}
"""
    )

    rv = True
    try:
        AlgorithmNameInterpreter().visit(tree)
    except ValueError as e:
        print(f'ERROR: {e}')
        return False
    libs = {_.children[0].value for _ in tree.find_data('libname')}  # Defined libraries
    compalgs = {_.children[0].value for _ in tree.find_data('compalgname')}  # Defined composite algorithms
    
    # Check if the referenced libraries exist
    for lib in libs:
        if ROOT.gSystem.Load(lib) < 0:
            rv = False
            print(f'ERROR: Library {lib} defined by a libname clause cannot be found. Referenced by:')
            for alg in tree.find_data('algorithmblock'):
                alibname = alg.find_data('libname')
                if alibname and lib in {_.children[0].value for _ in alibname}:
                    print(list(alg.find_data('algorithmname'))[0].children[0].value)

    # Extract the list of "base" algorithms and summaries
    libalgos, libsummaries = ROOT.get_lib_algs()

    # Check that the composite algorithms reference existing base algorithms (and not summaries!)
    for alg in tree.find_data('compalgblock'):
        subalglist = [_ for _ in alg.children if _.data == 'subalglist']
        for tok in subalglist[0].children:
            if tok.value not in libalgos:
                algname = list(alg.find_data('compalgname'))[0].children[0].value
                print(f'ERROR: composite algorithm {algname} references {tok.value} which is not a known algorithm')
                rv = False

    # Check that all the instantiated algorithms and summaries refer to existing base algorithms and summaries
    instalgos = set()
    instsummaries = set()
    for alg in tree.find_data('algorithmblock'):
        try:
            algname = [_.children[0].value for _ in alg.children if isinstance(_, lark.Tree) and _.data == "algorithmname"][0]
            algrealname = [_.children[0].value for _ in alg.children if isinstance(_, lark.Tree) and _.data == "realname"][0]
            if algrealname in libalgos:
                instalgos.add(algname)
            elif algrealname in libsummaries:
                instsummaries.add(algname)
            elif algrealname in compalgs:
                instalgos.add(algname)
            else:
                print(f'ERROR: undefined base algorithm {algrealname} for {algname}')
                rv = False
        except Exception as e:
            print('??? problem processing', alg, e)

    # These SHOULD just be summaries
    for ref in tree.find_data('algorefstatement'):
        nodealg = ref.children[0].children[0].value
        if nodealg not in instsummaries:
            print(f'ERROR: an output is specifying algorithm = {nodealg} which is not a known summary algorithm')
            rv = False
    
    # These SHOULD just be non-summary algos
    for ref in list(tree.find_data('dirblock')) + list(tree.find_data('histblock')):
        nodealg = [_ for _ in ref.children if _.data == 'algorithmreference']
        if nodealg:
            nodealgname = nodealg[0].children[0].value
            if nodealgname not in instalgos:
                refname = [_ for _ in ref.children if _.data in ('histname', 'dirname')][0].children[0].value
                print(f'ERROR: a dir/hist is specifying algorthm = {nodealgname} which is not a known histogram algorithm. Referenced by {refname}')
                rv = False

    return rv


infile = open(sys.argv[1]).read()
try:
    tree = parser.parse(infile)
    print('Tree done')
    print('Checking algorithms')
    if algorithm_check(tree):
        print(tree.pretty())
        print('All ok')
    else:
        sys.exit(1)

except Exception as e:
    print('ERROR:')
    print(e)
    raise e
    sys.exit(1)
