#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>

#define BUFFER_SZ 10000
static char s_buffer[BUFFER_SZ];
static dev_t skeleton_dev;
static struct cdev skeleton_cdev;

static int skeleton_open(struct inode* i, struct file* f)
{
    pr_info("skeleton : open operation... major:%d, minor:%d\n", imajor(i), iminor(i));
    if ((f->f_mode & (FMODE_READ | FMODE_WRITE)) != 0)
        pr_info("skeleton : opened for reading & writing...\n");
    else if (f->f_mode & FMODE_READ)
        pr_info("skeleton : opened for reading...\n");
    else if (f->f_mode & FMODE_WRITE)
        pr_info("skeleton : opened for writing...\n");
    return 0;
}

static int skeleton_release(struct inode* i, struct file* f)
{
    pr_info("skeleton: release operation...\n");
    return 0;
}

static ssize_t skeleton_read(struct file* f, char __user* buf, size_t count, loff_t* off)
{
    ssize_t remaining = BUFFER_SZ - *off;
    if (count > remaining) count = remaining;
    if (copy_to_user(buf, s_buffer + *off, count)) return -EFAULT;
    *off += count;
    pr_info("skeleton: read operation... read=%ld\n", count);
    return count;
}

static ssize_t skeleton_write(struct file* f, const char __user* buf, size_t count, loff_t* off)
{
    ssize_t remaining = BUFFER_SZ - *off;
    if (count >= remaining) return -EIO;
    if (copy_from_user(s_buffer + *off, buf, count)) return -EFAULT;
    s_buffer[*off + count] = 0; // null-terminate
    *off += count;
    pr_info("skeleton: write operation... written=%ld\n", count);
    return count;
}

static struct file_operations skeleton_fops = {
    .owner = THIS_MODULE,
    .open = skeleton_open,
    .release = skeleton_release,
    .read = skeleton_read,
    .write = skeleton_write,
};

static int __init skeleton_init(void)
{
    int status = alloc_chrdev_region(&skeleton_dev, 0, 1, "mymodule");
    if (status == 0) {
        cdev_init(&skeleton_cdev, &skeleton_fops);
        skeleton_cdev.owner = THIS_MODULE;
        status = cdev_add(&skeleton_cdev, skeleton_dev, 1);
    }
    pr_info("skeleton: device loaded — major=%d, minor=%d\n",
        MAJOR(skeleton_dev), MINOR(skeleton_dev));

    return status;
}

static void __exit skeleton_exit(void)
{
    cdev_del(&skeleton_cdev);
    unregister_chrdev_region(skeleton_dev, 1);
    pr_info("Linux module skeleton unloaded\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Yoann Künti");
MODULE_DESCRIPTION ("Pilote orienté caractère");
MODULE_LICENSE ("GPL");
