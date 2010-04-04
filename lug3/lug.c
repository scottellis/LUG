#include <linux/init.h>
#include <linux/module.h>

static int int_param = 0;
module_param(int_param, int, S_IRUGO);
MODULE_PARM_DESC(int_param, "LUG module parameter");

static char *str_param = "Not set";
module_param(str_param, charp, 0000); 
MODULE_PARM_DESC(str_param, "Another LUG module parameter");

static int __init lug_init(void)
{
	printk(KERN_INFO "lug_init()\n");
	printk(KERN_INFO "int_param = %d\n", int_param);
	printk(KERN_INFO "str_param = %s\n", str_param);

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
MODULE_VERSION("0.3-scott");

