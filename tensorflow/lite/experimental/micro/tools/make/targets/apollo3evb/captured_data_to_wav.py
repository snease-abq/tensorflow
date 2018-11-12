import numpy as np
import re
import struct
import matplotlib.pyplot as plt

def new_data_to_array(fn):
    vals = []
    with open(fn) as f:
        for n, line in enumerate(f):
            if n is not 0:
                vals.extend([int(v, 16) for v in line.split()])
    b = ''.join(map(chr, vals))
    y = struct.unpack('<'+'h'*int(len(b)/2), b)
            
    return y

        
data = 'captured_data.txt'            
vals = new_data_to_array(data)

plt.plot(vals, 'o-')
plt.show()
