import numpy as np

class MappingBucket(object):
    def __init__(self, bstart, bend, ranges):
        self.start = bstart
        self.end = bend
        self.ranges = ranges

    def to_string(self):
        s = str(self.start) + "\t" + str(self.end) + "\t%d\n" % len(self.ranges)
        for r in self.ranges:
            s += str(r[0]) + "\t" + str(r[1]) + "\n"
        return s


class ContinuousCorrelationMap(object):
    def __init__(self, mapped_dim, target_dim):
        self.mapped_dim = mapped_dim
        self.target_dim = target_dim
        self.buckets = []

    # bstart and bend are the beginning (inclusive) and end (exclusive) of the mapped column.
    # Ranges is a list of [start, end) ranges as well.
    def add_mapping(self, bstart, bend, ranges):
        self.buckets.append(MappingBucket(bstart, bend, ranges))

    def to_string(self):
        s = "continuous\n%d\t%d\n" % (self.mapped_dim, self.target_dim)
        for b in self.buckets:
            s += b.to_string()
        return s


