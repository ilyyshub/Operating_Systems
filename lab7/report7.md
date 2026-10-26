```markdown
# Lab 7 Observations

## Part 1 — fork, exec and wait

### 1. Why is perror("exec") printed only with nosuchprog?

When execlp succeeds, it replaces the child’s program with the requested program, so the code after execlp is not executed. With nosuchprog, execlp fails and returns, so perror prints "exec: No such file or directory".

### 2. Which number does the child print as "my parent", and whose PID is it?

The child prints the PID of its parent process, obtained through getppid(). In the ls test, this was 13470, matching the parent’s printed PID. In the date and nosuchprog tests, the parent PIDs were 13518 and 13543 respectively.

### 3. What status does the parent see with ls and with nosuchprog?

The parent receives status 0 with ls, indicating successful execution. With nosuchprog, it receives status 127 because the child explicitly calls exit(127) after execlp fails. The date test also returned status 0.

## Part 2 — A zombie process

### 1. Which letter does ls have in the STAT column, and what does it mean?

The ls process has the letter Z, meaning zombie. It has finished executing, but its parent has not yet collected its exit status. The captured output showed PID 13686 for the zombie and PPID 13683 for its parent.

### 2. After 20 seconds, is the defunct line still there? Which line made it disappear?

The later process listing no longer contained the [ls] <defunct> entry. After sleep(20) finished, waitpid(pid, &status, 0); collected the child’s exit status, allowing the kernel to remove its process-table entry.

## Part 3 — Output redirection

### 1. Which number did open give, and why exactly that one?

The output was "open gave descriptor 1". The program first called close(1), freeing the standard-output descriptor. Since open returns the lowest available descriptor, it returned 1.

### 2. Why does the listing not appear on the screen even though ls was not changed?

The ls program writes to standard output, descriptor 1. In the child, this descriptor now refers to out.txt and remains open across execlp, so the listing is written into the file. The head -3 out.txt command confirmed that the file contained the listing.

### 3. Which descriptor would you close to redirect errors instead?

I would close descriptor 2, which represents standard error, before opening err.txt.

## Part 4 — A pipe

### 1. Did ./pipeline and ls /etc | wc -l give the same number?

Yes. Both ./pipeline and the shell command ls /etc | wc -l printed 237.

### 2. After the change, which child never ends, ls or wc?

The wc child never ends. The ls child finishes, but the parent still holds the pipe’s writing end open because its close(p[1]); line was commented out. The program remained running until interrupted with Ctrl+C.

### 3. What is wc waiting for?

The wc process is waiting for end-of-file, which it cannot receive until all descriptors referring to the pipe’s writing end are closed.

## Conclusion

These experiments demonstrated process creation with fork, program replacement with exec, and child-status collection with waitpid. They also showed how a zombie remains until its parent waits for it, how file descriptors redirect output, and why closing unused pipe ends is necessary for a pipeline to finish.
```