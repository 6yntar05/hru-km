#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("6yntar05");
MODULE_DESCRIPTION("The hru kernel module inspired by ~begs/hru");
MODULE_VERSION("0");

#define DEVICE_NAME "hru"

/*****************************************************************/
// Available file operations:
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations file_ops = {
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
#define BUFFER_SIZE 2048
static char* hru_buffer;
static int runtime_msg_lenght = 0;
void HRUNEFY_BUFFER(char* buffer, int len);

/*****************************************************************/
// Module implementation
static ssize_t device_read(struct file *flip, char *buffer, size_t len, loff_t *offset) {
    int ret_buffer_size;
    if (!runtime_msg_lenght)
        return 0;
    ret_buffer_size = runtime_msg_lenght;
    if (copy_to_user(buffer, hru_buffer, runtime_msg_lenght))
        return -EFAULT;
    runtime_msg_lenght = 0;
    return ret_buffer_size;
}

static ssize_t device_write(struct file *flip, const char *buffer, size_t len, loff_t *offset) {
    if (len >= BUFFER_SIZE)
        return -ENOSPC;
    if (copy_from_user(hru_buffer, buffer, len))
        return -EFAULT;
    runtime_msg_lenght = len;
    if (hru_buffer[runtime_msg_lenght] == '\n')
        runtime_msg_lenght--;
    HRUNEFY_BUFFER(hru_buffer, runtime_msg_lenght);
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
    // Allocate buffer and write instruction
    hru_buffer = (char*)kzalloc(sizeof(char) * BUFFER_SIZE, GFP_KERNEL);
    if (unlikely(!hru_buffer)) {
        printk(KERN_ALERT "hru: Could not allocate hru buffer with size: %d :(\n", BUFFER_SIZE);
        return -1;
    }
    runtime_msg_lenght = 54; // size without null-terminator; why do we need this ;p
    strncpy(hru_buffer, "Hey, just write ur message to this file before read it\0", runtime_msg_lenght);

    // Init device
    device_major_num = register_chrdev(0, DEVICE_NAME, &file_ops);
    if (device_major_num < 0) {
        printk(KERN_ALERT "hru: Could not register device: %d :(\n", device_major_num);
        return device_major_num;
    } else {
        printk(KERN_INFO "hru: Module loaded\n");
    }

    //* Make character device in /dev *//
    device_num = MKDEV(device_major_num, 0);  // Create a 32 bit version of numbers

        // Create /sys/class/hru in preparation of creating /dev/hru
    device_class = class_create(THIS_MODULE, DEVICE_NAME);
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

/*****************************************************************/
// HRUNEFICATOR
#define SPACE_CHAR ' '
void HRUNEFY_BUFFER(char* buffer, int len) {
    int local_counter = 0;
    for (int i = 0; i < len; i++) {
        if (buffer[i] == SPACE_CHAR) {
            local_counter = 0;
        } else {
            switch (local_counter) {
                case 0:
                    buffer[i] = 'h';
                    break;
                case 1:
                    buffer[i] = 'r';
                    break;
                default:
                    buffer[i] = 'u';
                    break;
            };
            local_counter++;
        }
    }
    return;
};