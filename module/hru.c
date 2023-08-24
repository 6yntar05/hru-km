#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

#include "hru.h"    // Module properties
#include "grunt.h"  // "Hru"nefy header -> HRUNEFY_BUFFER()

#define DEVICE_NAME "hru"

extern char* fill_with;
extern int fill_size;

/*****************************************************************/
// Available file operations:
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations file_ops = {
    .owner = THIS_MODULE,
    .read = device_read,
    .write = device_write,
    .open = device_open,
    .release = device_release
};

// Device
static int device_major_num;
static int device_open_count = 0;
dev_t device_num;
struct class* device_class;
struct device* device;

// Buffer
static char* hru_buffer;
static int runtime_msg_lenght = 0;

/*****************************************************************/
// Module implementation
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
    int ret_buffer_size;
    if (!runtime_msg_lenght)
        return 0;
    ret_buffer_size = runtime_msg_lenght;
    if (copy_to_user(buffer, hru_buffer, runtime_msg_lenght)) // Kernelspace -> Userspace
        return -EFAULT;
    runtime_msg_lenght = 0; // Buffer read clears it
    return ret_buffer_size;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
    if (len >= buffer_size) // Buffer overflow ._.
        return -ENOSPC;
    if (copy_from_user(hru_buffer, buffer, len)) // Userspace -> Kernelspace
        return -EFAULT;
    runtime_msg_lenght = len;
    if (hru_buffer[runtime_msg_lenght] == 10)
        runtime_msg_lenght--;
    HRUNEFY_BUFFER(hru_buffer, &runtime_msg_lenght); // Can change buffer lenght because utf8
    return len;
}

static int device_open(struct inode *inode, struct file *file) {
    if (device_open_count)
        return -EBUSY;
    device_open_count++;
    try_module_get(THIS_MODULE);
    return 0;
}

static int device_release(struct inode *inode, struct file *file) {
    device_open_count--;
    module_put(THIS_MODULE);
    return 0;
}

static int __init hru_init(void) {
    // String len from module arg.
    fill_size = walk_for_end(fill_with);

    // Allocate buffer and write instruction
    hru_buffer = (char*)kzalloc(sizeof(char) * buffer_size, GFP_KERNEL);
    if (unlikely(!hru_buffer)) {
        printk(KERN_ALERT "hru: Could not allocate hru buffer with size: %d :(\n", buffer_size);
        return -1;
    }
    
    // Check for buffer overflow
    if (buffer_size > 55) {
        runtime_msg_lenght = 55;
        strncpy(hru_buffer, "Hey, just write ur message to this file before read it\0", runtime_msg_lenght);
    }
    
    // Init device
    device_major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if (device_major_num < 0) {
        printk(KERN_ALERT "hru: Could not register device: %d :(\n", device_major_num);
        return device_major_num;
    } else {
        printk(KERN_INFO "hru: Module loaded:\n");
    }

    // Make character device in /dev
    device_num = MKDEV(device_major_num, 0);  // Create a 32 bit version of numbers

        // Create /sys/class/hru in preparation of creating /dev/hru
    device_class = class_create(DEVICE_NAME);
    if (IS_ERR(device_class)) {
        printk(KERN_WARNING "hru: Cannot create device class :(\n");
        unregister_chrdev_region(device_num, 1);
        return -1;
    }

        // Create /dev/hru for this char dev
    if (IS_ERR(device = device_create(device_class, NULL, device_num, NULL, DEVICE_NAME))) {
        printk(KERN_WARNING "hru: Failet to create device :(\n");
        class_destroy(device_class);
        unregister_chrdev_region(device_num, 1);
        return -1;
    }

    return 0;
}

static void __exit hru_exit(void) {
    kfree(hru_buffer);

    device_destroy(device_class, device_num);
    class_destroy(device_class);
    unregister_chrdev(device_major_num, DEVICE_NAME);
  
    printk(KERN_INFO "hru: Unloaded\n");
}

module_init(hru_init);
module_exit(hru_exit);
