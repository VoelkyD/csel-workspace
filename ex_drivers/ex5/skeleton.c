#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/miscdevice.h>
#include <linux/device.h>
#include <linux/string.h>

#define BUFFER_SZ 1024

static char dev_buffer[BUFFER_SZ] = "World\n";
static char sysfs_buffer[BUFFER_SZ] = "Hello\n";

// ───── Fichier /dev/mymodule : opérations caractère ─────

static ssize_t my_read(struct file *f, char __user *buf, size_t count, loff_t *off)
{
    size_t len = strlen(dev_buffer);
    if (*off >= len)
        return 0;
    if (count > len - *off)
        count = len - *off;

    if (copy_to_user(buf, dev_buffer + *off, count))
        return -EFAULT;

    *off += count;
    pr_info("mymodule: read %zu bytes\n", count);
    return count;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t count, loff_t *off)
{
    size_t len = min(count, (size_t)(BUFFER_SZ - 1));
    if (copy_from_user(dev_buffer, buf, len))
        return -EFAULT;

    dev_buffer[len] = '\0';
    pr_info("mymodule: wrote %zu bytes: \"%s\"\n", len, dev_buffer);
    return len;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read = my_read,
    .write = my_write,
    .llseek = default_llseek,
};

// ───── Attribut sysfs : attr ─────

static ssize_t sysfs_show(struct device *dev,
                          struct device_attribute *attr,
                          char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s", sysfs_buffer);
}

static ssize_t sysfs_store(struct device *dev,
                           struct device_attribute *attr,
                           const char *buf,
                           size_t count)
{
    size_t len = min(count, sizeof(sysfs_buffer) - 1);
    strncpy(sysfs_buffer, buf, len);
    sysfs_buffer[len] = '\0';
    return count;
}

static DEVICE_ATTR(attr, 0664, sysfs_show, sysfs_store);

// ───── miscdevice ─────

static struct miscdevice my_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mymodule",
    .fops = &my_fops,
    .mode = 0666
};

// ───── Init / Exit ─────

static int __init mymodule_init(void)
{
    int status;

    status = misc_register(&my_miscdev);
    if (status) {
        pr_err("mymodule: failed to register miscdevice\n");
        return status;
    }

    status = device_create_file(my_miscdev.this_device, &dev_attr_attr);
    if (status) {
        misc_deregister(&my_miscdev);
        return status;
    }

    pr_info("mymodule: loaded — /dev/mymodule and sysfs attr ready\n");
    return 0;
}

static void __exit mymodule_exit(void)
{
    device_remove_file(my_miscdev.this_device, &dev_attr_attr);
    misc_deregister(&my_miscdev);
    pr_info("mymodule: unloaded\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yoann Künti");
MODULE_DESCRIPTION("sysfs");
