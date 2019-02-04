
ABOUT:
In this assignment, we deal with the measurement of interrupt latency with and without background computing and context switching overhead.
Context switching is the process of storing the state of an executing thread and restoring the context of the new executing thread. The delay associated with it is known as context switching overhead.
Interrupt latency overhead is the delay between the assertion of the interrupt and the execution of the interrupt service routine. Here, we create a histogram to observe the changes in the various cases,


________________


SYSTEM:
-Zephyr OS V 1.10.0 and SDK 0.8.2 with Cmake version 3.8.2 or above
-Linux as a host machine
-Intel Galileo Gen 2 board
-LED


________________


SETUP:
- Install Zephyr
-Format SD Card as FAT
- Create the  directories
  efi
  efi/boot
  kernel
- after cloning find the binary at $ZEPHYR_BASE/boards/x86/galileo/support/grub/bin/grub.efi and copy it to $SDCARD/efi/boot and rename it to bootia32.efi.
- Create a $SDCARD/efi/boot/grub.cfg file containing: 
  set default=0
  set timeout=10
  menuentry "Zephyr Kernel" {multiboot /kernel/zephyr.strip}

- Make sure that cmake version is 3.8.2 or higher
- Export ZEPHYR_GCC_VARIANT=zephyr
- Export ZEPHYR_SDK_INSTALL_DIR=<sdk installation directory>
- Export ZEPHYR_TOOLCHAIN_VARIANT=zephyr
- Connect GPIO pins on Galileo Gen 2, IO5 is PWM and IO2 is input pin.


________________


COMPILATION
-> type: cd $ZEPHYR_BASE/samples/measure_02
-> mkdir build
-> cd build
-> cmake -DBOARD=galileo ..
-> make

-> Copy zephyr.strip file to in $ZEPHYR_BASE/samples/measure_n/build/zephyr to $SDCARD/kernel
-> Put SD Card in board and reboot.



Shell module:(Execution Instructions)
________________


-> After booting Zephyr into the Galileo board,type ‘help’. This will display all the available modules.
->Select the module ‘Results’. After you type ‘help’ again,there will have various methods available.
->Select the one that you want for example, if you want to measure the interrupt latency with background computing, enter in ‘1’.
->You will be able to get the values of 500 different sample values respectively for the three measurements.(namely interrupt latency with background computing, without background computing and context switching overhead).
