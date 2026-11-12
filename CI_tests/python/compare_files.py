import hashlib
from sys import argv
from os import system

def crash(error_message : str):
    print(error_message)
    exit(1)

if (len(argv) != 3):
    crash("Exactly 2 input arguments are required!")

h1 = hashlib.sha256(open(argv[1],'rb').read())
h2 = hashlib.sha256(open(argv[2],'rb').read())

if (h1.hexdigest() != h2.hexdigest()):
    print("The files are not identical")
    system("diff " + argv[1] + " " + argv[2])
    exit(1)
else:
    print("Files are identical.")