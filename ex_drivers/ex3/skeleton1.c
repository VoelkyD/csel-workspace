#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/cdev.h>
#include <linux/slab.h>      // pour kmalloc/kfree
#include <linux/moduleparam.h>

#define BUFFER_SZ 10000
#define DEVICE_NAME "mymodule"

static int nb_devices = 1;
module_param(nb_devices, int, 0);
MODULE_PARM_DESC(nb_devices, "Nombre d'instances du périphérique");

struct skeleton_device {
    char buffer[BUFFER_SZ];
    struct cdev cdev;
};

static struct skeleton_device *devices;
static dev_t base_dev;

static int skeleton_open(struct inode *i, struct file *f)
{
    struct skeleton_device *dev = container_of(i->i_cdev, struct skeleton_device, cdev);
    f->private_data = dev;
    pr_info("skeleton: open major=%d minor=%d\n", imajor(i), iminor(i));
    return 0;
}

static int skeleton_release(struct inode *i, struct file *f)
{
    pr_info("skeleton: release\n");
    return 0;
}

static ssize_t skeleton_read(struct file *f, char __user *buf, size_t count, loff_t *off)
{
    struct skeleton_device *dev = f->private_data;
    ssize_t remaining = BUFFER_SZ - *off;
    if (count > remaining) count = remaining;
    if (copy_to_user(buf, dev->buffer + *off, count)) return -EFAULT;
    *off += count;
    pr_info("skeleton: read %ld bytes\n", count);
    return count;
}

static ssize_t skeleton_write(struct file *f, const char __user *buf, size_t count, loff_t *off)
{
    struct skeleton_device *dev = f->private_data;
    ssize_t remaining = BUFFER_SZ - *off;
    if (count >= remaining) return -EIO;
    if (copy_from_user(dev->buffer + *off, buf, count)) return -EFAULT;
    dev->buffer[*off + count] = 0;
    *off += count;
    pr_info("skeleton: wrote %ld bytes\n", count);
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
    int status;
    int i;

    status = alloc_chrdev_region(&base_dev, 0, nb_devices, DEVICE_NAME);
    if (status) return status;

    devices = kzalloc(nb_devices * sizeof(struct skeleton_device), GFP_KERNEL);
    if (!devices) {
        unregister_chrdev_region(base_dev, nb_devices);
        return -ENOMEM;
    }

    for (i = 0; i < nb_devices; i++) {
        cdev_init(&devices[i].cdev, &skeleton_fops);
        devices[i].cdev.owner = THIS_MODULE;
        status = cdev_add(&devices[i].cdev, base_dev + i, 1);
        if (status) {
            pr_err("skeleton: failed to add cdev %d\n", i);
            // rollback
            while (--i >= 0) cdev_del(&devices[i].cdev);
            kfree(devices);
            unregister_chrdev_region(base_dev, nb_devices);
            return status;
        }
        pr_info("skeleton: device /dev/%s%d created (major=%d minor=%d)\n",
            DEVICE_NAME, i, MAJOR(base_dev), MINOR(base_dev) + i);
    }

    return 0;
}

static void __exit skeleton_exit(void)
{
    int i;
    for (i = 0; i < nb_devices; i++) {
        cdev_del(&devices[i].cdev);
    }
    kfree(devices);
    unregister_chrdev_region(base_dev, nb_devices);
    pr_info("skeleton: unloaded %d devices\n", nb_devices);
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Yoann Künti");
MODULE_DESCRIPTION ("Pilote orienté caractère");
MODULE_LICENSE ("GPL");
