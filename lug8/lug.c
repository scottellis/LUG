/*
  lug8
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>
#include <linux/string.h>
#include <linux/kernel.h>

#define USER_BUFF_SIZE 128

static int read_counter = 0;
module_param(read_counter, int, S_IRUGO);
MODULE_PARM_DESC(read_counter, "Keep track of file read calls");


struct lug_dev {
	dev_t devt;
	struct cdev cdev;
	struct semaphore sem;
	char *user_buff;
};

static struct lug_dev lug_dev;


static ssize_t lug_write(struct file *filp, const char __user *buff,
		size_t count, loff_t *f_pos)
{
	ssize_t status;
	unsigned long val;
	size_t len = USER_BUFF_SIZE - 1;

	if (count == 0)
		return 0;

	if (down_interruptible(&lug_dev.sem))
		return -ERESTARTSYS;

	if (len > count)
		len = count;
	
	memset(lug_dev.user_buff, 0, USER_BUFF_SIZE);

	if (copy_from_user(lug_dev.user_buff, buff, len)) {
		status = -EFAULT;
		goto lug_write_done;
	}

	val = simple_strtoul(lug_dev.user_buff, NULL, 0);

	if (val < 1000)
		read_counter = val;

	status = len;
	*f_pos += len;

lug_write_done:

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
		goto lug_read_done;
	} 

	*offp += len;
	status = len;

lug_read_done:
				
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

static int __init lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");
	
	memset(&lug_dev, 0, sizeof(struct lug_dev));

	sema_init(&lug_dev.sem, 1);

	if (lug_init_cdev())
		return -1;	

	printk(KERN_INFO "Run : mknod /dev/lug c %d %d\n", 
			MAJOR(lug_dev.devt), MINOR(lug_dev.devt));

	return 0;
}
module_init(lug_init);

static void __exit lug_exit(void)
{
	printk(KERN_INFO "lug_exit()\n");

	cdev_del(&lug_dev.cdev);
	unregister_chrdev_region(lug_dev.devt, 1);

	if (lug_dev.user_buff)
		kfree(lug_dev.user_buff);
}
module_exit(lug_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("LUG driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.8-scott");

