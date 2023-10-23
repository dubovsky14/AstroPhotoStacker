import hashlib
from sys import argv

def crash(error_message : str):
    print(error_message)
    exit(1)

if (len(argv) != 3):
    crash("Exactly 2 input arguments are required!")

h1 = hashlib.sha256(open(argv[1],'rb').read())
h2 = hashlib.sha256(open(argv[2],'rb').read())

if (h1.hexdigest() != h2.hexdigest()):
    crash("The files are not identical!")
else:
    print("Files are identical.")