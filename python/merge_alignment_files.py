import os
from sys import argv

if __name__ == "__main__":
    if len(argv) < 2:
        print("Usage:  python merge_alignment_files.py <folder_with_alignment_files>")
        exit(1)

    folder_with_alignment_files = argv[1]
    alignment_extension = "_alignment.txt"

    alignment_files = []
    for file_name in os.listdir(folder_with_alignment_files):
        if file_name.lower().endswith(alignment_extension):
            alignment_file_path = os.path.join(folder_with_alignment_files, file_name)
            alignment_files.append(os.path.abspath(alignment_file_path))

    alignment_files.sort()

    merged_alignment_file_path = os.path.join(folder_with_alignment_files, "merged_alignment.txt")
    with open(merged_alignment_file_path, 'w') as merged_file:
        for alignment_file in alignment_files:
            with open(alignment_file, 'r') as af:
                for line in af:
                    if line.startswith("#") or line.startswith("!reference_file!"):
                        continue
                    merged_file.write(line)
