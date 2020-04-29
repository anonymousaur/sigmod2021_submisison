import numpy as np
import networkx as nx
from collections import defaultdict
import sys

class CorrGraph(object):
    # Items in col2 are mapped to col1, so col1 is in the index.
    def __init__(self, col1, col2):
        assert len(col1) == len(col2)
        self.npts = len(col1)
        self.dims = (len(set(col1)), len(set(col2)))
        self.edges0 = [defaultdict(int) for _ in range(self.dims[0])]
        self.edges1 = [defaultdict(int) for _ in range(self.dims[1])]
        for (c1, c2) in zip(col1, col2):
            self.edges0[c1][c2] += 1
            self.edges1[c2][c1] += 1
        self.outliers = 0
        self.edge_cut_scores = []
        self.inverse_locs = []

    def clone_edge_graph(self):
        g = {}
        # Clones the edge graph and returns a dictionary mapping nodes in the mapped set (col2) to
        # the target set (col1).
        for i, e0 in enumerate(self.edges1):
            n = []
            for k, _ in e0.items():
                n.append(k)
            g[i] = n
        return g

    def name(self, node, side=0):
        return 'c%d:%d' % (side, node)

    # The fraction of nodes in the other set this node is connected to
    def locality(self, node, side=0):
        eset = self.edges0 if side == 0 else self.edges1
        return len(eset[node]) / float(self.dims[1-side])

    def scan_overhead(self, outliers=0, strategy="pts"):
        vals = []
        for eset in self.edges1:
            correct = 0
            scanned = 0
            for v, c in eset.items():
                scanned += sum(self.edges0[v].values())
                correct += c
            val = scanned + outliers - correct
            if strategy == "overhead":
                val = float(scanned + outliers) / correct
            vals.append(val)
        return np.mean(vals)

    def get_edges(self):
        return self.edges0, self.edges1

    # Strategy is "pts", which minimizes the number of points scanned, and "overhead", which
    # minimizes the average scan overhead.
    # Assumes we are mapping from side to the opposite.
    # Returns an edge (side 0 node, side 1 node)
    def init_edge_cut(self, strategy="pts"):
        for i, eset in enumerate(self.edges0):
            s, e = self.recompute_score(i, strategy=strategy)
            if s is None:
                s = 1 << 50
            self.edge_cut_scores.append(s)
            self.inverse_locs.append((i, e))
        self.edge_cut_scores = np.array(self.edge_cut_scores, dtype=int)

    def cut_best_edge(self, strategy="pts"):
        ix = np.argmax(self.edge_cut_scores)
        s = self.edge_cut_scores[ix]
        e = self.inverse_locs[ix]
        if e[1] is None:
            return None, None
        self.cut(e)
        new_score, edge = self.recompute_score(e[0])
        if new_score is None:
            new_score = 1 << 50
        self.edge_cut_scores[ix] = new_score
        self.inverse_locs[ix] = (ix, edge)
        return e, s

    # Modifies the graph in place and adds to the outlier buffer.
    def cut(self, edge):
        s = self.edges0[edge[0]][edge[1]]
        self.outliers += s
        del self.edges0[edge[0]][edge[1]]
        del self.edges1[edge[1]][edge[0]]
        # Recompute the scores for this edge

    def recompute_score(self, node, strategy="pts"):
        mv, mk = None, None
        tot = 0
        if len(self.edges0[node]) == 0:
            return None, None
        for k, v in self.edges0[node].items():
            if mv is None or v < mv:
               mv, mk = v, k
            tot += v 
        score = tot - mv if strategy == "pts" else tot / float(mv)
        return score, mk 

    def compute_tradeoff(self, strategy="pts", max_outlier_frac=0.1):
        data_x = []
        data_y = []
        i = 0
        tot_cutsize = 0
        self.init_edge_cut()
        while True:
            so = self.scan_overhead(outliers=self.outliers, strategy=strategy)
            data_x.append(float(self.outliers)/self.npts)
            data_y.append(so)
            ecut, score = self.cut_best_edge(strategy)
            if ecut is None:
                break
            if self.outliers >= max_outlier_frac*self.npts:
                break
            sys.stdout.write("\r%d" % self.outliers)
            sys.stdout.flush()
        return data_x, data_y 

    def minimize_overhead(self, strategy="pts", max_outlier_frac=0.1):
        best_so = 1 << 50
        best_so_graph = None
        best_outliers = 0
        self.init_edge_cut()
        while True:
            so = self.scan_overhead(outliers=self.outliers, strategy=strategy)
            if so < best_so:
                best_so = so
                best_outliers = self.outliers
                best_so_graph = self.clone_edge_graph()
            ecut, score = self.cut_best_edge(strategy)
            if ecut is None:
                break
            if self.outliers >= max_outlier_frac*self.npts:
                break
        return best_so_graph, best_so, best_outliers

