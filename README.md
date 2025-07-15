# DupeFind

**DupeFind** is a simple command-line tool written in C++ that scans a folder for duplicate files based on their content (using hashing). It's a personal project mainly built for learning purposes, but it's fully functional and can help clean up duplicate files from your disk.

## Features

- Recursively scans folders for files
- Uses file hashes to detect duplicates
- Interactive or automatic duplicate removal
- Deleted files go to the Recycle Bin (safer than direct deletion)
- Unicode path support
- Outputs logs to `scan_results.txt` and `duplicate_log.txt`

## How it works

1. You enter the folder path.
2. DupeFind scans all files inside (recursively).
3. It calculates a hash for each file and compares them.
4. If duplicates are found, you can choose to:
   - Keep everything
   - Remove duplicates interactively
   - Automatically remove all but the version with the shortest path

# Notes

- This is a local tool, no network access or uploading.
- Files are moved to the system Recycle Bin, so accidental deletes are reversible.
- Performance depends on file sizes and number of files (it hashes every eligible file).
- Skips System files as well as files with some extensions (see shouldSkipFile function in FileScanner.cpp)

# Potential future improvements

I might add multi-threading and turn this from a CLI to an application with a GUI. I also want to look into using partial hashing instead of hashing entire files.
A possible implementation could be a first pass which just hashes a small part of the file, which is used to quickly filter out differing files. If there are identical groups found, a second pass could then be more precise.
On a more basic level, currently every file is hashed. This is totally unneccessary since files could be disregarded if their file size is unique. A "file size filter" is missing.

## Why I made this

I noticed that I often download files multiple times by accident, cluttering my folders with duplicates. I took this as an opportunity to practice building a complete CLI tool in C++ that does real filesystem work, including hashing, Unicode support, and safe file deletion. Might be useful for others too.

---

**Built with C++20 / Windows API**
