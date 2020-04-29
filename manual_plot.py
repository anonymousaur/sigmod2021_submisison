import matplotlib
matplotlib.use('Agg')
import matplotlib.pyplot as plt
import numpy as np

data_initial_time = 1.79e7 / 1e6
data_initial_overhead = 2969549

data_final_time = 4.35e6 / 1e6
data_final_overhead = 289434

fig, (ax1, ax2) = plt.subplots(1,2)
ax1.bar([1,2], [data_initial_time, data_final_time], width=0.5)
ax2.bar([1,2], [data_initial_overhead, data_final_overhead], width=0.5)
ax1.set_ylabel('Avg query time (ms)')
ax2.set_ylabel('Avg points scanned')
ax2.yaxis.tick_right()
ax2.yaxis.set_label_position("right")
ax1.set_xticks([1,2])
ax1.set_xticklabels(["CM", "Outliers"])
ax2.set_xticks([1,2])
ax2.set_xticklabels(["CM", "Outliers"])

plt.tight_layout()
plt.savefig('dmv_performance_results.png')
