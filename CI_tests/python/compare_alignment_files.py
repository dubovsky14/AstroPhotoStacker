import hashlib
from sys import argv
from os import system

def compare_lines(line1 : str, line2 : str, relative_numerical_tolerance : float = 0.01) -> bool:
    # Lines can differ only in numbers. The function should return True if the lines are considered equal within the given relative numerical tolerance, False otherwise.
    import re
    numbers1 = [float(x) for x in re.findall(r"[-+]?\d*\.\d+|\d+", line1)]
    numbers2 = [float(x) for x in re.findall(r"[-+]?\d*\.\d+|\d+", line2)]
    if len(numbers1) != len(numbers2):
        return False
    for n1, n2 in zip(numbers1, numbers2):
        if abs(n1 - n2) > relative_numerical_tolerance * max(abs(n1), abs(n2), 1.0):
            return False

    # Now let's compare the non-numeric parts of the lines
    import re
    non_numbers1 = re.split(r"[-+]?\d*\.\d+|\d+", line1)
    non_numbers2 = re.split(r"[-+]?\d*\.\d+|\d+", line2)
    if non_numbers1 != non_numbers2:
        return False

    return True

def crash(error_message : str):
    print(error_message)
    exit(1)

if (len(argv) != 3):
    crash("Exactly 2 input arguments are required!")

h1 = hashlib.sha256(open(argv[1],'rb').read())
h2 = hashlib.sha256(open(argv[2],'rb').read())

if (h1.hexdigest() != h2.hexdigest()):
    # If the files are not identical, let's compare them line by line using the compare_lines function
    with open(argv[1], 'r') as f1, open(argv[2], 'r') as f2:
        for line1, line2 in zip(f1, f2):
            if not compare_lines(line1, line2):
                crash("The files differ at line:\n" + line1 + "\n" + line2)
    print("The files are not identical within numerical tolerance")
else:
    print("Files are identical.")