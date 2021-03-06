MSR-SAFE
========

[![Build Status](https://travis-ci.com/LLNL/msr-safe.svg?branch=master)](https://travis-ci.com/LLNL/msr-safe)

The msr-safe.ko module is comprised of the following source files:

    Makefile
    msr_entry.c         Original MSR driver with added calls to batch and
                        whitelist implementations.
    msr_batch.[ch]      MSR batching implementation
    msr_whitelist.[ch]  MSR whitelist implementation
    whitelists          Sample text whitelist that may be input to msr_safe

Kernel Build & Load
-------------------

Building and loading the msr-safe.ko module can be done with the commands
below. When no command line arguments are specified, the kernel will
dynamically assign major numbers to each device. A successful load of the
msr-safe kernel module will have `msr_batch` and `msr_whitelist` in `/dev/cpu`,
and will have an `msr_safe` present under each CPU directory in `/dev/cpu/*`.

    $ git clone https://github.com/LLNL/msr-safe
    $ cd msr-safe
    $ make
    $ insmod msr-safe.ko

Kernel Load with Command Line Arguments
---------------------------------------

Alternatively, this module can be loaded with command line arguments. The
arguments specify the major device number you want to associate with a
particular device. When loading the kernel, you can specify 1 or all 3 of the
msr devices.

    $ insmod msr-safe.ko mdev_msr_safe=<#> \
                         mdev_msr_whitelist=<#> \
                         mdev_msr_batch=<#>

Configuration Notes After Install
---------------------------------

Setup permissions and group ownership for `/dev/cpu/msr_batch`,
`/dev/cpu/msr_whitelist`, and `/dev/cpu/*/msr_safe` as you like since the
whitelist will protect you from harm.

Sample whitelists for specific architectures are provided in `whitelists/`
directory. These are meant to be a starting point, and should be used with
caution. Each site may add to, remove from, or modify the write masks in the
whitelist depending on specific needs. See the Troubleshooting section below
for more information.

To configure whitelist:

    cat whitelist/wl_file > /dev/cpu/msr_whitelist

Where `wl_file` can be determined as follows:

    printf 'wl_%.2x%x\n' $(lscpu | grep "CPU family:" | awk -F: '{print $2}') $(lscpu | grep "Model:" | awk -F: '{print $2}')

To confirm successful whitelist configured:

    cat /dev/cpu/msr_whitelist

To enumerate the current whitelist (i.e., implies whitelist was loaded
successfully):

    cat < /dev/cpu/msr_whitelist

To remove whitelist (as root):

    echo > /dev/cpu/msr_whitelist

msrsave
-------

The msrsave utility provides a mechanism for saving and restoring MSR values
based on entries in the whitelist. To restore MSR values, the register must
have an appropriate writemask.

Modification of MSRs that are marked as safe in the whitelist may impact
subsequent users on a shared HPC system. It is important the resource manager
on such a system use the msrsave utility to save and restore MSR values between
allocating compute nodes to users. An example of this has been implemented for
the SLURM resource manager as a SPANK plugin. This plugin can be built with the
"make spank" target and installed with the "make install-spank" target. This
uses the SLURM SPANK infrastructure to make a popen(3) call to the msrsave
command line utility in the job epilogue and prologue.

Troubleshooting
---------------

If you encounter errors attempting to read a particular MSR, it may be for
several reasons:

If you encounter a "Permission denied" error, likely the MSR was not exposed
in the current whitelist.

It is possible that the MSR you are attempting to read is not supported by
your CPU. You will likely see this if attempting to use the msrsave utility.
In that case, you should see an error message like the following:

    Warning: Failed to read msr value ...

These messages are benign and should not interfere with msrsave's ability to
save and restore MSR values that are currently supported. If it is desired to
remove the warning messages, remove the corresponding entry from the whitelist.

Release
-------

msr-safe is released under the GPLv3 license. For more details, please see the
[LICENSE](https://github.com/LLNL/msr-safe/blob/master/LICENSE) file.
