# rush - A Simplified Unix Shell

## Description
`rush` is a basic Unix shell implemented in C. It supports both interactive and batch modes, allowing users to execute commands from the terminal or from a file. The shell handles built-in commands (`exit`, `cd`, `path`), supports output redirection using `>`, and allows parallel command execution using `&`. External programs are launched using `execv()`, and executable paths are validated using `access()`.

## Features
- Interactive mode with prompt (`rush>`)
- Batch mode via input file
- Built-in commands: `exit`, `cd`, `path`
- Output redirection (`>`)
- Parallel command execution (`&`)
- Customizable search paths for executables

## Compilation
```bash
gcc -o rush rush.c
```
## Usage
- Interactive mode
```bash
./rush
```
- Batch mode
```bash
./rush input.txt
```
## Notes
- Only one redirection (>) is supported per command
- Parallel commands must be separated by &
- Invalid inputs produce a standard error message: An error has occurred
## Author
- Name: Nghi Hoang
- Course: Operating System
- Date: Sep 17th 2025
## Example Commands

### Built-in commands
```bash
path /bin /usr/bin 
cd /tmp
exit
```
### External commands
```bash
ls
echo Hello, world!
```
### Output redirection
```bash
echo Hello > hello.txt
ls > listing.txt
```
### Parallel commands
```bash
echo cat & echo dog
ls & echo done
```
