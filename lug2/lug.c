/*
  lug2
*/

#include <linux/init.h>
#include <linux/module.h>

static int __init lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");
	return 0;
}
module_init(lug_init);

static void __exit lug_exit(void)
{
	printk(KERN_INFO "lug_exit()\n");
}
module_exit(lug_exit);


MODULE_AUTHOR("Scott Ellis");
MODULE_DESCRIPTION("LUG driver");
MODULE_LICENSE("Dual BSD/GPL");
MODULE_VERSION("0.2-scott");

