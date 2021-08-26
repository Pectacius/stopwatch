# Searches the <path_name> directory and renames all files with the name <search_file_name>
# with <rename_file_name>. Useful for working with Stopwatch results when each result for
# each processor is in a seperate file.


import os
import sys


def find_files(folder, file_name):
    return [os.path.join(r, fn) for r, ds, fs in os.walk(folder) for fn in fs if fn==file_name]

if len(sys.argv) != 4:
    print(f"Usage: python3 {sys.argv[0]} <path_name> <search_file_name> <rename_file_name>")
else:
    search_path = sys.argv[1]
    search_file_name = sys.argv[2]
    rename_file_name = sys.argv[3]
    all_files = find_files(search_path, search_file_name)

    for file_name in all_files:
        os.rename(file_name, file_name.replace(search_file_name, rename_file_name))

