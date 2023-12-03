import argparse
from colorama import init, Fore

# Initialize colorama
init(autoreset=True)

def compare_files(file1_path, file2_path):
    try:
        # Open and read the first file
        with open(file1_path, "r") as f1:
            f1_data = f1.readlines()

        # Open and read the second file
        with open(file2_path, "r") as f2:
            f2_data = f2.readlines()

        for i, (line1, line2) in enumerate(zip(f1_data, f2_data), start=1):
            if line1 == line2:
                print(Fore.GREEN + f"Line {i}: IDENTICAL")
            else:
                print(Fore.RED + f"Line {i}:")
                print(Fore.CYAN + f"\tFile 1: {line1}", end='')
                print(Fore.LIGHTYELLOW_EX + f"\tFile 2: {line2}", end='')

    except FileNotFoundError:
        print(Fore.RED + "File not found. Please check the file paths.")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Compare two text files.")
    parser.add_argument("file1", help="Path to the first text file")
    parser.add_argument("file2", help="Path to the second text file")
    args = parser.parse_args()

    compare_files(args.file1, args.file2)
