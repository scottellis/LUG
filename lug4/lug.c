/*
  lug4
*/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>

struct lug_dev {
	dev_t devt;
	struct cdev cdev;
};

static struct lug_dev lug_dev;

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

	printk(KERN_INFO 
		"lug char device MAJOR = %d MINOR = %d\n",
		MAJOR(lug_dev.devt), 
		MINOR(lug_dev.devt));
		
	return 0;
}

static int __init lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");

	if (lug_init_cdev())
		return -1;	

	return 0;
}
module_init(lug_init);

static void __exit lug_exit(void)
{
	printk(KERN_INFO "lug_exit()\n");

	unregister_chrdev_region(lug_dev.devt, 1);
}
module_exit(lug_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("LUG driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.4-scott");

