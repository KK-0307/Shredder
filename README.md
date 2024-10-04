# Shredder

**Shredder** is a C-based project designed to efficiently shred or delete files from a system, ensuring that sensitive data is securely erased and cannot be recovered. The program overwrites files with random data multiple times before deleting them, following best practices for data destruction. This tool is particularly useful for individuals or institutions requiring secure file deletion, such as universities or organizations handling confidential information.

## Features

- **Secure File Deletion**: Overwrites files multiple times with random data before deletion to prevent data recovery.
- **Configurable Overwrite Passes**: Allows users to specify how many times the file should be overwritten before deletion.
- **Error Handling**: Robust error handling for non-existent files, file permissions, and other issues that may arise during the shredding process.
- **Fast and Efficient**: Written in C for high performance, enabling fast file shredding even for larger files.

