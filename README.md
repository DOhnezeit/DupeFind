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

- This is a local tool — no network access or uploading.
- Files are moved to the system Recycle Bin, so accidental deletes are reversible.
- Performance depends on file sizes and number of files (it hashes every file).

## Why I made this

I noticed that I often download files multiple times by accident, cluttering my folders with duplicates. I took this as an opportunity to practice building a complete CLI tool in C++ that does real filesystem work, including hashing, Unicode support, and safe file deletion. Might be useful for others too.

---

**Built with C++20 / Windows API**
