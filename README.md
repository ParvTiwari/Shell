# Custom Unix Shell in C

A lightweight Unix-like command-line shell implemented in C. This project demonstrates core Operating System concepts such as process creation, inter-process communication, signal handling, and file descriptor management.

---

## 🚀 Features

### 🔹 Command Execution
- Executes standard Linux commands using execvp()
- Supports multiple arguments

### 🔹 Command History
- Stores up to 100 previously executed commands  
- View history using:
```bash
history
```

### 🔹 Signal Handling
- Handles Ctrl + C (SIGINT) without terminating the shell  
- Ignores Ctrl + \ (SIGQUIT)

### 🔹 Background Execution
Run commands in the background using &:
```bash
sleep 10 &
```

### 🔹 Input / Output Redirection
Output redirection:
```bash
ls > output.txt
```

Input redirection:
```bash
sort < input.txt
```

Error redirection:
```bash
ls invalid_dir 2> error.txt
```

### 🔹 Piping
Supports multiple pipes:
```bash
ls | grep txt | wc -l
```

### 🔹 Environment Variables
Supports environment variable expansion:
```bash
echo $HOME
```

---

## 🧠 Concepts Used

- fork() – process creation  
- execvp() – command execution  
- pipe() – inter-process communication  
- dup2() – file descriptor redirection  
- signal() – signal handling  
- open() – file operations  
- strtok() – command parsing  

---

## ⚙️ Installation & Usage

### Compile
```bash
gcc myshell.c -o myshell
```

### Run
```bash
./myshell
```

---

## 📌 Example Commands

```bash
>>> ls -l
>>> echo Hello World
>>> cat file.txt | grep test
>>> sleep 5 &
>>> history
>>> exit
```
