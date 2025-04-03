#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/device.h>
#include <linux/string.h>

#define CLASS_NAME "my_sysfs_class"

static struct class *sysfs_class;
static struct device *sysfs_device;

static char sysfs_buf[100] = "Hello\n";

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

static void sysfs_dev_release(struct device *dev) {}

static struct platform_device sysfs_pdev = {
    .name = "my_sysfs_dev",
    .id = -1,
    .dev.release = sysfs_dev_release,
};

static int __init skeleton_init(void)
{
    int status = 0;

    // Créer la classe visible dans /sys/class/
    sysfs_class = class_create(THIS_MODULE, CLASS_NAME);
    if (IS_ERR(sysfs_class))
        return PTR_ERR(sysfs_class);

    // Enregistrer un "fake" device associé à cette classe
    status = platform_device_register(&sysfs_pdev);
    if (status)
        goto err_class;

    // Créer un device sysfs pour rendre visible l’attribut
    sysfs_device = device_create(sysfs_class, NULL,
                                 MKDEV(0, 0), NULL, "my_sysfs_dev");
    if (IS_ERR(sysfs_device)) {
        status = PTR_ERR(sysfs_device);
        goto err_device;
    }

    // Ajouter un attribut
    status = device_create_file(sysfs_device, &dev_attr_attr);
    if (status)
        goto err_attr;

    pr_info("SysFS module loaded: /sys/class/%s/my_sysfs_dev/attr\n", CLASS_NAME);
    return 0;

err_attr:
    device_destroy(sysfs_class, MKDEV(0, 0));
err_device:
    platform_device_unregister(&sysfs_pdev);
err_class:
    class_destroy(sysfs_class);
    return status;
}

static void __exit skeleton_exit(void)
{
    device_remove_file(sysfs_device, &dev_attr_attr);
    device_destroy(sysfs_class, MKDEV(0, 0));
    platform_device_unregister(&sysfs_pdev);
    class_destroy(sysfs_class);
    pr_info("SysFS module unloaded.\n");
}

module_init(skeleton_init);
module_exit(skeleton_exit);

MODULE_AUTHOR ("Yoann Künti");
MODULE_DESCRIPTION ("sysfs");
MODULE_LICENSE ("GPL");
