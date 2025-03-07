from sys import argv
import os

def adjust_values(settings_dict : dict) -> None:
    resolution = settings_dict["ROI"].split("x")
    settings_dict["Width"] = resolution[0]
    settings_dict["Height"] = resolution[1]

    exposure_time = float(settings_dict["Shutter"].split("ms")[0])
    settings_dict["Exposure"] = str(round(exposure_time / 1000, 4)) + " Sec"
    settings_dict["Gain"] = settings_dict["Gain"].split()[0]

def convert_firecapture_txt_to_zwo_txt(firecapture_txt_file : str, zwo_txt_file : str) -> None:
    settings = {}
    with open(firecapture_txt_file, 'r') as input_file:
        for line in input_file:
            if "=" not in line:
                continue
            elements = line.split("=")
            settings[elements[0].strip()] = elements[1].strip()

    adjust_values(settings)

    with open(zwo_txt_file, 'w') as output_file:
        output_file.write(f'[{settings["Camera"]}]\n')
        output_file.write(f'Bin = {settings["Binning"][0]}\n')
        output_file.write(f'Capture Area Size = {settings["Width"]} * {settings["Height"]}\n')
        output_file.write("Colour Format = RAW8\n")
        output_file.write(f'Exposure = {settings["Exposure"]}\n')
        output_file.write(f'Gain = {settings["Gain"]}\n')
        output_file.write("Bayer = RG\n")
        output_file.write("Debayer Type = RGGB\n")
        output_file.write(f'White Balance (B) = {settings["WBlue"]}\n')
        output_file.write(f'White Balance (R) = {settings["WRed"]}\n')

def get_list_of_input_files_and_output_files(input_folder : str) -> list[tuple[str, str]]:
    result = []
    for file in os.listdir(input_folder):
        if file.endswith(".txt") and not file.endswith("avi.txt"):
            input_file = os.path.join(input_folder, file)
            output_file = input_file[:-4] + ".avi.txt"
            result.append((input_file, output_file))
    return result


if __name__ == "__main__":
    if len(argv) == 3:
        input_file_address = argv[1]
        output_file_address = argv[2]
        if not os.path.isfile(input_file_address):
            files = get_list_of_input_files_and_output_files(input_file_address)
            print("Usage: python convert_firecapture_txt_to_zwo_txt.py firecapture.txt zwo.txt")
            exit(1)
        convert_firecapture_txt_to_zwo_txt(input_file_address, output_file_address)
    elif len(argv) == 2:
        if not os.path.isdir(argv[1]):
            print("Usage: python convert_firecapture_txt_to_zwo_txt.py <directory with firecapture txt files>")
            exit(1)
        files = get_list_of_input_files_and_output_files(argv[1])
        for input_file, output_file in files:
            convert_firecapture_txt_to_zwo_txt(input_file, output_file)