#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

struct lug_dev {
	dev_t devt;
	struct cdev cdev;
	struct semaphore sem;
};

static struct lug_dev lug_dev;


static int lug_open(struct inode *inode, struct file *filp)
{	
	int status = 0;

	if (down_interruptible(&lug_dev.sem)) 
		return -ERESTARTSYS;

	printk(KERN_INFO "lug_open()\n");

	up(&lug_dev.sem);

	return status;
}

static const struct file_operations lug_fops = {
	.owner = THIS_MODULE,
	.open =	lug_open,	
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
}
module_exit(lug_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("LUG driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.5-scott");

