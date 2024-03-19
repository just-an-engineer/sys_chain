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
- Do more testing of conditionals, such as having multiple (which I have not done)
- Make a better way to handle errors in conditionals. I don't like necessarily jumping to functions (which currently, if those functions return, would jump back into the syscall code, but that's because it's userspace code, in kernelspace we could overwrite rip or something).
- Make some code that implements and tests it in the Linux kernel
- Do it again in Rust, for the funsies
- Add a compiler pass. See below

# Compiler pass
Ultimately, this work is useless if 1 of 2 things do not happen. 1, programmers manually write the code to use this syscall by hand. Which is very much error prone, and uses lots of static values that can increase either config or const sections, or is a place you need to keep in mind for updating. Or 2, we make a compiler pass that can detect syscalls made, and which ones can be combined, by analyzing what syscalls depend on further userspace calculation of return values, or later objects. We would need to have a way to detect error checking after syscalls.

# How to contribute
Feel free to fork and make a PR, or start an issue for cool features, bugs, security issues, or anything else. Be courteous

# License stuff
Idk man, don't use this to run your grandma's life support, don't use this in your production server's kernel. I'll add a license later