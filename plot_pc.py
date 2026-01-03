import re
import matplotlib.pyplot as plt

pcs = []
instr = []

with open("../build/out.txt", "r", encoding="utf-8") as f:
    for line in f:
        m = re.match(r"\[(\d+)\]\s+PC:\s+\$([0-9A-Fa-f]+)", line)
        if m:
            instr.append(int(m.group(1)))
            pcs.append(int(m.group(2), 16))

plt.plot(instr, pcs, marker="o")
plt.xlabel("Instruction #")
plt.ylabel("PC (hex)")
plt.title("Evolution du PC")
plt.grid(True)
plt.show()