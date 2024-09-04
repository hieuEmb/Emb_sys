#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>

static int __init hello_driver_init(void){
	printk("Driver was installed \n");
	return 0;
}

static void __exit hello_driver_exit(void){
	printk("Driver was removed \n");
}

module_init(hello_driver_init);
module_exit(hello_driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("CDT");
MODULE_VERSION("0.1");
