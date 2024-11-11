import subprocess

print("Building Debug:")
subprocess.run(["cmake", "--build", "."], cwd="build/debug")
print("Building Release:")
subprocess.run(["cmake", "--build", "."], cwd="build/release")
