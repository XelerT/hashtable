import matplotlib.pyplot as plt
import numpy as np

hash_files = ["one", "ascii", "length", "ascii_sum", "rol", "ror", "my_hash", "crc32"]

for n in range(len(hash_files)):
        file_string = "graphics/" + hash_files[n] + ".txt"
        file = open(file_string, "r")

        y = []
        x = []
        i = 1

        for line in file:
                for word in line.split():
                        y.append(int(word))
                        x.append(i)
                        i += 1

        plt.figure(figsize = (15, 10))
        plt.bar(x, y)
        plt.xticks(np.arange(0, i, step=1))

        plt.savefig('graphics/{}.png'.format(hash_files[n]))
        plt.clf()
