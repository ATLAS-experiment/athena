# Copyright (C) 2002-2025 CERN for the benefit of the ATLAS collaboration.
#
# File: share/xAODRootTest.py
# Author: snyder@bnl.gov
# Date: Jun 2014
# Purpose: Test reading xAOD objects directly from root.
#

# Work around library loading order issue seen with the LTO build.
# Otherwise we can get inconsistent resolution of a static std::string,
# leading to a free() failure during exit().
import ROOT
if ROOT.gSystem.FindDynamicLibrary (cppyy.gbl.TString ("libGaudiKernel"), True):
    ROOT.gSystem.Load("libGaudiKernel")

import cppyy

import sys
cppyy.load_library("libDataModelTestDataCommonDict")
if 'LOAD_WRITE_DIR' in globals():
    cppyy.load_library("libDataModelTestDataWriteDict")
else:
    cppyy.load_library("libDataModelTestDataReadDict")

def CHECK(sc):
    if not sc.isSuccess():
        raise Exception ('bad StatusCode')
    return



reg=ROOT.SG.AuxTypeRegistry.instance()

def _typename(t):
    return getattr (t, '__cpp_name__', t.__name__)

def xAODInit():
    ROOT.xAOD.TEvent
    CHECK(ROOT.xAOD.Init())
    return



cvec_cls=ROOT.DataVector(ROOT.DMTest.C_v1)
cel_cls=ROOT.ElementLink(cvec_cls)



def format_int(x): return '%d'%x
def format_float(x): return '%.1f'%x
def format_str(x): return f"'{x}'"
def format_el(x): return '%s[%s]' % (x.dataID(), ('inv' if x.isDefaultIndex() else x.index()))
def format_el_vec(v): return '[' + ', '.join([format_el(el) for el in v.asVector()]) + ']'
def format_float_vec(v):
    l = [format_float(x) for x in v]
    return '[' + ','.join(l) + ']'
def format_int_vec(v):
    l = [format_int(x) for x in v]
    return '[' + ','.join(l) + ']'
def format_str_vec(v):
    l = [format_str(x) for x in v]
    return '[' + ','.join(l) + ']'
CVec_type = ROOT.DataVector(ROOT.DMTest.C_v1)
accessors = {
    'int'   :  (ROOT.SG.ConstAccessor(int), format_int),
    'unsigned int'   :  (getattr (ROOT, 'SG::ConstAccessor<unsigned int>'), format_int),
    'float' :  (ROOT.SG.ConstAccessor(float), format_float),
    'std::vector<float>' : (getattr (ROOT, 'SG::ConstAccessor<std::vector<float> >'), format_float_vec),
    'std::vector<int>' : (getattr (ROOT, 'SG::ConstAccessor<std::vector<int> >'), format_int_vec),
    'ElementLink<DataVector<DMTest::C_v1> >' :
               (ROOT.SG.ConstAccessor(cel_cls), format_el),
    'SG::JaggedVecElt<int>' :
    (ROOT.SG.ConstAccessor(ROOT.SG.JaggedVecElt(ROOT.int)),
     format_int_vec),
    'SG::JaggedVecElt<float>' :
    (ROOT.SG.ConstAccessor(ROOT.SG.JaggedVecElt(ROOT.float)),
     format_float_vec),
    'SG::JaggedVecElt<double>' :
    (ROOT.SG.ConstAccessor(ROOT.SG.JaggedVecElt(ROOT.double)),
     format_float_vec),
    'SG::JaggedVecElt<std::string>' :
    (ROOT.SG.ConstAccessor(ROOT.SG.JaggedVecElt(ROOT.std.string)),
     format_str_vec),
    'SG::JaggedVecElt<ElementLink<DataVector<DMTest::C_v1> > >' :
    (ROOT.SG.ConstAccessor(ROOT.SG.JaggedVecElt(ROOT.ElementLink(CVec_type))),
     format_el_vec),
    'SG::PackedLink<DataVector<DMTest::C_v1> >' :
    (ROOT.SG.ConstAccessor(ROOT.SG.PackedLink(ROOT.DataVector(ROOT.DMTest.C_v1))),
     format_el),
    'std::vector<SG::PackedLink<DataVector<DMTest::C_v1> > >' :
    (ROOT.SG.ConstAccessor(ROOT.std.vector(ROOT.SG.PackedLink(ROOT.DataVector(ROOT.DMTest.C_v1)))),
     format_el_vec),
    }

def dump_auxitem (x, auxid, f = sys.stdout):
    tname = reg.getTypeName (auxid)
    ac_p = accessors.get (tname)
    if not ac_p:
        print ('<unknown %s>'%tname, end='', file=f)
    else:
        (ac_cl, formatter) = ac_p
        val = ac_cl(reg.getName(auxid))(x)
        print (formatter(val) + '; ', end='', file=f)
    return


def dump_auxdata (x, exclude=[], f = sys.stdout):
    auxids = list(x.getAuxIDs())
    auxids = [(reg.getName(id), id) for id in auxids]
    auxids.sort()
    for name, auxid in auxids:
        if name in exclude: continue
        if reg.isLinked (auxid): continue
        print (name + ': ', file=f, end='')
        dump_auxitem (x, auxid, f)
    return


def dump_c (c, f=sys.stdout):
    if hasattr(c, '__deref__'):
        c = c.__deref__()
    print ('  anInt1: %d; aFloat: %.1f; ' % (c.anInt(), c.aFloat()), end='')
    dump_auxdata (c, exclude = ['anInt1', 'aFloat'])
    print ('')
    return


def dump_xaodobj (h, f=sys.stdout):
    if hasattr(h, '__deref__'):
        h = h.__deref__()
    dump_auxdata (h)
    print ('')
    return


def dump_plinks (p, f=sys.stdout):
    if hasattr(p, '__deref__'):
        p = p.__deref__()
    dump_auxdata (p)
    print ('')
    return


def copy_obj (event, obj, key):
    copy = obj.__class__()
    copy_aux = obj.getConstStore().__class__()
    copy.setNonConstStore (copy_aux)
    copy.__assign__ (obj)
    CHECK (event.record (copy, key))
    CHECK (event.record (copy_aux, key + 'Aux.'))
    ROOT.SetOwnership (copy, False)
    ROOT.SetOwnership (copy_aux, False)
    return


def copy_vec (event, obj, key):
    copy = obj.__class__()
    copy_aux = obj.getConstStore().__class__()
    copy.setNonConstStore (copy_aux)
    for i in range(obj.size()):
        elt_orig = obj[i]
        if _typename (elt_orig.__class__).startswith ('DataModel_detail::ElementProxy<'):
            elt_orig = elt_orig.__follow__()
        elt = elt_orig.__class__()
        copy.push_back(elt)
        ROOT.SetOwnership (elt, False)
        elt.__assign__ (elt_orig)
    CHECK (event.record (copy, key))
    CHECK (event.record (copy_aux, key + 'Aux.'))
    ROOT.SetOwnership (copy, False)
    ROOT.SetOwnership (copy_aux, False)
    return


def copy_view (event, obj, key):
    copy = obj.__class__(obj)
    copy.toPersistent()
    CHECK (event.record (copy, key))
    ROOT.SetOwnership (copy, False)
    return


class xAODTestRead:
    def __init__ (self, readPrefix = ''):
        self.readPrefix = readPrefix
        return


    def execute (self, event):
        print (self.readPrefix + 'cvec')
        vec = event[self.readPrefix + 'cvec']
        for c in vec:
            dump_c (c)

        print (self.readPrefix + 'cinfo')
        cinfo = event[self.readPrefix + 'cinfo']
        dump_c (cinfo)

        print (self.readPrefix + 'ctrig')
        ctrig = event[self.readPrefix + 'ctrig']
        for c in ctrig:
            dump_c (c)

        vec = event[self.readPrefix + 'cvecWD']
        print (self.readPrefix + 'cvecWD' + ' ', vec.meta1)
        for c in vec:
            dump_c (c)

        vec = event[self.readPrefix + 'cview']
        print (self.readPrefix + 'cview')
        for c in vec:
            dump_c (c)

        print (self.readPrefix + 'pvec')
        vec = event[self.readPrefix + 'pvec']
        for p in vec:
            dump_xaodobj (p)

        print (self.readPrefix + 'hvec')
        vec = event[self.readPrefix + 'hvec']
        for h in vec:
            dump_xaodobj (h)

        print (self.readPrefix + 'jvecContainer')
        vec = event[self.readPrefix + 'jvecContainer']
        for h in vec:
            dump_xaodobj (h)

        print (self.readPrefix + 'jvecInfo')
        jvecInfo = event[self.readPrefix + 'jvecInfo']
        dump_xaodobj (jvecInfo)

        print (self.readPrefix + 'plinksContainer')
        vec = event[self.readPrefix + 'plinksContainer']
        for h in vec:
            dump_plinks (h)

        print (self.readPrefix + 'plinksInfo')
        plinksInfo = event[self.readPrefix + 'plinksInfo']
        dump_plinks (plinksInfo)

        #vec = event[self.readPrefix + 'hview']
        #print (self.readPrefix + 'hview')
        #for h in vec:
        #    dump_h (h)

        return
    

class xAODTestCopy:
    def __init__ (self, readPrefix = '', writePrefix = None):
        self.readPrefix = readPrefix
        self.writePrefix = writePrefix
        return

    def execute (self, event):
        CHECK (event.copy (self.readPrefix + 'cvec'))
        CHECK (event.copy (self.readPrefix + 'cinfo'))
        CHECK (event.copy (self.readPrefix + 'ctrig'))
        CHECK (event.copy (self.readPrefix + 'cvecWD'))
        CHECK (event.copy (self.readPrefix + 'cview'))
        CHECK (event.copy (self.readPrefix + 'pvec'))
        CHECK (event.copy (self.readPrefix + 'hvec'))
        CHECK (event.copy (self.readPrefix + 'jvecContainer'))
        CHECK (event.copy (self.readPrefix + 'jvecInfo'))
        CHECK (event.copy (self.readPrefix + 'plinksContainer'))
        CHECK (event.copy (self.readPrefix + 'plinksInfo'))
        #CHECK (event.copy (self.readPrefix + 'hview'))

        if self.writePrefix != None:
            cinfo = event[self.readPrefix + 'cinfo']
            copy_obj (event, cinfo, self.writePrefix + 'cinfo')

            cvec = event[self.readPrefix + 'cvec']
            copy_vec (event, cvec, self.writePrefix + 'cvec')

            ctrig = event[self.readPrefix + 'ctrig']
            copy_vec (event, ctrig, self.writePrefix + 'ctrig')

            cvecwd = event[self.readPrefix + 'cvecWD']
            copy_vec (event, cvecwd, self.writePrefix + 'cvecWD')

            cview = event[self.readPrefix + 'cview']
            copy_view (event, cview, self.writePrefix + 'cview')
            
            pvec = event[self.readPrefix + 'pvec']
            copy_vec (event, pvec, self.writePrefix + 'pvec')

            hvec = event[self.readPrefix + 'hvec']
            copy_vec (event, hvec, self.writePrefix + 'hvec')

            jvec = event[self.readPrefix + 'jvecContainer']
            copy_vec (event, jvec, self.writePrefix + 'jvecContainer')

            jvecinfo = event[self.readPrefix + 'jvecInfo']
            copy_obj (event, jvecinfo, self.writePrefix + 'jvecInfo')

            plinks = event[self.readPrefix + 'plinksContainer']
            copy_vec (event, plinks, self.writePrefix + 'plinksContainer')

            plinksinfo = event[self.readPrefix + 'plinksInfo']
            copy_obj (event, plinksinfo, self.writePrefix + 'plinksInfo')

            #hview = event[self.readPrefix + 'hview']
            #copy_view (event, hview, self.writePrefix + 'hview')
            
        return


class xAODTestDecor:
    def __init__ (self, decorName, offset=0, readPrefix = ''):
        self.readPrefix = readPrefix
        self.offset = offset
        self.decor = ROOT.SG.Decorator(int)(decorName)
        return

    
    def execute (self, event):
        cvec = event[self.readPrefix + 'cvec']
        for c in cvec:
            self.decor.set(c, self.offset + c.anInt())

        ctrig = event[self.readPrefix + 'ctrig']
        for c in cvec:
            self.decor.set(c, self.offset + c.anInt())

        cinfo = event[self.readPrefix + 'cinfo']
        self.decor.set(cinfo, self.offset + cinfo.anInt())
        return

    
class xAODTestPDecor:
    def __init__ (self, decorName, offset=0, readPrefix = ''):
        self.readPrefix = readPrefix
        self.offset = offset
        self.decor = ROOT.SG.Decorator(int)(decorName)
        return

    
    def execute (self, event):
        cvec = event[self.readPrefix + 'cvec']
        assert cvec.setOption (self.decor.auxid(), ROOT.SG.AuxDataOption ('nbins', 23))
        for c in cvec:
            self.decor.set(c, self.offset + c.anInt())
        return

    
class xAODTestClearDecor:
    def __init__ (self, readPrefix = ''):
        self.readPrefix = readPrefix
        return

    
    def execute (self, event):
        cvec = event[self.readPrefix + 'cvec']
        cvec.clearDecorations()

        ctrig = event[self.readPrefix + 'ctrig']
        ctrig.clearDecorations()

        cinfo = event[self.readPrefix + 'cinfo']
        cinfo.clearDecorations()
        return

    

class AllocTestRead:
    def execute (self, event):
        cont = event['AllocTest']
        print ('AllocTest: ', end='')
        for a in cont:
            print (a.atInt1(), a.atInt2(), end=' ')
        print()
        return

    

class Analysis:
    def __init__ (self, ifname, ofname = None):
        self.algs = []
        self.f = ROOT.TFile (ifname)
        from xAODRootAccess.TPyEvent import TPyEvent
        self.event = TPyEvent (ROOT.xAOD.TEvent.kAthenaAccess)
        CHECK (self.event.readFrom (self.f, True, 'CollectionTree'))
        self.fout = None
        if ofname:
            self.fout = ROOT.TFile.Open (ofname, 'recreate')
            CHECK (self.event.writeTo (self.fout))
        return

    def add (self, alg):
        self.algs.append (alg)
        return

    def run (self, n=None):
        nent = self.event.getEntries()
        if n != None:
            nent = min (n, nent)
        for i in range(nent):
            self.event.getEntry(i)
            print ('---> Event', i)
            for a in self.algs:
                a.execute (self.event)
            if self.fout != None:
                self.event.fill()
        return

    def finalize (self):
        if self.fout:
            CHECK (self.event.finishWritingTo (self.fout))
        return
    
    
