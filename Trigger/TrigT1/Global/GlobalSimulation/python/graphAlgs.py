#  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration


class DirectedCycle:
    """determine whether a digraph has a cycle"""

    def __init__(self, G):
        self.onStack = [False for i in range(G.V)]
        self.marked = [False for i in range(G.V)]
        self.edgeTo = [-1 for i in range(G.V)]
        self.cycle_ = []
        
        GV = G.V
        for v in range(GV):
            if not self.marked[v]:
                self.dfs_(G, v)

    def has_cycle(self):
        return len(self.cycle_) > 0

    def cycle(self):
        return self.cycle_

    def dfs_(self, G, v):
        self.onStack[v] = True
        self.marked[v] = True

        for w in G.adj(v):
            if self.cycle_: return
            if not self.marked[w]:
                self.edgeTo[w] = w
                self.dfs_(G, w)
            elif self.onStack[w]:
                x = v
                while x != w:
                    self.cycle_.append(x)
                    x = self.edgeTo(x)
                self.cycle_.append(w)
                self.cycle_.append(v)

        self.onStack[v] = False
                
        

class DepthFirstOrder:
    """Carry out a depth first traversal of a graph, noting the order
    nodes are processed."""

    def __init__(self, G, roots = []):
        self.marked = [False for v in range(G.V)]
        self.pre_ = []
        self.post_ = []
        self.reversePost_ = []
        
        if not roots:
            for v in range(G.V):
                if not self.marked[v]:
                    self.dfs_(G, v)
        else:
            for v in roots:
                if not self.marked[v]:
                    self.dfs_(G, v)

        self.reversePost_ = self.post_[:]
        self.reversePost_.reverse()
        
    def pre(self): return self.pre_
    def post(self): return self.post_
    def reversePost(self): return self.reversePost_

    def dfs_(self, G, v):
        self.pre_.append(v)
        self.marked[v] = True

        for w in G.adj(v):
            if not self.marked[w]:
                self.dfs_(G, w)
                
    
        self.post_.append(v)
    

class Topological:
    """order the nodes of a digraph in topological (i.e. execution) order."""

    def __init__(self, G, roots=[]):
        self.marked = [False for i in range(G.V)]
        self.order_ = []
        cycleFinder = DirectedCycle(G)
        if not cycleFinder.has_cycle():
            dfs = DepthFirstOrder(G, roots)

            # if all edges were reversed, this would be
            # self.order = dfs.reversePost();
            self.order_ = dfs.post()
        
    def order(self):
        return self.order_

    def isDAG(self):
        return len(self.order_) != 0
