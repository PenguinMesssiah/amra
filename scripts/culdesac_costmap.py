import sys
import numpy as np
import matplotlib.pyplot as plt
import scipy.ndimage as ndimage

def rgb2gray(rgb):

    r, g, b = rgb[:,:,0], rgb[:,:,1], rgb[:,:,2]
    gray = 0.2989 * r + 0.5870 * g + 0.1140 * b

    return gray

seeds = [4, 64, 144, 441, 3375, 4096, 4261] 
np.random.seed(seed=np.random.choice(seeds))

MAP = sys.argv[1]
mapname = MAP.split('/')[-1].split('.')[0]

# M = np.genfromtxt(MAP, delimiter=',')
M = plt.imread(MAP)
M = np.ceil(rgb2gray(M))
m = M.shape[0]
n = M.shape[1]
noise = np.random.randn(m, n) * 15
noise = ndimage.gaussian_filter(noise, sigma=(150, 150), order=1, mode='wrap')
noise = np.abs(noise)
costs = 97 + (122-97) * ((noise - np.min(noise)) / (np.max(noise) - np.min(noise)))
costs = costs * M
costs = costs.astype(np.int)

with open('../dat/' + mapname + '_costs.map', 'w') as F:
	F.write("type octile\n")
	F.write("height " + str(m) + "\n")
	F.write("width " + str(n) + "\n")
	F.write("map\n")

	for d1 in range(m):
		for d2 in range(n):
			if costs[d1, d2] == 0:
				F.write("T")
			else:
				F.write(chr(costs[d1, d2]))
		F.write("\n")

im = plt.imshow(costs.transpose(), cmap=plt.get_cmap('rainbow'), vmin=97, vmax=122)
im.cmap.set_under('k')
plt.colorbar()
# plt.savefig('../dat/imgs/bmps/' + mapname + '.png', bbox_inches='tight')
plt.show()