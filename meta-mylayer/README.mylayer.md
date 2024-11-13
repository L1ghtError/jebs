## mylayer
====
Layer that provides hwdbg (hardware debug) kernel module

### Instalation Guide 
-----------------------
1. Tested and expected to run on [poky](https://git.yoctoproject.org/poky/) based project
2. Copy this layer to root folder of your yocto project
3. in root dir run:`source oe-init-build-env`
4. Expected to run on `MACHINE ??= "qemux86"` with `IMAGE_INSTALL:append = "hwdbg-mod"`
5. build with `bitbake core-image-full-cmdline`
6. run with `runqemu core-image-full-cmdline`
### Usage 
-----------------------
1. To trace logs `dmesg | grep HWDBG`
2. to set specify object to debug use: `echo 0x<addr> > /sys/kernel/hwdbg/target_varible`
3. for debuging purpose you can use dummy varible to trace its behavior, example:

`
$ dmesg | grep HWD
`

[  3.551482] HWDG Dummy Value is:acabacab:0xd3786010

`
$ echo 0xd3786010 > /sys/kernel/hwdbg/target_varible
`
`
$ cat /sys/kernel/hwdbg/target_varible
`
d3786010
`
$ cat /sys/kernel/hwdbg/target_varible
`
d3786010
`
$ cat /sys/kernel/hwdbg/target_varible
`
d3786010
`
$ cat /sys/kernel/hwdbg/target_varible
`
d3786010
`
$ cat /sys/kernel/hwdbg/target_varible
`
d3786010
`
$ dmesg | grep HWD
`
[  3.551482] HWDG Dummy Value is:acabacab:0xd3786010
[ 30.133775] HWDG read-write target_varible = acabacab
[ 30.1334316] HWDG New Value is:0xd3786010! dummy:acabacab
[ 42.849918] HWDG write target_varible = deadbef0
[ 43.475620] HWDG write target_varible = deadbef1
[ 43.939688] HWDG write target_varible = deadbef2
### Restrictions
-----------------------
* Reading from the `target_variable` sysfs has a side effect that increments a dummy counter, which was created to debug the write breakpoint.
* For some reason, the first write to the `dummy` variable isn’t caught by the breakpoint (four calls to `cat` result in three write outputs).
