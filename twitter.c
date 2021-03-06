#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/socket.h>
#include <linux/in.h>
#include <linux/inet.h>
#include <linux/net.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/uio.h>
#include <asm/uaccess.h>

#define MODNAME "twitter"
#define MINOR_COUNT 1

static dev_t twitter_dev_t;
static struct cdev twitter_cdev;
static struct socket* twitter_sock;

struct socket* ktcp_sock_connect(const char* ip_addr, unsigned int port)
{
  struct socket* sock;
  struct sockaddr_in* server;
  int ret;

  server = (struct sockaddr_in*)kmalloc(sizeof(struct sockaddr_in), GFP_KERNEL);
  sock_create(PF_INET, SOCK_STREAM, IPPROTO_TCP, &sock);
  server->sin_family = AF_INET;
  server->sin_addr.s_addr = in_aton(ip_addr);
  server->sin_port = htons(port);

  printk(KERN_INFO "Connect to %X:%u\n", server->sin_addr.s_addr, server->sin_port);

  ret = kernel_connect(sock, (struct sockaddr*)server, sizeof(struct sockaddr_in), !O_NONBLOCK);
  if (ret < 0) {
    printk(KERN_WARNING "Error %d\n", -ret);
    return NULL;
  }

  printk(KERN_INFO "Connected\n");
  return sock;
}

int ktcp_send(struct socket* sock, char* buf, int len)
{
  struct iovec iov;
  struct msghdr msg;
  int size;
  mm_segment_t oldfs;

  printk(KERN_INFO "ktcp_send");

  if (sock == NULL) {
    printk(KERN_WARNING "sock is NULL");
    return -1;
  }

  iov.iov_base = buf;
  iov.iov_len = len;

  msg.msg_name = 0;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  oldfs = get_fs(); set_fs(KERNEL_DS);
  size = sock_sendmsg(sock, &msg, len);
  set_fs(oldfs);

  if (size < 0) {
    printk(KERN_WARNING "sock_sendmsg failed : %d", -size);
  } else {
    printk(KERN_INFO "sock_sendmsg succeed : %d", size);
  }

  return size;
}

int ktcp_recv(struct socket* sock, char* buf, int len)
{
  struct iovec iov;
  struct msghdr msg;
  mm_segment_t oldfs;
  int size;

  printk(KERN_INFO "ktcp_recv");

  if (sock == NULL) {
    printk(KERN_WARNING "sock is NULL");
    return -1;
  }

  iov.iov_base = buf;
  iov.iov_len = len;

  msg.msg_name = 0;
  msg.msg_namelen = 0;
  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = NULL;
  msg.msg_controllen = 0;
  msg.msg_flags = 0;

  oldfs = get_fs(); set_fs(KERNEL_DS);
  size = sock_recvmsg(sock, &msg, len, msg.msg_flags);
  set_fs(oldfs);

  if (size < 0) {
    printk(KERN_WARNING "sock_recvmsg failed : %d", -size);
  } else {
    printk(KERN_INFO "sock_recvmsg succeed : %d", size);
  }

  return size;
}

static int twitter_open(struct inode* inode, struct file* filp)
{
  twitter_sock = ktcp_sock_connect("127.0.0.1", 8000);
  if (twitter_sock == NULL) {
    return -1;
  }

  return 0;
}

static int twitter_release(struct inode* inode, struct file* filp)
{
  sock_release(twitter_sock);

  return 0;
}

static ssize_t twitter_read(struct file* filp, char* buf, size_t buflen, loff_t* offset)
{
  char b[1024];
  int len;

  if (*offset == 0) {
    memset(b, 0, sizeof(b));
    snprintf(b, sizeof(b), "GET / HTTP/1.1\r\n\r\n");
    ktcp_send(twitter_sock, b, strlen(b));
  }

  len = ktcp_recv(twitter_sock, buf, buflen);
  if (len < 0) {
    return 0;
  }

  *offset += len;
  return len;
}

static ssize_t twitter_write(struct file* filp, const char* buf, size_t buflen, loff_t* offset)
{
  return buflen;
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
