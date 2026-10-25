# Operating Systems — Labs 5 and 6

**Student:** Ciocanu Ilinca  
**Group:** FAF-241  
**Academic year:** 2026–2027  
**Repository:** https://github.com/ilyyshub/Operating_Systems

## Working environment

Linux experiments use Ubuntu in UTM with the normal user `ilinca77`, inside `~/os-lab5`. xv6 RISC-V runs in QEMU from `~/xv6-riscv`. Commands marked Ubuntu are executed outside the xv6 shell.

# Lab 5 — The File System with Your Own Hands

## Part 1 — The tree and file types

### 1. Types of the listed paths

Command:

```sh
ls -ld /etc /etc/passwd /bin /dev/null /tmp
```

| Path | First character | Type |
|---|---|---|
| `/etc` | `d` | Directory |
| `/etc/passwd` | `-` | Regular file |
| `/bin` | `l` | Symbolic link |
| `/dev/null` | `c` | Character device |
| `/tmp` | `d` | Directory |

`whoami` returned `ilinca77`, confirming the experiments used a normal account. The corrected `/tmp` listing showed `drwxrwxrwt`.

### 2. What is /bin and where does it point?

Both `ls -ld /bin` and `file /bin` identified `/bin` as a symbolic link to `usr/bin`. Since the link is located in `/`, its target resolves to `/usr/bin`.

`file /etc/passwd` identified ASCII text. On this installation, `/usr/bin/ls` is also a symbolic link, pointing to `../lib/cargo/bin/coreutils/ls`.

### 3. Character and block devices

`/dev/null` is a character device; its listing begins with `c`. `/dev/vda` is a block device; its listing begins with `b`. The block-device listing also includes the partitions `/dev/vda1` and `/dev/vda2`.

Commands:

```sh
ls -l /dev | head
ls -l /dev | grep '^b'
```

### 4. Comparing directory sizes

```text
$ du -sh /var/log 2>/dev/null
21M     /var/log
$ du -sh /usr 2>/dev/null
5.3G    /usr
```

`/usr` uses approximately 260 times as much allocated space as `/var/log`, roughly 5.28 GiB more. The estimates use rounded human-readable values. Hidden permission errors can make totals incomplete for directories the normal user cannot read.

## Part 2 — Inodes and links

### 1. Inode number and number of names

In `stat notes.txt`, the `Inode` field identifies the inode within the file system. The `Links` field gives the number of hard links. `ls -li notes.txt` displays the inode number in its first column.

### 2. Why hard.txt survives removal of notes.txt

```sh
echo hello > notes.txt
ln notes.txt hard.txt
ls -li notes.txt hard.txt
rm notes.txt
cat hard.txt
```

A hard link creates another directory entry referring to the same inode. Both names therefore access the same file data. `rm notes.txt` removes that name and decreases the link count; it does not remove the other name. `hard.txt` remains readable. Storage can be reclaimed once no hard links and no open references remain.

### 3. Why soft.txt becomes unreadable and why its size is 8 bytes

```sh
ln -s hard.txt soft.txt
rm hard.txt
cat soft.txt
ls -l soft.txt
```

A symbolic link has its own inode and stores a target path. After `hard.txt` is removed, the target cannot be resolved, although `soft.txt` itself still exists. Its logical size is 8 bytes because `hard.txt` contains eight characters; a terminating null byte is not counted. A symbolic link does not preserve the target inode as a hard link does.

### 4. Directory link counts of 2 and 4

For the Linux directory-link behavior in this exercise, a new directory `d` has two links: the entry named `d` in its parent and the `.` entry inside `d`. Creating `d/x` and `d/y` adds two more references to `d`, through the `..` entry in each child. The link count therefore becomes four. The `..` entry inside `d` refers to its parent and does not add a link to `d` itself.

### 5. Logical size versus allocated space

`echo x > one.txt` writes two bytes: `x` and a newline. On a file system allocating 4 KiB blocks, this small file can occupy one full block, so `ls -l` reports 2 bytes while `du -h` reports 4.0K.

`truncate -s 1G big.img` creates a sparse file with a logical size of 1 GiB. Unwritten regions are holes that read as zeroes and need no data blocks. Thus its allocated data space can be zero even though its logical size is large. `ls` reports logical size; `du` reports allocated storage. Exact allocation behavior depends on the file system.

## Part 3 — Permissions

### 1. Numeric modes and practical uses

| Mode | Permission string | Example use |
|---|---|---|
| `600` | `rw-------` | Private file readable and writable by its owner |
| `644` | `rw-r--r--` | Document others may read but only the owner may edit |
| `755` | `rwxr-xr-x` | Shared directory or executable |
| `700` | `rwx------` | Private directory accessible only to its owner |

### 2. Why ownership does not bypass chmod 000

The corrected command `chmod 000 s.txt; cat s.txt` produced `Permission denied`. Ownership allows the user to change the file's permissions, but does not bypass the permission check when reading it. After `chmod u+r s.txt`, `cat s.txt` printed `secret`.

### 3. Measured umask and creation modes

```text
umask:   0002
new.txt: -rw-rw-r--
newdir:  drwxrwxr-x
```

The measured umask was `0002`. It removes write permission for other users:

- Regular file: `0666 & ~0002 = 0664` (`rw-rw-r--`).
- Directory: `0777 & ~0002 = 0775` (`rwxrwxr-x`).

The `0` displayed for `new.txt` and `4096` for `newdir` are sizes in bytes, not numeric permission modes.

### 4. Read and execute permissions on directories

With `chmod u=r,go= box`, `ls box` printed `f`, but `cat box/f` produced `Permission denied`. Read permission allows entry names to be listed, while the missing execute permission prevents searching the directory to access the file.

With `chmod u=x,go= box`, `ls box` produced `Permission denied`, but `cat box/f` printed `hi`. Execute permission allows access through a known name, provided the file's own permissions allow reading. It does not allow enumeration of the directory's names. `chmod 755 box` restored ordinary access afterward.

### 5. The s on passwd and the t on /tmp

The listing of `/usr/bin/passwd` showed `-rwsr-xr-x`. The `s` in the owner's execute position denotes set-user-ID. Because the executable belongs to root, it can run with root's effective identity to update protected password information after authorization checks.

The listing of `/tmp` showed `drwxrwxrwt`. The final `t` denotes the sticky bit. Although all users can create files there, directory write access alone does not allow deletion of another user's file; deletion is restricted to the file owner, directory owner or a privileged process.

Reading `/etc/shadow` produced `Permission denied`. Its listing showed `-rw-r-----`, owned by `root` with group `shadow`, restricting access to password-related data.

## Part 4 — Disks and the kernel as files

### 1. Disk layout

Commands: `lsblk`, `lsblk -f`, and `mount | grep " / "`.

```text
/dev/vda — virtual disk, 45G
  /dev/vda1 — partition, mounted at /boot/efi, vfat (FAT32)
  /dev/vda2 — partition, mounted at /, ext4
Root device: /dev/vda2
```

`lsblk` displayed partition sizes of `1G` for `vda1` and `43.9G` for `vda2`. The mount output confirmed:

```text
/dev/vda2 on / type ext4 (rw,relatime)
```

### 2. Root file-system usage

The captured `df -h` output reported:

| Device | Total | Used | Available | Use | Mount |
|---|---|---|---|---|---|
| `/dev/vda2` | `43G` | `12G` | `30G` | `28%` | `/` |

These values belong to the same measurement. A later `lsblk -f` screenshot showed `29.4G` available and `26%` usage; those later values are not mixed into the earlier `df` row. The root file system is identified by mount point `/`. The `tmpfs` rows describe separate memory-backed file systems.

### 3. Logical processors and RAM

`grep -c processor /proc/cpuinfo` returned `4`. `/proc/meminfo` reported `MemTotal: 3461740 kB`.

```text
3,461,740 / 1,048,576 ≈ 3.30 GiB
```

Ubuntu therefore sees four logical processors and approximately 3.30 GiB of usable RAM. These are resources exposed to the virtual machine, not necessarily the Mac's physical core count or total RAM.

### 4. Program name in /proc/self/status

`head -5 /proc/self/status` showed `Name: head`. `/proc/self` refers to the process reading the path, so the information describes `head`, rather than the shell that launched it.

The file-descriptor listing showed descriptors `0`, `1` and `2` pointing to `/dev/pts/0`, the terminal used for standard input, output and error. Descriptor `3` pointed to `/proc/4700/fd`, the directory opened by the listing process.

## Lab 5 conclusion

Linux separates directory names from inode identity. Hard links share an inode, symbolic links store paths, and permissions control access to files and directories. Logical size can differ from allocated storage, while `/proc` provides a file-based interface to current kernel information.

# Lab 6 — Inside the xv6 File System

## Part 1 — Read what xv6 shows

### 1. Meanings of file types

In `kernel/stat.h`, `T_DIR = 1` identifies a directory, `T_FILE = 2` a regular file, and `T_DEVICE = 3` a device. xv6 `ls` displays name, type, inode number and byte size.

### 2. Root inode and its dot entries

The root inode number is `1`. The captured listing showed both `.` and `..` with inode `1`. At the root, the parent is the root itself, so both entries refer to the same directory.

### 3. Why an empty directory occupies 32 bytes

Each `struct dirent` occupies 16 bytes: a 2-byte inode number and a 14-byte name field. A new directory contains `.` and `..`, giving `2 × 16 = 32` bytes. This is the size of its directory-entry data, not the total size of its descendants.

## Part 2 — mystat and hard links

### Program behavior

`user/mystat.c` calls `stat` for each supplied path, converts its numeric type to a readable name, and prints its inode, link count and size. A failed lookup is reported on file descriptor 2, and the loop continues with the next path. `$U/_mystat` is added to the Makefile's `UPROGS` list.

### 1. Outputs after creating and removing links

Initial captured output:

```text
$ mystat README . console
README: file, inode 2, links 1, size 2441 bytes
.: directory, inode 1, links 1, size 1024 bytes
console: device, inode 26, links 1, size 0 bytes
```

Two differently capitalized links were created in this run:

```text
$ ln README R2
$ ln README r2
$ mystat README r2
README: file, inode 2, links 3, size 2441 bytes
r2: file, inode 2, links 3, size 2441 bytes
$ rm README
$ mystat r2
r2: file, inode 2, links 2, size 2441 bytes
```

The names `README`, `R2` and `r2` referred to the same inode. xv6 distinguishes uppercase and lowercase, so the two link commands created distinct directory entries. Removing `README` reduced the count from 3 to 2; `R2` and `r2` remained. The inode number, type and size did not change. The surviving name still referred to inode 2, whose size remained 2441 bytes.

The captured attempts to run `cat r2 | wc` included typing/input errors and do not establish a successful word-count result. The verification commands are:

```sh
cat r2
wc r2
cat r2 | wc
```

### 2. Link-count decrement and block reclamation

In `kernel/sysfile.c`, `sys_unlink` decrements the target inode's link count with:

```c
ip->nlink--;
iupdate(ip);
```

Removing a directory entry does not immediately free a file if another hard link or an open reference remains. Once the link count is zero and the final in-memory reference is released, `iput` in `kernel/fs.c` calls `itrunc` to free the file's blocks and clears the inode's type. The log records file-system changes so multi-step operations can be committed consistently.

### 3. Fields of struct dinode

The base xv6 on-disk inode contains:

| Field | Meaning |
|---|---|
| `type` | Directory, regular file or device |
| `major` | Major device number, for device inodes |
| `minor` | Minor device number, for device inodes |
| `nlink` | Hard-link count |
| `size` | Logical size in bytes |
| `addrs[NDIRECT + 1]` | Direct block addresses and one indirect-block address |

The filename resides in a directory entry rather than in the inode. The base version used in this lab has no symbolic-link implementation or Unix ownership and permission fields.

## Part 3 — File-system limits

### 1. Long filename

```text
$ mkdir averyveryverylongname
$ ls
...
averyveryveryl 1 27 32
```

The name is truncated to `averyveryveryl`, the first 14 characters, because `DIRSIZ = 14`. A directory-entry name field is fixed-width and need not contain a terminating null byte when all 14 bytes are used.

### 2. Maximum file size

For the layout specified by the lab, `BSIZE = 1024`, `NDIRECT = 12`, and a block number occupies 4 bytes. One indirect block holds `1024 / 4 = 256` addresses.

```text
Maximum data blocks = 12 + 256 = 268
Maximum file size   = 268 × 1024
                    = 274,432 bytes
                    = 268 KiB
```

The indirect block itself consumes storage but does not count as file contents. Actual free space can limit a file before this addressing maximum is reached.

### 3. Measured disk layout

Ubuntu commands:

```sh
cd ~/xv6-riscv
make clean
make fs.img 2>&1 | grep nmeta
```

Captured output:

```text
nmeta 47 (boot, super, log blocks 31, inode blocks 13, bitmap blocks 1) blocks 1953 total 2000
```

| Region | Blocks |
|---|---:|
| Boot block and superblock | 2 |
| Log | 31 |
| Inode area | 13 |
| Free-block bitmap | 1 |
| Metadata total nmeta | 47 |
| Data area | 1953 |
| Whole image | 2000 |

```text
nmeta = 2 + 31 + 13 + 1 = 47 blocks
Total = 47 + 1953 = 2000 blocks
Image capacity = 2000 × 1024 = 2,048,000 bytes ≈ 1.95 MiB
```

The data area includes allocated and free blocks; 1953 is not a count of remaining free blocks.

## Part 4 — Recursive directory search

### Implementation

`user/find.c` accepts a starting directory and a target name. It compares the last component of each path using `strcmp`. For directories, it reads `struct dirent` entries, skips entries with `inum == 0`, and excludes `.` and `..` to avoid traversal loops. It copies fixed-width names into a null-terminated buffer, checks path-buffer capacity, and closes opened directories. `$U/_find` is added to `UPROGS`.

### Test commands

In Ubuntu, after saving both source files and Makefile entries:

```sh
make clean
make qemu
```

In xv6, one command per line:

```sh
mkdir a
mkdir a/b
echo x > a/b/r2
echo y > a/r2
ln cat r2
find . r2
find a b
```

### Recorded search results and explanation

The successful run in Figure 6.3 of the uploaded report shows:

```text
./a/b/r2
./a/r2
./r2
```

The command `find a b` printed:

```text
a/b
```

The search matches the final component rather than the complete path. It must visit nested directories and match directory names as well as regular files. `a/b/r2` and `a/r2` are separate files created by `echo`; root-level `r2` is a hard link to `cat` and shares its inode. Equal filenames do not imply equal inode identity.

Figure 6.3 records the completed build and successful searches. Earlier failed attempts occurred before `find` was added to the image. The two search commands in the final screenshot produced the paths listed above.

## Lab 6 conclusion

xv6 exposes directory entries and inode metadata directly. The measured hard-link experiment retained inode 2 and a size of 2441 bytes while the link count decreased from 3 to 2. The image build allocated 47 metadata blocks and 1953 data-area blocks. File-size limits follow from the inode's direct and indirect block pointers, while recursive search follows the directory-entry structure.

## Submission files

- Combined report with terminal screenshots.
- `notes.md` containing observation answers.
- `user/mystat.c`.
- `user/find.c`.
- `Makefile` with `$U/_mystat` and `$U/_find` in `UPROGS`.

## References

- UTM Operating Systems Lab 5 handout, *The File System with Your Own Hands*, 2026–2027.
- UTM Operating Systems Lab 6 handout, *Inside the xv6 File System*, 2026–2027.
- MIT xv6 source: https://github.com/mit-pdos/xv6-riscv
- Course repository: https://github.com/ilyyshub/Operating_Systems
