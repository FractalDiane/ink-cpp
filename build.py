import subprocess

subprocess.run(["ninja"], cwd="build/debug")
subprocess.run(["ninja"], cwd="build/release")
