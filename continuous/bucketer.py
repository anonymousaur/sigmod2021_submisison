import numpy as np
import time

class Schema(object):
    def __init__(self, dim, num_buckets=None, bucket_width=None, bucket_ids=None, categorical=False, mode='flattened'):
        valid = int(num_buckets is not None)
        valid += int(bucket_width is not None)
        valid += int(categorical)
        valid += int(bucket_ids is not None)
        assert valid == 1, \
                "Exactly one of num_buckets, bucket_width, bucket_ids, and categorical can be specified"
        self.dim = dim
        self.num_buckets = num_buckets
        self.bucket_width = bucket_width
        self.categorical = categorical
        self.mode = mode
        self.bucket_ids = bucket_ids

    # Given some bucket ids, if there are gaps (e.g., there is no point with bucket 10), shift the
    # bucket IDs so there are no gaps
    def sequentialize(self, buckets):
        start = time.time()
        _, inv = np.unique(buckets, return_inverse=True)
        end = time.time()
        print("Sequentializing took %.02fs" % (end-start))
        return inv

    def bucketize_with_nbuckets(self, data):
        buckets = None
        if mode == 'flattened':
            ptls = np.linspace(0, 100, s[1]+1)
            buckets = np.percentile(data, ptls)
        elif mode == 'equispaced':
            buckets = np.linspace(data.min(), data.max(), s+1)
        else:
            print("Unrecognized mode %s" % mode)
        buckets[-1] = np.nextafter(data.max(), np.inf)
        return np.digitize(data, buckets[1:], right=False), buckets

    def bucketize_with_width(self, data):
        b = np.floor(data/self.bucket_width)
        bmin, bmax = b.min(), b.max()
        return b.astype(int), np.arange(bmin, bmax+2) * self.bucket_width

    def bucketize(self, data, sequentialize=False):
        buckets = None
        bounds = None
        if self.bucket_width is not None:
            buckets, bounds = self.bucketize_with_width(data)
        elif self.num_buckets is not None:
            buckets, bounds = self.bucketize_with_nbuckets(data)
        elif self.bucket_ids is not None:
            buckets, bounds = self.bucket_ids, None
        elif self.categorical:
            buckets, bounds = data, data
        if sequentialize:
            buckets = self.sequentialize(buckets)
        return buckets - buckets.min(), bounds 
    
class Bucketer(object):
    # Spec is a list of Schema objects
    def __init__(self, spec, data, mode='flattened', sequentialize=False):
        self.spec = spec
        self.ids = None
        self.ranges = None
        self.bucket(data, sequentialize)

    def bucket(self, data, sequentialize=False):
        ranges = []
        ids = np.zeros((len(data), len(self.spec)), dtype=int)
        for j, s in enumerate(self.spec):
            ids[:,j], r = s.bucketize(data[:,s.dim], sequentialize=sequentialize)
            ranges.append(r)
        folded_ids = np.zeros(ids.shape[0], dtype=int)
        scale = 1
        for j in range(ids.shape[1]):
            folded_ids += ids[:,j] * scale
            scale *= ids[:,j].max()+1
        self.ids = folded_ids
        self.ranges = ranges
        return folded_ids, ranges

    def get_ids(self):
        return self.ids

    def get_ranges(self):
        return self.ranges

    def dims(self):
        return [s.dim for s in self.spec] 
