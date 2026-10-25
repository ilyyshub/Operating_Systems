# Lab 2 — Meet the OS You Will Build

Student: Ciocanu Ilinca  
Group: FAF-241

## Part 2 observations

1. xv6 includes `ls`, `cat` and `echo`. `ls` lists directory entries, `cat` prints file contents, and `echo` prints its arguments.
2. A pipe requires processes and interprocess communication. The shell starts the producer and consumer as separate processes and connects them through the kernel's pipe mechanism.
3. The xv6 shell supports basic execution, redirection and pipes, but has fewer expansion and interactive features than Bash.

## Part 3 observations

1. `user/cat.c` uses `open` to obtain a descriptor, `read` to receive bytes, `write` to print them, `close` to release a descriptor, and `exit` to terminate. `fprintf` is a userspace formatting function that ultimately uses `write`.
2. `sys_read` is implemented in `kernel/sysfile.c`. The report's reference lists it at line 66; verify the local line with `grep -n 'sys_read' kernel/sysfile.c`, since line numbers depend on the xv6 revision.
3. Files in `kernel/` implement privileged operating-system services; files in `user/` implement applications that request these services through system calls.

## sleep program

`user/sleep.c` accepts one tick count, converts it using `atoi`, calls `pause`, and exits. Missing arguments produce a usage message and exit status 1. The executable remains named `sleep`; the current system-call interface is `pause`.


## Reference

https://github.com/mit-pdos/xv6-riscv
