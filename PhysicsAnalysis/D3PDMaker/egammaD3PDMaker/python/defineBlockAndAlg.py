# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#
# @file egammaD3PDMaker/python/defineBlockAndAlg.py
# @author scott snyder <snyder@bnl.gov>
# @date Nov, 2011
# @brief Helper to schedule an algorithm only if a block is included.
#


def defineAlgLODFunc (level, alg):
    """Return a level-of-detail function that also schedules an algorithm.

    LEVEL is the level of detail.

    ALG is the name of the algorithm.
"""

    def lodfunc (reqlev, blockargs, hookargs):
        if reqlev < level: return False
        algmod = __import__ ('egammaD3PDAnalysis.' + alg + 'Config',
                             fromlist = [alg + 'Config'])
        algfunc = getattr (algmod, alg + 'Cfg')
        def hookfn (c, flags, acc, *args, **kw):
            acc.merge (algfunc (flags,
                                prefix = hookargs['prefix'],
                                sgkey = hookargs['sgkey'],
                                typeName = hookargs['typeName'],
                                allowMissing = hookargs.get('allowMissing',False)))
            return
        hookargs['d3pdo'].defineHook (hookfn)
        return True
    return lodfunc

def defineBlockAndAlg (d3pdo, level, blockname, blockfunc, alg,
                       **kw):
    """Define a block, and schedule an algorithm if the block is used.

    D3PDO is the D3PDObject for which the block should be defined.

    LEVEL is the level of detail.

    BLOCKNAME is the name of the block.

    BLOCKFUNC is the function that creates the block.

    ALG is the name of the algorithm.
"""

    lodfunc = defineAlgLODFunc (level, alg)
    d3pdo.defineBlock (lodfunc, blockname, blockfunc, **kw)
    return
