from sys import argv

if __name__ == "__main__":
    if len(argv) != 3:
        print("Usage: python drop_rotation.py <input file> <output file>")
        exit(1)

    lines = []
    with open(argv[1], "r") as f:
        lines = f.readlines()

    with open(argv[2], "w") as f:
        separator = " | "
        for line in lines:
            cells = line.split(separator)
            for i in range(0, len(cells)):
                if i != 5:
                    f.write(cells[i])
                else:
                    f.write("0.0")
                if i < len(cells) - 1:
                    f.write(separator)
