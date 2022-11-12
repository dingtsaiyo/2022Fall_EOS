#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()
#include <linux/gpio.h> //GPIO

//LED is connected to this GPIO
#define GPIO_5 (5)
#define GPIO_6 (6)
#define GPIO_20 (20)
#define GPIO_21 (21)

#define BUF_SIZE 9

static struct gpio leds[] = {
    {  GPIO_5, GPIOF_OUT_INIT_LOW, "LED_0" },
    {  GPIO_6, GPIOF_OUT_INIT_LOW, "LED_1" },
    {  GPIO_20, GPIOF_OUT_INIT_LOW, "LED_2" },
    {  GPIO_21, GPIOF_OUT_INIT_LOW, "LED_3" }
};

dev_t deviceNO = 0;
static struct class *dev_class;
static struct cdev etx_cdev;

static int __init etx_driver_init(void);
static void __exit etx_driver_exit(void);

/*************** Driver functions **********************/
static int etx_open(struct inode *inode, struct file *file);
static int etx_release(struct inode *inode, struct file *file);
static ssize_t etx_read(struct file *filp,
	char __user *buf, size_t len,loff_t * off);
static ssize_t etx_write(struct file *filp,
	const char *buf, size_t len, loff_t * off);
/******************************************************/

//File operation structure
static struct file_operations fops =
{
	.owner = THIS_MODULE,
	.read = etx_read,
	.write = etx_write,
	.open = etx_open,
	.release = etx_release,
};


/*
** This function will be called when we open the Device file
*/
static int etx_open(struct inode *inode, struct file *file)
{
	pr_info("Device File Opened...!!!\n");
	return 0;
}

/*
** This function will be called when we close the Device file
*/
static int etx_release(struct inode *inode, struct file *file)
{
	pr_info("Device File Closed...!!!\n");
	return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t etx_read(struct file *filp,
	char __user *buf, size_t len, loff_t *off)
{
	uint8_t gpio_state = 0;
	
	//get specific GPIO to be read
	unsigned int gpio = iminor(filp->f_path.dentry->d_inode);
	
	//reading GPIO value
	gpio_state = gpio_get_value(leds[gpio].gpio);
	
	//write to user
	if(copy_to_user(buf, &gpio_state, len) > 0) {
		pr_err("ERROR: Not all the bytes have been copied to user\n");
	}
	
	return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t etx_write(struct file *filp,
	const char __user *buf, size_t len, loff_t *off)
{
	uint8_t rec_buf[BUF_SIZE] = {0};
	
	//get specific GPIO to be written
	unsigned int gpio = iminor(filp->f_path.dentry->d_inode);
	unsigned int length = len < BUF_SIZE ? len-1 : BUF_SIZE-1;
	if(copy_from_user(rec_buf, buf, length) > 0) {
		pr_err("ERROR: Not all the bytes have been copied from user\n");
	}
	
	//binary convert
	//binary[0] is least significant bit
	uint8_t binary[4] = {0};
	uint8_t i = 0, number = rec_buf[0] - '0';
	while (number > 0) {
		binary[i++] = number % 2;
		number /= 2;
	}

	//display LEDs
	//GPIO 5 is least significant bit
	if (binary[gpio] == 1) {
		//set the GPIO value to HIGH
		gpio_set_value(leds[gpio].gpio, 1);
	} else if (binary[gpio] == 0) {
		//set the GPIO value to LOW
		gpio_set_value(leds[gpio].gpio, 0);
	} else {
		pr_err("Unknown command : Please provide either 1 or 0 \n");
	}
	
	pr_info("Complete writing...!!!\n");
	return len;
}

/*
** Module Init function
*/
static int __init etx_driver_init(void)
{
	int i;
	
	/*Allocating Major number*/
	if((alloc_chrdev_region(&deviceNO, 0, ARRAY_SIZE(leds), "etx_Dev")) <0){
		pr_err("Cannot allocate major number\n");
		goto r_unreg;
	}
	pr_info("Major = %d Minor = %d \n", MAJOR(deviceNO), MINOR(deviceNO));
	
	/*Creating cdev structure*/
	cdev_init(&etx_cdev, &fops);
	
	/*Adding character device to the system*/
	if((cdev_add(&etx_cdev, deviceNO, ARRAY_SIZE(leds))) < 0){
		pr_err("Cannot add the device to the system\n");
		goto r_del;
	}
	
	/*Creating struct class*/
	if((dev_class = class_create(THIS_MODULE, "etx_class")) == NULL){
		pr_err("Cannot create the struct class\n");
		goto r_class;
	}
	
	/*Creating device*/
	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		if (device_create(dev_class, NULL, MKDEV(MAJOR(deviceNO), MINOR(deviceNO) + i),
		                  NULL, leds[i].label) == NULL) {
		    pr_err( "Cannot create the Device \n");
		    goto r_device;
		}
	}
	
	//Checking the GPIO is valid or not
	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		if(gpio_is_valid(leds[i].gpio) == false){
			pr_err("GPIO %d is not valid\n", leds[i].gpio);
			goto r_device;
		}
	}
	
	//Requesting the GPIO
	if(gpio_request_array(leds, ARRAY_SIZE(leds)) < 0){
		pr_err("ERROR: GPIO array request\n");
		goto r_gpio;
	}
		
	//configure the GPIO as output
	//gpio_direction_output(GPIO_21, 0);
	
	/* Using this call the GPIO 21 will be visible in /sys/class/gpio/
	** Now you can change the gpio values by using below commands also.
	** echo 1 > /sys/class/gpio/gpio21/value (turn ON the LED)
	** echo 0 > /sys/class/gpio/gpio21/value (turn OFF the LED)
	** cat /sys/class/gpio/gpio21/value (read the value LED)
	**
	** the second argument prevents the direction from being changed.
	*/
	for (i = 0; i < ARRAY_SIZE(leds); ++i)
		gpio_export(leds[i].gpio, false);
	
	pr_info("Device Driver Insert...Done!!!\n");
	return 0;
	
r_gpio:
	gpio_free_array(leds, ARRAY_SIZE(leds));
r_device:
	for (i = 0; i < ARRAY_SIZE(leds); i++) {
        gpio_set_value(leds[i].gpio, 0);
        device_destroy(dev_class, MKDEV(MAJOR(deviceNO), MINOR(deviceNO) + i));
    }
r_class:
	class_destroy(dev_class);
r_del:
	cdev_del(&etx_cdev);
r_unreg:
	unregister_chrdev_region(deviceNO, ARRAY_SIZE(leds));
	
	return -1;
}

/*
** Module exit function
*/
static void __exit etx_driver_exit(void)
{
	int i;
	for (i = 0; i < ARRAY_SIZE(leds); ++i) {
		gpio_set_value(leds[i].gpio, 0);
		gpio_unexport(leds[i].gpio);
		device_destroy(dev_class, MKDEV(MAJOR(deviceNO), MINOR(deviceNO) + i));
	}
	
	class_destroy(dev_class);
	cdev_del(&etx_cdev);
	unregister_chrdev_region(deviceNO, ARRAY_SIZE(leds));
	gpio_free_array(leds, ARRAY_SIZE(leds));
	pr_info("Device Driver Remove...Done!!\n");
}

module_init(etx_driver_init);
module_exit(etx_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("handian-yang <smalliron987.eed06@g2.nctu.edu.tw>");
MODULE_DESCRIPTION("lab3_driver");
