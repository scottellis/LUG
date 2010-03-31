/*
  lug9
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/device.h>

#define USER_BUFF_SIZE 128

static int read_counter = 0;
module_param(read_counter, int, S_IRUGO);
MODULE_PARM_DESC(read_counter, "Keep track of file read calls");


struct lug_dev {
	dev_t devt;
	struct cdev cdev;
	struct semaphore sem;
	struct class *class;
	char *user_buff;
};

static struct lug_dev lug_dev;


static ssize_t lug_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *f_pos)
{
	unsigned long val;
	size_t len = USER_BUFF_SIZE - 1;

	if (count == 0)
		return 0;

	if (down_interruptible(&lug_dev.sem))
		return -ERESTARTSYS;

	if (len > count)
		len = count;
	
	memset(lug_dev.user_buff, 0, USER_BUFF_SIZE);
	strncpy(lug_dev.user_buff, buff, len);

	val = simple_strtoul(lug_dev.user_buff, NULL, 0);

	if (val < 1000)
		read_counter = val;

	up(&lug_dev.sem);

	return count;
}

static ssize_t lug_read(struct file *filp, char __user *buff, size_t count,
			loff_t *offp)
{
	ssize_t status;
	size_t len;

	if (*offp > 0)
		return 0;

	if (down_interruptible(&lug_dev.sem)) 
		return -ERESTARTSYS;

	read_counter++;

	len = sprintf(lug_dev.user_buff, "read_counter = %d\n", read_counter);

	if (len > count)
		len = count;

	if (copy_to_user(buff, lug_dev.user_buff, len)) {
		printk(KERN_ALERT "lug_read(): copy_to_user() failed\n");
		status = -EFAULT;
	} else {
		*offp += len;
		status = len;
	}
				
	up(&lug_dev.sem);
	
	return status;	
}

static int lug_open(struct inode *inode, struct file *filp)
{	
	int status = 0;

	if (down_interruptible(&lug_dev.sem)) 
		return -ERESTARTSYS;
	
	if (!lug_dev.user_buff) {
		lug_dev.user_buff = kmalloc(USER_BUFF_SIZE, GFP_KERNEL);

		if (!lug_dev.user_buff) {
			printk(KERN_ALERT 
				"lug_open: user_buff alloc failed\n");

			status = -ENOMEM;
		}
	}

	up(&lug_dev.sem);

	return status;
}

static const struct file_operations lug_fops = {
	.owner = THIS_MODULE,
	.open =	lug_open,	
	.read =	lug_read,
	.write = lug_write,
};

static int __init lug_init_cdev(void)
{
	int error;

	lug_dev.devt = MKDEV(0, 0);

	error = alloc_chrdev_region(&lug_dev.devt, 0, 1, "lug");
	if (error < 0) {
		printk(KERN_ALERT 
			"alloc_chrdev_region() failed: error = %d \n", 
			error);
		
		return -1;
	}

	cdev_init(&lug_dev.cdev, &lug_fops);
	lug_dev.cdev.owner = THIS_MODULE;

	error = cdev_add(&lug_dev.cdev, lug_dev.devt, 1);
	if (error) {
		printk(KERN_ALERT "cdev_add() failed: error = %d\n", error);
		unregister_chrdev_region(lug_dev.devt, 1);
		return -1;
	}	

	return 0;
}

static int __init lug_init_class(void)
{
	lug_dev.class = class_create(THIS_MODULE, "lug");

	if (!lug_dev.class) {
		printk(KERN_ALERT "class_create(lug) failed\n");
		return -1;
	}

	return 0;
}

static int __init lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");
	
	memset(&lug_dev, 0, sizeof(struct lug_dev));

	sema_init(&lug_dev.sem, 1);

	if (lug_init_cdev())
		goto init_fail_1;

	if (lug_init_class())
		goto init_fail_2;
		
	printk(KERN_INFO "Run : mknod /dev/lug c %d %d\n", 
			MAJOR(lug_dev.devt), MINOR(lug_dev.devt));

	return 0;

init_fail_2:
	cdev_del(&lug_dev.cdev);
	unregister_chrdev_region(lug_dev.devt, 1);

init_fail_1:
	return -1;
}
module_init(lug_init);

static void __exit lug_exit(void)
{
	printk(KERN_INFO "lug_exit()\n");

  	class_destroy(lug_dev.class);

	cdev_del(&lug_dev.cdev);
	unregister_chrdev_region(lug_dev.devt, 1);

	if (lug_dev.user_buff)
		kfree(lug_dev.user_buff);
}
module_exit(lug_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("LUG driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.9-scott");

