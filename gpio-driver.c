/**
 * @file gpio-driver.c
 * @brief gpio driver for raspberry pi 4, linux kernel release 5.4 ++
 * @author Mouadh Dahech 
 * @date  October 22, 2021
 *
 */

#include <linux/kernel.h>
#include <linux/init.h>        /*Linux Headers files */
#include <linux/module.h>

#include <linux/proc_fs.h>
#include <linux/slab.h>

#include <asm/io.h>

#define MAX_USER_SIZE 1024

/**GPIO Base Address For:
 *  raspberry pi 4:      0xFE200000 
 *  Raspberry pi zero W: 0x20200000
 *  Raspberry pi 2 - 3 : 0x3F200000
 */
#define BCM2711_GPIO_ADDRESS 0xFE200000   

static struct proc_dir_entry *gpio_proc = NULL;  

static char data_buffer[MAX_USER_SIZE+1] = {0};

static unsigned int *gpio_registers = NULL;    


/***************************************************************************************************
*
*   FUNCTION NAME: gpio_pin_on
*
***************************************************************************************************/
/**
* @brief Function used to config the gpio pin as output and Turn on the led.
* Read BCM2711 ARM Peripherals Document to know how to access to GPIO regiters(pass the pinout command to display soc details) 
* link: https://pdf1.alldatasheet.com/datasheet-pdf/view/1283902/ETC1/BCM2711.html
*
* @par Parameters
* @param[in] pin: gpio pin to turn on 
* @return void.
*
***************************************************************************************************/
static void gpio_pin_on(unsigned int pin)
{
	unsigned int fsel_index = pin/10;
	unsigned int fsel_bitpos = pin%10;
	unsigned int* gpio_fsel = gpio_registers + fsel_index; /*pointer to the Configuration register*/
	unsigned int* gpio_on_register = (unsigned int*)((char*)gpio_registers + 0x1c); /*pointer to the Configuration register*/

	*gpio_fsel &= ~(7 << (fsel_bitpos*3)); /*clear the Configuration register*/
	*gpio_fsel |= (1 << (fsel_bitpos*3));  /*Set the gpio pin as Output*/
	*gpio_on_register |= (1 << pin);	   /*Set GPIO PIN(Turn on the led)*/

	return;
}

/***************************************************************************************************
*
*   FUNCTION NAME: gpio_pin_off
*
***************************************************************************************************/
/**
* @brief clear the GPIO pin.
* Read BCM2711 ARM Peripherals Document to know how to access to GPIO regiters(pass the pinout command to display soc details) 
* link: https://pdf1.alldatasheet.com/datasheet-pdf/view/1283902/ETC1/BCM2711.html
*
* @par Parameters
* @param[in] pin: gpio pin to turn on 
* @return void.
*
***************************************************************************************************/
static void gpio_pin_off(unsigned int pin)
{
	unsigned int *gpio_off_register = (unsigned int*)((char*)gpio_registers + 0x28); /*GPCLR0 Register Address*/
	*gpio_off_register |= (1<<pin); /*turn of the led*/
	return;
}

/***************************************************************************************************
*
*   FUNCTION NAME: gpio_read
*
***************************************************************************************************/
/**
* @brief Read the GPIO Pin.
* @par Parameters
* @param[in] file: kernel file structure that never appears in user programs 
* @param[in] user: buffer from the user 
* @param[in] size: the size ask to read
* @param[in] off: offset pointer into how much it's already read
* @return ssize_t : 7: Failed | 0: Successfully read.
*
***************************************************************************************************/
ssize_t gpio_read(struct file *file, char __user *user, size_t size, loff_t *off)
{
	return copy_to_user(user,"Hello!\n", 7) ? 0 : 7;
}

/***************************************************************************************************
*
*   FUNCTION NAME: gpio_write
*
***************************************************************************************************/
/**
* @brief Write To GPIO PIN.
* @par Parameters
* @param[in] file: kernel file structure that never appears in user programs 
* @param[in] user: buffer from the user 
* @param[in] size: the size ask to write
* @param[in] off: offset pointer into how much it's already read
* @return ssize_t : size. 
*
***************************************************************************************************/
ssize_t gpio_write(struct file *file, const char __user *user, size_t size, loff_t *off)
{
	unsigned int pin = UINT_MAX;
	unsigned int value = UINT_MAX;

	memset(data_buffer, 0x0, sizeof(data_buffer)); /*Clear The Buffer*/

	if (size > MAX_USER_SIZE)
	{
		size = MAX_USER_SIZE;
	}

	/*copy from user to data_buffer*/
	if (copy_from_user(data_buffer, user, size))
		return 0;

	printk("Data buffer: %s\n", data_buffer);

	if (sscanf(data_buffer, "%d,%d", &pin, &value) != 2) /*Read pin number and value from the user*/
	{
		printk("Inproper data format submitted\n");
		return size;
	}

	if (pin > 21 || pin < 0)
	{
		printk("Invalid pin number submitted\n");
		return size;
	}

	if (value != 0 && value != 1)
	{
		printk("Invalid on/off value\n");
		return size;
	}

	printk("You said pin %d, value %d\n", pin, value);
	if (value == 1)
	{
		gpio_pin_on(pin);
	} else if (value == 0)
	{
		gpio_pin_off(pin);
	}

	return size;
}



static const struct proc_ops proc_fops = 
{
	.proc_read = gpio_read,
	.proc_write = gpio_write,
};

/***************************************************************************************************
*
*   FUNCTION NAME: gpio_driver_init
*
***************************************************************************************************/
/**
* @brief this function run when the driver installed.
* @par Parameters
* @param[in] void   
* @return int : -1: Failed | 0: Successfully mapped in GPIO memory.
*
***************************************************************************************************/
static int __init gpio_driver_init(void)
{
	printk("Welcome to my driver!\n");
	
	gpio_registers = (int*)ioremap(BCM2711_GPIO_ADDRESS, PAGE_SIZE);
	if (gpio_registers == NULL)
	{
		printk("Failed to map GPIO memory to driver\n"); /*Print message to kernel system buffer access using dmesg*/
		return -1;
	}
	
	printk("Successfully mapped in GPIO memory\n");
	
	/* create an entry in the proc-fs (for linux kernel 5.4 ++)*/
	gpio_proc = proc_create("gpio-dr", 0666, NULL, &proc_fops); 
	if (gpio_proc == NULL)
	{
		return -1;
	}

	return 0;
}

/***************************************************************************************************
*
*   FUNCTION NAME: gpio_driver_exit
*
***************************************************************************************************/
/**
* @brief this function run when the driver uninstalled.
* @par Parameters
* @param[in] void   
* @return void
*
***************************************************************************************************/
static void __exit gpio_driver_exit(void)
{
	printk("Leaving my driver!\n");
	iounmap(gpio_registers);
	proc_remove(gpio_proc);
	return;
}

/* Run these functions at the boot up of the driver */
module_init(gpio_driver_init);
module_exit(gpio_driver_exit);

/* Some Additional information */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Mouadh Dahech");
MODULE_DESCRIPTION("Test of writing drivers for Raspberry pi 4");
MODULE_VERSION("1.0");