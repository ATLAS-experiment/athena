#! /usr/bin/env python

# Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

#  @brief Unittests for Athena MT and MP support
#  @author graeme.andrew.stewart@cern.ch

import os
import subprocess
import unittest

import logging
msg = logging.getLogger(__name__)

# Allowable to import * from the package for which we are the test suite
from PyJobTransforms.trfMTMPTools import detectAthenaThreadsProcesses, athenaMPOutputHandler, _threadsPerProcess
from PyJobTransforms.trfArgClasses import argBool, argList, argSubstepList, argFile, argSubstepInt

import PyJobTransforms.trfExceptions as trfExceptions


## Unit tests
class AthenaOptionsMTTests(unittest.TestCase):
    def setUp(self):
        os.environ.pop("ATHENA_CORE_NUMBER", "")
    
    def test_noMTMP(self):
        self.assertEqual(detectAthenaThreadsProcesses(), (0, 0, 0))
        
    def test_noMTwithArgdict(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['some', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 0))
             
    def test_MTfromArgdict(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--threads=8', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (8, 8, 0))

    def test_MTfromArgdictStep(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--threads=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (8, 8, 0))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'all': ['--threads=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (8, 8, 0))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'all': ['--threads=8', 'random', 'values'], 'A': ['--threads=4']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (4, 4, 0))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--threads=4'], 'all': ['--threads=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (4, 4, 0))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--threads=8', 'random', 'values']})}
        step='B'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 0))

    def test_MTfromArgdictEmpty(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--threads=0', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 0))

    def test_MTfromArgdictBad(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--threads=-4', 'random', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--threads=notAnInt', 'random', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--threads=4', '--threads=8', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)

    def test_MTMixed(self):
        argdict={'multithreaded': argBool(True), 'athenaopts': argSubstepList({'A': ['--threads=2']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (2, 2, 0))

    def test_MTMixedBad(self):
        argdict={'multithreaded': argBool(True), 'athenaopts': argSubstepList(['--threads=2'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'multithreaded': argBool(True), 'athenaopts': argSubstepList({'all': ['--threads=2']})}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)


class AthenaOptionsMPTests(unittest.TestCase):
    def setUp(self):
        os.environ.pop("ATHENA_CORE_NUMBER", "")
    
    def test_noMTMP(self):
        self.assertEqual(detectAthenaThreadsProcesses(), (0, 0, 0))
        
    def test_noMPwithArgdict(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['some', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 0))
             
    def test_MPfromArgdict(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--nprocs=8', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 8))

    def test_MPfromArgdictStep(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--nprocs=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 8))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'all': ['--nprocs=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 8))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'all': ['--nprocs=8', 'random', 'values'], 'A': ['--nprocs=4']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 4))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--nprocs=4'], 'all': ['--nprocs=8', 'random', 'values']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 4))

        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList({'A': ['--nprocs=8', 'random', 'values']})}
        step='B'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 0))

    def test_MPfromArgdictEmpty(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--nprocs=0', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 0))

    def test_MPfromArgdictBad(self):
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--nprocs=-4', 'random', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--nprocs=notAnInt', 'random', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'movealong': argList('nothing to see here'), 'athenaopts': argSubstepList(['--nprocs=4', '--nprocs=8', 'values'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)

    def test_MPMixed(self):
        argdict={'multiprocess': argBool(True), 'athenaopts': argSubstepList({'A': ['--nprocs=2']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (0, 0, 2))

    def test_MTMixedBad(self):
        argdict={'multiprocess': argBool(True), 'athenaopts': argSubstepList(['--nprocs=2'])}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)
        argdict={'multiprocess': argBool(True), 'athenaopts': argSubstepList({'all': ['--nprocs=2']})}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)


class AthenaThreadsPerProcessTests(unittest.TestCase):
    def setUp(self):
        os.environ["ATHENA_CORE_NUMBER"] = '8'

    def test_threadsPerProcess_notSet(self):
        self.assertEqual(_threadsPerProcess(), 0)

    def test_threadsPerProcess_setAll(self):
        self.assertEqual(_threadsPerProcess({'threadsPerProcess': argSubstepInt(8)}), 8)

    def test_threadsPerProcess_setMatching(self):
        self.assertEqual(_threadsPerProcess({'threadsPerProcess': argSubstepInt({'A': 8})}, 'A'), 8)

    def test_threadsPerProcess_setNotMatching(self):
        self.assertEqual(_threadsPerProcess({'threadsPerProcess': argSubstepInt({'A': 8})}, 'B'), 0)


class AthenaMTMPTests(unittest.TestCase):
    def setUp(self):
        os.environ["ATHENA_CORE_NUMBER"] = '8'

    def test_MTStandard(self):
        argdict={'multithreaded': argBool(True)}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (8, 8, 0))

    def test_MPStandard(self):
        argdict={'multiprocess': argBool(True)}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (0, 0, 8))

    def test_HybridStandard(self):
        argdict={'multithreaded': argBool(True), 'threadsPerProcess': argSubstepInt(4)}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (4, 4, 2))

        argdict={'multithreaded': argBool(True), 'threadsPerProcess': argSubstepInt({'A': 4})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (4, 4, 2))

        argdict={'multithreaded': argBool(True), 'threadsPerProcess': argSubstepInt({'A': 4})}
        step='B'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (8, 8, 0))

    def test_HybridAthenaopts(self):
        argdict={'athenaopts': argSubstepList(['--nprocs=8', '--threads=2', 'random', 'values'])}
        self.assertEqual(detectAthenaThreadsProcesses(argdict), (2, 2, 8))

    def test_HybridMixed(self):
        argdict={'multithreaded': argBool(True), 'athenaopts': argSubstepList({'A': ['--threads=2', '--nprocs=4']})}
        step='A'
        self.assertEqual(detectAthenaThreadsProcesses(argdict, step), (2, 2, 4))

    def test_HybridBad(self):
        argdict={'multiprocess': argBool(True), 'threadsPerProcess': argSubstepInt(2)}
        self.assertRaises(trfExceptions.TransformExecutionException, detectAthenaThreadsProcesses, argdict)


class AthenaMPOutputParseTests(unittest.TestCase):
    def setUp(self):
        # Gah, this is a pest to setup! Need to creat stub files for the mother outputs
        # and the worker outputs
        outputStruct = [('.', [], ['data15_13TeV.00267167.physics_Main.merge.RAW._lb0176._SFO-1._0001.1', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'tmp.HIST_ESD_INT', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002']),
                         ('athenaMP-workers-RAWtoESD-r2e', ['worker_3', 'worker_7', 'worker_4', 'worker_5', 'worker_2', 'worker_6', 'evt_counter', 'worker_1', 'worker_0'], []), ('athenaMP-workers-RAWtoESD-r2e/worker_3', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_7', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_4', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_5', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_2', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_6', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/evt_counter', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_1', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out']), ('athenaMP-workers-RAWtoESD-r2e/worker_0', [], ['tmp.HIST_ESD_INT', 'AthenaMP.log', 'data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002', 'eventLoopHeartBeat.txt', 'ntuple_RAWtoESD.pmon.gz', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002', 'FileManagerLog', 'PoolFileCatalog.xml.BAK', 'PoolFileCatalog.xml', 'data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002', 'data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002', 'AtRanluxGenSvc.out'])]
        for delement in outputStruct:
            try:
                os.mkdir(delement[0])
            except OSError:
                pass
            for subdir in delement[1]:
                try:
                    os.mkdir(os.path.join(delement[0], subdir))
                except OSError:
                    pass
            for fname in delement[2]:
                open(os.path.join(delement[0], fname), "w").close()
        
        with open("athenaMP-outputs-RAWtoESD-r2e", "w") as mpoutput:
            print("""<?xml version="1.0" encoding="utf-8"?>
<athenaFileReport>
  <Files OriginalName="data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002">
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_0/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_1/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_2/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_3/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_4/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_5/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_6/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
    <File description="POOL" mode="WRITE|CREATE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_7/data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002" shared="True" technology="ROOT"/>
  </Files>
  <Files OriginalName="tmp.HIST_ESD_INT">
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_0/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_1/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_2/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_3/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_4/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_5/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_6/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
    <File description="HIST" mode="WRITE" name="{CWD}/athenaMP-workers-RAWtoESD-r2e/worker_7/tmp.HIST_ESD_INT" shared="False" technology="ROOT"/>
  </Files>
</athenaFileReport>
""".format(CWD=os.getcwd()), file=mpoutput)

    def tearDown(self):
        subprocess.call(['rm -fr athenaMP* data15* tmp.*'], shell=True)
    
    def test_basicMPoutputs(self):
        dataDict = {'BS': argFile("data15_13TeV.00267167.physics_Main.merge.RAW._lb0176._SFO-1._0001.1", io="input"),
                    'ESD': argFile("data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002"),
                    'HIST_ESD_INT': argFile("tmp.HIST_ESD_INT"),
                    'DRAW_EMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002"),
                    'DRAW_EGZ': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002"),
                    'DRAW_TAUMUH': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002"),
                    'DRAW_ZMUMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002"),}
        self.assertEqual(athenaMPOutputHandler("athenaMP-outputs-RAWtoESD-r2e", "athenaMP-workers-RAWtoESD-r2e", dataDict, 8), None)
        
    def test_missingMPoutputs(self):
        dataDict = {'ESD': argFile("data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002"),
                    'HIST_ESD_INT': argFile("tmp.HIST_ESD_INT"),
                    'DRAW_EMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002"),
                    'DRAW_EGZ': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002"),
                    'DRAW_TAUMUH': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002"),
                    'DRAW_NOTHERE': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_NOTHERE.f594._lb0176._SFO-1._0002"),
                    'DRAW_ZMUMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002"),}
        self.assertRaises(trfExceptions.TransformExecutionException, athenaMPOutputHandler, "athenaMP-outputs-RAWtoESD-r2e", "athenaMP-workers-RAWtoESD-r2e", dataDict, 8)

    def test_wrongMPoutputs(self):
        dataDict = {'ESD': argFile("data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002"),
                    'HIST_ESD_INT': argFile("tmp.HIST_ESD_INT"),
                    'DRAW_EMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002"),
                    'DRAW_EGZ': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002"),
                    'DRAW_TAUMUH': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002"),
                    'DRAW_NOTHERE': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_NOTHERE.f594._lb0176._SFO-1._0002"),
                    'DRAW_ZMUMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002"),}
        self.assertRaises(trfExceptions.TransformExecutionException, athenaMPOutputHandler, "athenaMP-outputs-RAWtoESD-r2e", "athenaMP-workers-RAWtoESD-r2e", dataDict, 20)
        
    def test_wrongMPoutputDir(self):
        dataDict = {'ESD': argFile("data15_13TeV.00267167.physics_Main.recon.ESD.f594._lb0176._SFO-1._0002"),
                    'HIST_ESD_INT': argFile("tmp.HIST_ESD_INT"),
                    'DRAW_EMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EMU.f594._lb0176._SFO-1._0002"),
                    'DRAW_EGZ': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_EGZ.f594._lb0176._SFO-1._0002"),
                    'DRAW_TAUMUH': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_TAUMUH.f594._lb0176._SFO-1._0002"),
                    'DRAW_NOTHERE': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_NOTHERE.f594._lb0176._SFO-1._0002"),
                    'DRAW_ZMUMU': argFile("data15_13TeV.00267167.physics_Main.recon.DRAW_ZMUMU.f594._lb0176._SFO-1._0002"),}
        self.assertRaises(trfExceptions.TransformExecutionException, athenaMPOutputHandler, "athenaMP-outputs-RAWtoESD-r2e-missing", "athenaMP-workers-RAWtoESD-r2e", dataDict, 20)
        
        
if __name__ == '__main__':
    unittest.main()
