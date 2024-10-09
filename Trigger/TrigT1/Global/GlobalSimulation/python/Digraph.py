#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration

# Data structure adapted from Sedgewick and Wayne, Algorithms v4 p569

class Digraph:
    """Digraph is an implementation of digraphs which supports
    adding edges and reversal"""

    def __init__(self, V=0, fn=''):
        self.adjTable_ = {}
        self.E = 0
        if fn:
            self.fromFile(fn)
        else:
            self.V = V
            for i in range(V):
                self.adjTable_[i] = []

    def fromFile(self, fn):
        state = 'START'
        with open(fn, 'r') as fh:
            for ll in fh:
                l = ll.strip()
                if not l or l.startswith('#'): continue
                
                if state == 'START':
                    self.V = int(l.strip())
                    for i in range(self.V):
                        self.adjTable_[i] = list()
                    state = 'NE'
                    continue
                if state == 'NE':
                    # self.E = int(l.strip())
                    state = 'EDGES'
                    continue

                if state == 'EDGES':
                    toks = l.strip().split()
                    assert len(toks) == 2
                    v = int(toks[0])
                    w = int(toks[1])
                    self.addEdge(v, w)
                    continue
    
    def addEdge(self, v, w):
        assert v < self.V
        assert w < self.V
        self.adjTable_[v].append(w)
        self.E += 1

    def adj(self, v):
        assert v < self.V
        return self.adjTable_[v]

    def reverse(self):
        R = Digraph(self.V)
        for v in range(self.V):
            for w in self.adj(v):
                R.addEdge(w, v)
        return R

    def __str__(self):
        lines = ['Digraph. V: '+ str(self.V) + ' E: ' + str(self.E)]
        for v in range(self.V):
            for w in self.adj(v):
                lines.append('%d -> %d' % (v, w))

        return '\n'.join(lines)
