#!/usr/bin/env python3
"""
Test runner for minieigen tests.
Run all pytest tests or specific test files.
"""

import sys
import os
import subprocess

# Get the directory containing this script
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))

# Add the build directory to PYTHONPATH for minieigen module
BUILD_LIB_DIR = os.path.join(SCRIPT_DIR, "build", "src", "lib")
if os.path.exists(BUILD_LIB_DIR):
    sys.path.insert(0, BUILD_LIB_DIR)

def run_pytest(args=None):
    """Run pytest with the given arguments."""
    # Build pytest command
    cmd = [sys.executable, "-m", "pytest"]
    
    if args:
        cmd.extend(args)
    else:
        # Default: run all tests in the tests directory
        cmd.append("tests")
    
    # Add verbose output by default
    if "-v" not in cmd and "--verbose" not in cmd:
        cmd.append("-v")
    
    # Change to the script directory
    os.chdir(SCRIPT_DIR)
    
    # Set PYTHONPATH environment variable
    env = os.environ.copy()
    env["PYTHONPATH"] = BUILD_LIB_DIR + os.pathsep + env.get("PYTHONPATH", "")
    
    print(f"Running: {' '.join(cmd)}")
    print(f"PYTHONPATH: {env['PYTHONPATH']}")
    print("=" * 70)
    
    # Run pytest
    result = subprocess.run(cmd, cwd=SCRIPT_DIR, env=env)
    return result.returncode


def main():
    """Main entry point."""
    # Get command line arguments
    args = sys.argv[1:]
    
    # Run pytest
    returncode = run_pytest(args)
    
    # Exit with pytest's return code
    sys.exit(returncode)


if __name__ == "__main__":
    main()