import numpy as np

# Like convolution grid, but on a single row along the mapped dimension, for a single bucket along
# the target dimension (or combination of target dimensions).
# The burden is on the caller to define what the buckets are, and send this object only the points
# within a single bucket
class SingleColumnStasher(object):
    # Counts is the number of points per bucket in the mapped dimension
    # k is the number of buckets the average query spans
    def __init__(self, counts, k):
        self.counts = counts
        self.original_counts = np.copy(counts)
        self.k = k
        self.cumul_count = np.sum(counts)
        self.init_zeros()
        self.init_benefit()
        self.removed = 0

    def init_zeros(self):
        self.zeros_left = np.zeros(len(self.counts), dtype=np.int16)
        self.zeros_right = np.zeros(len(self.counts), dtype=np.int16)
        rowsize = len(self.counts)
        z_left, z_right = -1, rowsize 
        for j in range(rowsize):
            if self.counts[j] == 0:
                self.zeros_left[j] = j-1
            else:
                self.zeros_left[j] = max(z_left, j-self.k)
                z_left = j
        for j in range(rowsize):
            if self.counts[rowsize-j-1] == 0:
                self.zeros_right[rowsize-j-1] = rowsize-j
            else:
                self.zeros_right[rowsize-j-1] = min(z_right, rowsize-j-1+self.k)
                z_right = rowsize-j-1

    def update_zeros(self, ix, k):
        self.zeros_left[ix:ix+k] = self.arange_reset + ix - 1 
        self.zeros_right[ix:ix+k] = self.arange_reset + ix + 1
        rowsize = len(self.counts)
        for i in range(ix + k, min(rowsize, ix + 2*k - 1)):
            if self.zeros_left[i] < ix + k and self.counts[i] > 0:
                self.zeros_left[i] = i - k
        for i in range(max(0, ix - k + 1), ix):
            if self.zeros_right[i] >= ix and self.counts[i] > 0:
                self.zeros_right[i] = i + k

    def init_benefit(self):
        self.benefit = np.zeros(len(self.counts)-self.k+1), dtype=int)
        self.benefit2 = np.zeros(len(self.counts)-self.k+1), dtype=np.float64)
        self.update_benefits(0, len(self.benefit))
        
    def update_benefits(self, start, end):
        # Computes the "benefit" of removing the k elements beginning at each position. The benefit
        # is the number of points we avoid scanning by removing the queries minus the number of
        # extra points we have to scan in the outlier buffer.
        tot_nq = len(self.benefit)
        for i in range(start, end):
            # Number of queries eliminated depends on how many zeros are to the left and right of
            # the edgemost non-zero elements in each k-block.
            # TODO: we can compute this a bit faster using zeros_left and zeros_right to find the
            # edgemost non-zero elements in the k-block. No need for a linear time scan.
            npts = np.sum(self.counts[i:i+self.k])
            if npts == 0:
                self.benefit[i] = 0
                self.benefit2[i] = 0
            else:
                z_left = np.min(self.zeros_left[i:i+self.k])
                z_right = np.max(self.zeros_right[i:i+self.k])
                queries_elim = z_right - z_left - self.k
                self.benefit[i] = queries_elim*self.cumul_count - tot_nq*npts
                self.benefit2[i] = float(queries_elim) / npts
                if self.cumul_count < float(npts * tot_nq) / queries_elim:
                    self.benefit2[i] = 0

    def pop_best(self):
        col_ix = self.benefit.argmax()
        ben = self.benefit[col_ix]
        if ben <= 0:
            return None, None, None
        stash_size = self.counts[col_ix:col_ix+self.k].sum()
        self.cumul_count -= stash_size
        self.counts[col_ix:col_ix+self.k] = 0
        self.update_zeros(col_ix, self.k)
        # TODO: make this faster by storing the number of eliminated queries, so we can do a one
        # time subtraction using cumul_count, which is the only non-local thing that gets updated.
        self.update_benefits(0, len(self.benefit))
                #max(0, col_ix-self.k+1),
                #min(col_ix+2*self.k-1, self.benefit.shape[1])) 
        self.removed += stash_size
        return stash_size, ben, (col_ix, col_ix+self.k)

    def scan_overhead(self):
        overhead = 0
        for i in range(len(self.benefit)):
            true_pts = self.original_counts[i:i+self.k].sum()
            is_accessed = self.counts[i:i+self.k].sum() > 0
            accessed = is_accessed * self.cumul_count
            overhead += self.removed + accessed - true_pts
        return overhead

