import numpy as np

class Bucketer(object):
    # Spec is a list of (dim, buckets) tuples, with dim being the dimension to bucket
    # and buckets being the number of buckets in that dimension
    def __init__(self, data, spec, mode='flattened'):
        self.spec = spec
        self.data = data
        self.buckets = [None] * len(spec)
        for i, s in enumerate(spec):
            self.buckets[i] = self.get_bins(data[:, s[0]], s, mode)

    def get_bins(self, data, s, mode):
        buckets = None
        if mode == 'flattened':
            ptls = np.linspace(0, 100, s[1]+1)
            buckets = np.percentile(data, ptls)
        elif mode == 'equispaced':
            buckets = np.linspace(data.min(), data.max(), s+1)
        else:
            print("Unrecognized mode %s" % mode)

        buckets[-1] = np.nextafter(data.max(), np.inf)
        return buckets

    def get_ids(self):
        ids = np.zeros((len(self.data), len(self.spec)), dtype=int)
        for j in range(ids.shape[1]):
            dim = self.spec[j][0]
            ids[:,j] = np.digitize(self.data[:,dim], self.buckets[j][1:], right=False)
        folded_ids = np.zeros(ids.shape[0], dtype=int)
        scale = 1
        for j in range(ids.shape[1]):
            folded_ids += ids[:,j] * scale
            scale *= len(self.buckets[j])
        return folded_ids

