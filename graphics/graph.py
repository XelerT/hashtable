import matplotlib.pyplot as plt
import numpy as np

hash_files = ["one", "ascii", "length", "ascii_sum", "rol", "ror", "my", "crc32"]

file_string = "graphics/" + "test_all.txt"
file = open(file_string, "r")
# for n in range(len(hash_files)):



for line in file:
        y = []
        x = []
        i = 1
        is_name = True
        for word in line.split():
                if is_name:
                        name = word
                        is_name = False
                        continue
                y.append(int(word))
                x.append(i)
                i += 1

        plt.figure(figsize = (15, 10))
        plt.bar(x, y)
        plt.xticks(np.arange(0, i, step=1))

        plt.savefig('graphics/{}.png'.format(name))
        plt.clf()
