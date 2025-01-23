Import("env")
import os, zipfile, shutil
import fileinput
from pathlib import Path

# Get the version number from the build environment.
firmware_version = os.environ.get('VERSION', "")
if firmware_version == "":
    firmware_version = "0.0.1"
firmware_version = firmware_version.lstrip("v")
firmware_version = firmware_version.strip(".")

zip_file_name = 'Mobiflight-Connector'
build_path = './_build'
build_path_fw = build_path + '/firmware'
build_path_json = build_path + '/Boards'
distrubution_path = './_dist'
board_folder = ['./_Boards/Atmel', './_Boards/RaspberryPi']

def copy_fw_files (source, target, env):
    fw_file_name=str(target[0])

    if os.path.exists(build_path_fw) == False:
        os.makedirs(build_path_fw)
    if os.path.exists(build_path_json) == False:
        os.makedirs(build_path_json)

    if fw_file_name[-3:] == "bin":
        fw_file_name=fw_file_name[0:-3] + "uf2"

    # Copy build FW file
    shutil.copy(fw_file_name, build_path_fw)

    # Copy reset/uf2/json files
    file_extension = '.hex'
    copy_files_by_extension(board_folder, build_path_fw, file_extension)
    file_extension = '.uf2'
    copy_files_by_extension(board_folder, build_path_fw, file_extension)
    file_extension = '.json'
    copy_files_by_extension(board_folder, build_path_json, file_extension)

    # set FW version within boad.json files
    replacements = {
        "0.0.1": firmware_version
    }
    for file_path in build_path_json.rglob("*.json"):
        replace_in_file(file_path, replacements)

    # Create ZIP file and add files from distrubution folder
    zip_file_path = distrubution_path + '/' + zip_file_name + '_' + firmware_version + '.zip'
    createZIP(build_path, zip_file_path, zip_file_name)


def copy_files_by_extension(source_folders, target_folder, file_extension):
    for source_folder in source_folders:
        for root, dirs, files in os.walk(source_folder):
            for file in files:
                if file.endswith(file_extension):
                    source_path = os.path.join(root, file)
                    target_path = os.path.join(target_folder, file)
                    shutil.copy2(source_path, target_path)


def createZIP(original_folder_path, zip_file_path, new_folder_name):
    if os.path.exists(distrubution_path) == False:
        os.mkdir(distrubution_path)
    with zipfile.ZipFile(zip_file_path, 'w') as zipf:
        for root, dirs, files in os.walk(original_folder_path):
            for file in files:
                # Create a new path in the ZIP file
                new_path = os.path.join(new_folder_name, os.path.relpath(os.path.join(root, file), original_folder_path))
                # Add the file to the ZIP file
                zipf.write(os.path.join(root, file), new_path)


def replace_in_file(file_path, replacements):
    """Replace all keys in `replacements` with their values in the given file."""
    with open(file_path, "r", encoding="utf-8") as file:
        content = file.read()

    for old, new in replacements.items():
        content = content.replace(old, new)

    with open(file_path, "w", encoding="utf-8") as file:
        file.write(content)


env.AddPostAction("$BUILD_DIR/${PROGNAME}.hex", copy_fw_files)
env.AddPostAction("$BUILD_DIR/${PROGNAME}.bin", copy_fw_files)
