# ledspire
Programmable LED driver with ESP8266/Arduino.

Example programs are in `fs/progs`.

There are 10 registers: v0 through v9. Command reference (`:r` means a register, `:v` means a register or an immediate value):

`wait delay:v` - wait `delay` milliseconds  
`fade r:v g:v b:v time:v` - linear fade from the current color to `r,g,b` in `time` milliseconds  
`stop` - stop the interpreter  
`rand dest:r min:v max:v` - set `dest` to a random number between `min` and `max` inclusive  
`call label` - call a subroutine  
`ret` - return from a subroutine  
`rgb r:v g:v b:v` - set LED color to `r,g,b`  
`add dest:r src:v` - add `src` to `dest`  
`sub dest:r src:v` - subtract `src` from `dest`  
`inc dest:r` - increment `dest`  
`dec dest:r` - decrement `dest`  
`mul dest:r src:v` - multiply `dest` by `src`  
`div dest:r src:v` - divide `dest` by `src`  
`cmp a:v b:v` - set condition flags according to the result of `a - b`. Note: only `cmp` updates the flags.  
`jmp label` - jump to a label  
(`jne`, `jge`, `jle`, `je`, `jg`, `jl`) `label` - conditionally jump to a label  
`ld dest:r src:v` - load `src` into `dest`  