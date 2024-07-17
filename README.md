# What is this?
This is a proof of concept for a new syscall that chains together multiple syscalls into 1. While each syscall still gets called (preventing us from having to reimplement each one, with all security features, and all the other nightmares that would bring), there is only 1 context switch, which could shave off multiple microseconds for even sample programs.
However, this POC is implemented in userspace, so no gains can be measured or observed.

# How does it work?
Currently, you create an array of unsigned longs of the following structure:
```
syscall number,
bitmap,
args // (if any),
error_function,
conditions,
conditional_value
```

The bitmap defines how many bits are used for arguments (currently 3, as on Linux, no syscall has more than 6 arguments), and how many conditions are used (currently 3, as I figured that's likely more than enough for testing the return value of a single syscall. Feel free to change it, though). And there are 6 bits for marking which registers (arguments) are symbolic, rather than the actual value they hold. The remaining bytes are reserved.
If a value is symbolic, then it represents the return value of the nth syscall, 0 indexed.
A note: currently, since we only need 4 bits to store a conditional, we store 2 conditionals in 1 byte. That means on a 64-bit system, we can store 16 conditionals in 1 unsigned long, which is more than we currently allow, but the infrastructure is there if you want to expand it.

So following the example in `main.c`, the last 3 syscalls use the return value of the first syscall (open, which returns the fd of the file opened) as the arguments for the fd for the next syscalls.
You can define a function to go to in the case of one of your conditionals being true. After a syscall is made, it runs through all conditionals, testing them. If the error function is 0 (or null), then it simply returns. 

# Work to do
- [ ] Do more testing of conditionals, such as having multiple (which I have not done)
- [ ] Make a better way to handle errors in conditionals. I don't like necessarily jumping to functions (which currently, if those functions return, would jump back into the syscall code, but that's because it's userspace code, in kernelspace we could overwrite rip or something).
- [ ] Fix file and directory organization, improve Makefiles, etc. This current organization is the result of me wanting to break up functionality out of just 1 file, but messing up terribly in the process. However, it was such a process debugging small things that resulted from breaking it into files
  - [ ] I also want to make an internal directory, and external/include API/header, so programmers can use constant functions, compile time stuff, or macro stuff to set up the syscall. IE, look at src/userspace/main.c for the LESS_SIGNED, BITMAP_CREATOR, etc. These need to be able to be imported and used in userspace (when this program is in the kernel), but they don't need access to the actual function and all those internal mechanics.
- [ ] Test theoretical performance gains
  - [ ] Write a test case that just calls a fast, non-blocking, etc, syscall, a few billion times. Time it and divide by number of calls to get average time per syscall
    - _Note: I need to try setting the nice value and other things to avoid the thread being preempted, but I'll explore more into that later_
- [ ] Implement locally
  - [ ] Build out 2 files with identical functionalities
  - [ ] Add the syscall to a local kernel, and compile
  - [ ] Run the test case on both, and compare
  - [ ] This will likely need to be implemented (if trying to do in real-world), in standard libraries, or directly via the provided macros. I don't think you can rely on compiler plugins to be able to scan for certain syscalls (without hardcoding the names of the syscall functions provided by your stdlib, the logic for dependencies, etc). Although you _could_, and it would make it more usable, but would be very libc specific, and then you get into a whole other area of shenanigans. Go for it if you want, though. But next steps in real world implementations would add to custom c libaries, after adding to any desired kernels
- [ ] Do it again in Rust, for the funsies

# How to contribute
Feel free to fork and make a PR, or start an issue for cool features, bugs, security issues, or anything else. Be courteous

# License stuff
Idk man, don't use this to run your grandma's life support, don't use this in your production server's kernel. I'll add a license later