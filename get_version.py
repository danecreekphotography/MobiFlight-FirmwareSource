Import("env")
import os

# Get the version number from the build environment.
firmware_version = os.environ.get('VERSION', "")

# Clean up the version number
if firmware_version == "":
  # When no version is specified default to "0.0.1" for
  # compatibility with MobiFlight desktop app version checks.
  firmware_version = "0.0.1"

# Strip any leading "v" that might be on the version and
# any leading or trailing periods.
firmware_version = firmware_version.lstrip("v")
firmware_version = firmware_version.strip(".")

# The community devices have a slightly different version where the
# core firmware version is defined within there .ini file
# For the core FW both versions are always the same
core_firmware_version = firmware_version

print(f'Using version {firmware_version} for the build')
print(f'Using version {core_firmware_version} as core version')

# Append the version and core version to the build defines so it gets baked into the firmware
env.Append(CPPDEFINES=[
  f'BUILD_VERSION={firmware_version}', f'CORE_BUILD_VERSION={core_firmware_version}'
])

# Set the output filename to the name of the board and the version
env.Replace(PROGNAME=f'{env["PIOENV"]}_{firmware_version.replace(".", "_")}')

