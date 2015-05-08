#include <linux/module.h>
#include <linux/kernel.h>

static int __init twitter_init(void)
{
  printk(KERN_INFO "twitter is loaded\n");
  return 0;
}

static void __exit twitter_exit(void)
{
  printk(KERN_INFO "twitter id unloaded\n");
}

module_init(twitter_init);
module_exit(twitter_exit);

MODULE_DESCRIPTION("twitter");
MODULE_LICENSE("BSD");
