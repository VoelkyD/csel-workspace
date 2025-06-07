#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/thermal.h>
#include <linux/gpio.h>
#include <linux/timer.h>
#include <linux/device.h>
#include <linux/fs.h>

#define LED_GPIO 10

static struct thermal_zone_device *thermal_zone;
static struct timer_list blink_timer;
static bool led_state = false;
static int current_delay = 250; // ms

// sysfs
static struct class *fan_class;
static struct device *fan_device;
static char mode[8] = "auto"; // "auto" or "manual"
static int manual_freq = 2;   // Default manual frequency (Hz)

// LED blinking logic
static void blink_timer_callback(struct timer_list *t)
{
    int temp_mC = 0, temp_C = 0;
    int freq = 2;

    if (strcmp(mode, "auto") == 0) {
        if (!thermal_zone || thermal_zone_get_temp(thermal_zone, &temp_mC) != 0) {
            pr_warn("Unable to read temperature\n");
        } else {
            temp_C = temp_mC / 1000;

            if (temp_C < 35)
                freq = 2;
            else if (temp_C < 40)
                freq = 5;
            else if (temp_C < 45)
                freq = 10;
            else
                freq = 20;
        }
    } else {
        freq = manual_freq;
    }

    current_delay = 1000 / (2 * freq);

    if (strcmp(mode, "auto") == 0) {
        pr_info("Mode=auto Temp=%d°C -> Freq=%d Hz, delay=%d ms\n", temp_C, freq, current_delay);
    } else {
        pr_info("Mode=manual -> Freq=%d Hz, delay=%d ms\n", freq, current_delay);
    }

    led_state = !led_state;
    gpio_set_value(LED_GPIO, led_state);

    mod_timer(&blink_timer, jiffies + msecs_to_jiffies(current_delay));
}

// sysfs: mode
static ssize_t mode_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%s\n", mode);
}

static ssize_t mode_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    if (sysfs_streq(buf, "auto") || sysfs_streq(buf, "manual")) {
        strncpy(mode, buf, sizeof(mode) - 1);
        mode[sizeof(mode) - 1] = '\0';
    }
    return count;
}

// sysfs: frequency
static ssize_t frequency_show(struct device *dev, struct device_attribute *attr, char *buf)
{
    return sprintf(buf, "%d\n", manual_freq);
}

static ssize_t frequency_store(struct device *dev, struct device_attribute *attr, const char *buf, size_t count)
{
    int val;
    if (kstrtoint(buf, 10, &val) == 0 && val > 0 && val <= 100)
        manual_freq = val;
    return count;
}

static DEVICE_ATTR_RW(mode);
static DEVICE_ATTR_RW(frequency);

static int __init miniproj_init(void)
{
    int ret;

    pr_info("Module miniproj loaded (thermal + gpio + sysfs)\n");

    thermal_zone = thermal_zone_get_zone_by_name("cpu-thermal");
    if (IS_ERR(thermal_zone)) {
        pr_err("Could not get cpu-thermal zone\n");
        return PTR_ERR(thermal_zone);
    }

    ret = gpio_request(LED_GPIO, "status_led");
    if (ret) {
        pr_err("Failed to request GPIO %d\n", LED_GPIO);
        return ret;
    }

    gpio_direction_output(LED_GPIO, 0);

    timer_setup(&blink_timer, blink_timer_callback, 0);
    mod_timer(&blink_timer, jiffies + msecs_to_jiffies(current_delay));

    // Sysfs setup
    fan_class = class_create(THIS_MODULE, "fan_control");
    if (IS_ERR(fan_class)) {
        pr_err("Failed to create class\n");
        gpio_free(LED_GPIO);
        return PTR_ERR(fan_class);
    }

    fan_device = device_create(fan_class, NULL, 0, NULL, "fan");
    if (IS_ERR(fan_device)) {
        class_destroy(fan_class);
        gpio_free(LED_GPIO);
        return PTR_ERR(fan_device);
    }

    device_create_file(fan_device, &dev_attr_mode);
    device_create_file(fan_device, &dev_attr_frequency);

    return 0;
}

static void __exit miniproj_exit(void)
{
    del_timer_sync(&blink_timer);
    gpio_set_value(LED_GPIO, 0);
    gpio_free(LED_GPIO);

    device_remove_file(fan_device, &dev_attr_mode);
    device_remove_file(fan_device, &dev_attr_frequency);
    device_destroy(fan_class, 0);
    class_destroy(fan_class);

    pr_info("Module miniproj unloaded\n");
}

module_init(miniproj_init);
module_exit(miniproj_exit);

MODULE_AUTHOR("Yoann Künti");
MODULE_DESCRIPTION("Thermal-based LED blinking via GPIO with sysfs interface");
MODULE_LICENSE("GPL");