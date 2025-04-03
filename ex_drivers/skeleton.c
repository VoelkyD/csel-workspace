#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/uaccess.h>
#include <linux/device.h>
#include <linux/miscdevice.h>
#include <linux/string.h>

#define DEVICE_NAME "mymodule"
#define CLASS_NAME  "my_sysfs_class"
#define BUFFER_SZ   1024

static char kernel_buffer[BUFFER_SZ] = "Texte initial\n";
static char sysfs_buf[BUFFER_SZ]     = "SysFS par défaut\n";

static struct class *my_class;

// ───── Partie: read / write sur /dev/mymodule ─────

static ssize_t my_read(struct file *f, char __user *buf, size_t count, loff_t *off)
{
    size_t len = strlen(kernel_buffer);
    if (*off >= len) return 0;
    if (count > len - *off) count = len - *off;

    if (copy_to_user(buf, kernel_buffer + *off, count))
        return -EFAULT;

    *off += count;
    pr_info("my_read: %zu bytes\n", count);
    return count;
}

static ssize_t my_write(struct file *f, const char __user *buf, size_t count, loff_t *off)
{
    size_t len = min(count, (size_t)(BUFFER_SZ - 1));
    if (copy_from_user(kernel_buffer, buf, len))
        return -EFAULT;

    kernel_buffer[len] = '\0';
    pr_info("my_write: %zu bytes written: '%s'\n", len, kernel_buffer);
    return len;
}

static struct file_operations my_fops = {
    .owner = THIS_MODULE,
    .read  = my_read,
    .write = my_write,
    .llseek = default_llseek,
};

static struct miscdevice my_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &my_fops,
    .mode  = 0666,
};

// ───── Partie: SysFS ─────

static ssize_t sysfs_show_attr(struct device *dev,
                               struct device_attribute *attr,
                               char *buf)
{
    return scnprintf(buf, PAGE_SIZE, "%s", sysfs_buf);
}

static ssize_t sysfs_store_attr(struct device *dev,
                                struct device_attribute *attr,
                                const char *buf,
                                size_t count)
{
    size_t len = min(count, sizeof(sysfs_buf) - 1);
    strncpy(sysfs_buf, buf, len);
    sysfs_buf[len] = '\0';
    return count;
}

static DEVICE_ATTR(attr, 0664, sysfs_show_attr, sysfs_store_attr);

// ───── Init / Exit ─────

static int __init mymodule_init(void)
{
    int status;

    // Créer la classe dans /sys/class/
    my_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(my_class))
        return PTR_ERR(my_class);

    // Enregistrer le périphérique miscdevice (créera /dev/mymodule)
    status = misc_register(&my_miscdev);
    if (status)
        goto err_class;

    // Attacher le fichier sysfs au device créé
    status = device_create_file(my_miscdev.this_device, &dev_attr_attr);
    if (status)
        goto err_misc;

    pr_info("Module chargé. /dev/%s et /sys/class/%s/%s\n",
            DEVICE_NAME, CLASS_NAME, DEVICE_NAME);
    return 0;

err_misc:
    misc_deregister(&my_miscdev);
err_class:
    class_destroy(my_class);
    return status;
}

static void __exit mymodule_exit(void)
{
    device_remove_file(my_miscdev.this_device, &dev_attr_attr);
    misc_deregister(&my_miscdev);
    class_destroy(my_class);
    pr_info("Module déchargé\n");
}

module_init(mymodule_init);
module_exit(mymodule_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yoann Künti");
MODULE_DESCRIPTION("sysfs");
