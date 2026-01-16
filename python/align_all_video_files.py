import os
from sys import argv
import argparse

if __name__ == "__main__":
        parser = argparse.ArgumentParser()
        parser.add_argument("-reference_file", type=str, help="Reference frame (still image or video frame)")
        parser.add_argument("-video_files_folder", type=str, help="Folder with video files to align")
        parser.add_argument("-n_cpu", type=int, help="Number of CPU cores to use")
        parser.add_argument("-method"  , type=str, help="Alignment method, default is 'stars'", default="stars", nargs='?')

        args = parser.parse_args()
        reference_file        = args.reference_file
        video_files_folder    = args.video_files_folder
        n_cpu                 = args.n_cpu
        method                = args.method

        executable_path = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "bin", "produce_alignment_file")

        video_extensions = [".mp4", ".avi", ".ser"]

        video_files = []
        for file_name in os.listdir(video_files_folder):
            if any(file_name.lower().endswith(ext) for ext in video_extensions):
                video_file_path = os.path.join(video_files_folder, file_name)
                video_files.append(os.path.abspath(video_file_path))

        video_files.sort()

        for video_file in video_files:
            video_file_folder = os.path.dirname(video_file)
            video_file_name   = os.path.basename(video_file)
            output_alignment_file = os.path.join(video_file_folder, f"{os.path.splitext(video_file_name)[0]}_alignment.txt")

            command_template = f'"{executable_path}" -reference_file "{reference_file}" -file_to_align "{video_file}" -n_cpu {n_cpu} -method {method} -alignment_file "{output_alignment_file}"'
            os.system(command_template)
