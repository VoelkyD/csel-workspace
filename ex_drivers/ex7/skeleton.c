#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/poll.h>
#include <linux/miscdevice.h>

#define GPIO_K1     0
#define GPIO_LABEL  "gpio_a.0-k1"
#define DEVICE_NAME "irq_block"

static DECLARE_WAIT_QUEUE_HEAD(wq);
static int irq_flag = 0;

static irqreturn_t gpio_isr(int irq, void *dev_id)
{
    pr_info("irq_block: interrupt detected!\n");
    irq_flag = 1;
    wake_up_interruptible(&wq);
    return IRQ_HANDLED;
}

// ───── File operations ─────

static ssize_t irq_read(struct file *file, char __user *buf, size_t count, loff_t *ppos)
{
    wait_event_interruptible(wq, irq_flag != 0);
    irq_flag = 0; // Reset flag after waking
    return 0;
}

static unsigned int irq_poll(struct file *file, poll_table *wait)
{
    poll_wait(file, &wq, wait);
    if (irq_flag != 0)
        return POLLIN | POLLRDNORM;
    return 0;
}

static const struct file_operations irq_fops = {
    .owner = THIS_MODULE,
    .read  = irq_read,
    .poll  = irq_poll,
};

static struct miscdevice irq_miscdev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name  = DEVICE_NAME,
    .fops  = &irq_fops,
    .mode  = 0666
};

// ───── Init / Exit ─────

static int __init irq_block_init(void)
{
    int status;

    status = gpio_request(GPIO_K1, "k1");
    if (status) return status;

    status = request_irq(gpio_to_irq(GPIO_K1), gpio_isr,
                         IRQF_TRIGGER_FALLING | IRQF_SHARED,
                         GPIO_LABEL, GPIO_LABEL);
    if (status) {
        gpio_free(GPIO_K1);
        return status;
    }

    status = misc_register(&irq_miscdev);
    if (status) {
        free_irq(gpio_to_irq(GPIO_K1), GPIO_LABEL);
        gpio_free(GPIO_K1);
        return status;
    }

    pr_info("irq_block: loaded (/dev/%s)\n", DEVICE_NAME);
    return 0;
}

static void __exit irq_block_exit(void)
{
    misc_deregister(&irq_miscdev);
    free_irq(gpio_to_irq(GPIO_K1), GPIO_LABEL);
    gpio_free(GPIO_K1);
    pr_info("irq_block: unloaded\n");
}

module_init(irq_block_init);
module_exit(irq_block_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Yoann Künti");
MODULE_DESCRIPTION("Opérations bloquantes");
