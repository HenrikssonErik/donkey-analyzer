from memory_profiler import profile 
import os, subprocess

@profile
def run_program():
    result = subprocess.run("make attack_h1m_atlas", shell=True, check=True)
    print("Done")

if __name__ == "__main__":
    run_program()