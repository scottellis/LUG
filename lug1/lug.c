#include <linux/init.h>
#include <linux/module.h>

static int lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");
	return 0;
}
module_init(lug_init);

static void lug_exit(void)
{
	printk(KERN_INFO "lug_exit()\n");
}
module_exit(lug_exit);

