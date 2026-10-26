report8.md
# Lab 8 Observations

## Part 1 — fork, exec and wait inside xv6

### 1. Which three calls are used in xv6 and Linux? What is the same?

In xv6, run.c uses fork(), exec(), and wait().

In Linux, run.c uses fork(), execlp(), and waitpid().

Both programs follow the same sequence: create a child process, replace the child's program with another program, and have the parent wait for the child to finish and collect its exit status.

The interfaces differ: Linux execlp searches PATH for the executable, while xv6 exec uses the supplied path. Linux waitpid selects a specific child, while xv6 wait waits for any child.

### 2. Who prints "hello from the child"?

The echo program prints "hello from the child". After exec succeeds, echo replaces the child's run program and receives those words as its arguments.

In my test, the parent PID was 3 and the child PID was 4. After echo printed the message, the parent reported that child 4 ended with status 0.

### 3. Why is the failure status 1 in xv6 and 127 on Linux?

These values come from the exit calls written in the programs. After exec fails, the xv6 child calls exit(1), while the Linux child calls exit(127).

In my xv6 test, nosuchprog caused "exec failed" to be printed, and the parent reported status 1. The corresponding Linux test reported status 127.

## Part 2 — Communication through pipes

### 1. Why are two pipes needed?

Each pipe carries data in one direction. The p2c pipe carries the ping byte from the parent to the child, and the c2p pipe carries the reply from the child to the parent.

Separate pipes ensure that each process reads the other process's message. With one shared pipe, a process could read the byte it wrote itself.

### 2. Which call makes the parent wait for the child's reply?

The call read(c2p[0], &b, 1) blocks while the reply pipe is empty and its writing end remains open. It returns after the child writes the reply byte.

The later wait(0) collects the child's termination; it is not the call that waits for the reply byte.

### Recorded output

    $ pingpong
    4: received ping
    3: received pong
    $

The child printed ping before the parent printed pong, and the shell prompt returned after the program finished.

## Conclusion

The xv6 experiments demonstrated the same process lifecycle used on Linux: fork creates a child, exec replaces its program, and wait collects its exit status. The pingpong program demonstrated communication through two pipes and synchronization through blocking read calls.