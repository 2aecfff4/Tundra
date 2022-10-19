import fileinput
import os
import subprocess
import sys


dir_path = os.path.dirname(os.path.realpath(__file__))
dir_path = dir_path.replace('\\', '/')
asset_dir = os.path.join(
    dir_path, os.pardir, "assets/shaders").replace('\\', '/')


for root, dirs, files in os.walk(os.path.join(dir_path, "src")):
    for file in files:
        if not file.endswith(".hlsl"):
            continue

        hlsl_file = os.path.join(root, file)

        v = os.path.relpath(
            hlsl_file, os.path.join(
                dir_path, "src"))
        output_file = os.path.join(asset_dir, v).replace('\\', '/') + ".spv"
        os.makedirs(os.path.dirname(output_file), exist_ok=True)

        target_env = "-fspv-target-env=vulkan1.3"
        profile = ""
        extra_flags = ""

        if (hlsl_file.find('.vert') != -1):
            profile = 'vs_6_6'
            extra_flags += "-fvk-invert-y"
        elif (hlsl_file.find('.frag') != -1):
            profile = 'ps_6_6'
        elif (hlsl_file.find('.comp') != -1):
            profile = 'cs_6_6'
        else:
            continue

        print("")
        print(f"====== Compiling `{hlsl_file}` ... ======")
        subprocess.call([
            "dxc",
            "-HV 2021",
            "-O3",
            "-fspv-debug=line",
            "-DLM_VULKAN",
            "-spirv",
            # "/Zpc",
            "-I", os.path.join(dir_path, "src"),
            target_env,
            "-Zi",
            extra_flags,
            "-T", profile,
            "-E", "main",
            hlsl_file,
            "-Fo", output_file])
