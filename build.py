import subprocess

print("Building Debug:")
subprocess.run(["ninja"], cwd="build/debug")
print("Building Release:")
subprocess.run(["ninja"], cwd="build/release")
