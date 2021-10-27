/*********************************** Raspberry pi GPIO driver *********************************/

1. Install Kernel headers:
    sudo apt install raspberrypi-kernel-headers 

2. chek linux version runing:
    uname -r

3. chek if build system installed
    ls /lib/modules/$(uname -r)/build

4. build the Project:
    make 

5. chek kernel drive system:
    lsmode


7. install: 
    sudo insmod gpio-driver.ko

8. chek:
    ls /proc | grep gpio    

9. chek kernel system buffer (Printk):
    dmesg

10. test the driver:
    Turn on the led: echo "21,1" > /proc/gpio-dr
    Turn off: echo "21,0" > /proc/gpio-dr

to clean the driver: 
    sudo rmmod gpio-driver