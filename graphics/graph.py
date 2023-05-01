import matplotlib.pyplot as plt
import numpy as np

hash_files = ["one", "ascii", "length", "ascii_sum", "rol", "ror", "my", "crc32"]

file_string = "graphics/" + "test_all.txt"
file = open(file_string, "r")
# for n in range(len(hash_files)):

n = 1
n_row = 1
n_column = 1

fig, axs = plt.subplots(nrows=3, ncols=2, figsize=(15, 12))
plt.subplots_adjust(hspace=0.5)

for line, ax in zip(file, axs.ravel()):
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
        if name == 'ror':
                continue
        if name == 'my':
                continue

        ax.bar(x, y, width=1)

        ax.set_title(name)
        ax.set_xlabel("list number")
        ax.set_ylabel("n_elements in list")

        n += 1

        if n_column == 2:
                n_column = 1
                n_row += 1
        else:
                n_column += 1

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
        if name != 'crc32':
                continue

        ax.bar(x, y, width=1)

        ax.set_title(name)
        ax.set_xlabel("list number")
        ax.set_ylabel("n_elements in list")

        n += 1

        if n_column == 2:
                n_column = 1
                n_row += 1
        else:
                n_column += 1

plt.savefig('graphics/graphs.png')
