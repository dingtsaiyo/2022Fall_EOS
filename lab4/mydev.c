#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/uaccess.h> //copy_to/from_user()

#define BUF_SIZE 20
#define DEV_SIZE 16

#define MAJOR_NUM 220
#define DEVICE_NAME "mydev"

// dev_t deviceNO = 0;
// static struct class *dev_class;
static struct my_device_data {
	struct cdev cdev;
	ssize_t buffer[BUF_SIZE];
	ssize_t size;
} mydev;

static char seg[27][16] = {
    "1111001100010001", // A
    "0000011100000101", // b
    "1100111100000000", // C
    "0000011001000101", // d
    "1000011100000001", // E
    "1000001100000001", // F
    "1001111100010000", // G
    "0011001100010001", // H
    "1100110001000100", // I
    "1100010001000100", // J
    "0000000001101100", // K
    "0000111100000000", // L
    "0011001110100000", // M
    "0011001110001000", // N
    "1111111100000000", // O
    "1000001101000001", // P
    "0111000001010000", // q
    "1110001100011001", // R
    "1101110100010001", // S
    "1100000001000100", // T
    "0011111100000000", // U
    "0000001100100010", // V
    "0011001100001010", // W
    "0000000010101010", // X
    "0000000010100100", // Y
    "1100110000100010", // Z
    "0000000000000000"	// 0
};

static int __init my_init(void);
static void __exit my_exit(void);

/*************** Driver functions **********************/
static int my_open(struct inode *inode, struct file *file);
static int my_release(struct inode *inode, struct file *file);
static ssize_t my_read(struct file *file,
	char __user *user_buffer, size_t size, loff_t * offset);
static ssize_t my_write(struct file *file,
	const char __user *user_buffer, size_t size, loff_t * offset);
/******************************************************/

//File operation structure
static struct file_operations my_fops =
{
	.owner = THIS_MODULE,
	.read = my_read,
	.write = my_write,
	.open = my_open,
	.release = my_release
};


/*
** This function will be called when we open the Device file
*/
static int my_open(struct inode *inode, struct file *file)
{
	struct my_device_data *my_data;

	my_data = container_of(inode->i_cdev, struct my_device_data, cdev);

	file->private_data = my_data;

	printk("Device File Opened...!!!\n");
	return 0;
}

/*
** This function will be called when we close the Device file
*/
static int my_release(struct inode *inode, struct file *file)
{
	printk("Device File Closed...!!!\n");
	
	return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t my_read(struct file *file,
	char __user *user_buffer, size_t size, loff_t *offset)
{
	struct my_device_data *my_data = (struct my_device_data *) file->private_data;

	// Dealing with letters (case-insensitive)
	ssize_t letter;
	if (my_data->buffer[0] >= 97 && my_data->buffer[0] <= 122)
		letter = my_data->buffer[0] - 97;
	else if (my_data->buffer[0] >= 65 && my_data->buffer[0] <= 90)
		letter = my_data->buffer[0] - 65;
	else if (my_data->buffer[0] == 48)
		letter = 26;
	else
		letter = 26;
	
	/* read data from my_data->buffer to user buffer */
	if (copy_to_user(user_buffer, seg[letter], size))
		return -EFAULT;

	printk("Data read...!!!\n");
	return size;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t my_write(struct file *file,
	const char __user *user_buffer, size_t size, loff_t *offset)
{
	struct my_device_data *my_data = (struct my_device_data *) file->private_data;

	/* read data from user buffer to my_data->buffer */
	if (copy_from_user(my_data->buffer, user_buffer, size))
		return -EFAULT;

	printk("Data written...!!!\n");
	return size;
}

/*
** Module Init function
*/
static int __init my_init(void)
{
	printk("Call init\n");
	if(register_chrdev(MAJOR_NUM, DEVICE_NAME, &my_fops) < 0) {
		pr_err("Can not get major %d\n", MAJOR_NUM);
		return (-EBUSY);
	}
	
	/*Creating cdev structure*/
	cdev_init(&mydev.cdev, &my_fops);

	/*Adding character device to the system*/
	if(cdev_add(&mydev.cdev, MKDEV(MAJOR_NUM, 0), 1) < 0) {
		pr_err("Cannot add the device to the system\n");
		cdev_del(&mydev.cdev);
		return -1;
	}
	
	printk("My device is started and the major is %d\n", MAJOR_NUM);
	return 0;
}

/*
** Module exit function
*/
static void __exit my_exit(void)
{
	cdev_del(&mydev.cdev);
	unregister_chrdev(MAJOR_NUM, DEVICE_NAME);

	printk("Call exit\n");
}

module_init(my_init);
module_exit(my_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Handian Yang <smalliron987.eed06@g2.nctu.edu.tw>");
MODULE_DESCRIPTION("My driver");
