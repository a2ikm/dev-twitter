#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>

#define MODNAME "twitter"
#define MINOR_COUNT 1

static dev_t twitter_dev_t;
static struct cdev twitter_cdev;

static int twitter_open(struct inode* inode, struct file* fp)
{
  return 0;
}

static int twitter_release(struct inode* inode, struct file* fp)
{
  return 0;
}

static ssize_t twitter_read(struct file* fp, char* buf, size_t count, loff_t* offset)
{
  buf = "";
  return 0;
}

static ssize_t twitter_write(struct file* fp, const char* buf, size_t count, loff_t* offset)
{
  return 0;
}

static struct file_operations twitter_fops = {
  .owner   = THIS_MODULE,
  .open    = twitter_open,
  .release = twitter_release,
  .read    = twitter_read,
  .write   = twitter_write,
};

static int __init twitter_init(void)
{
  int ret;

  ret = alloc_chrdev_region(&twitter_dev_t,
                            0,
                            MINOR_COUNT,
                            MODNAME);
  if (ret < 0) {
    printk(KERN_WARNING "alloc_chrdev_region failed\n");
    return ret;
  }

  cdev_init(&twitter_cdev, &twitter_fops);
  twitter_cdev.owner = THIS_MODULE;

  ret = cdev_add(&twitter_cdev, twitter_dev_t, MINOR_COUNT);
  if (ret < 0) {
    printk(KERN_WARNING "cdev_add failed\n");
    return ret;
  }

  printk(KERN_INFO "twitter is loaded\n");
  printk(KERN_INFO "twitter : major = %d\n", MAJOR(twitter_dev_t));
  printk(KERN_INFO "twitter : minor = %d\n", MINOR(twitter_dev_t));
  return 0;
}

static void __exit twitter_exit(void)
{
  cdev_del(&twitter_cdev);
  unregister_chrdev_region(twitter_dev_t, MINOR_COUNT);
  printk(KERN_INFO "twitter id unloaded\n");
}

module_init(twitter_init);
module_exit(twitter_exit);

MODULE_DESCRIPTION("twitter");
MODULE_LICENSE("BSD");
