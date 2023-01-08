#pragma once
#include <linux/module.h>
#include <linux/moduleparam.h>

// Module
MODULE_LICENSE("GPL");
MODULE_AUTHOR("6yntar05");
MODULE_DESCRIPTION("The hru kernel module inspired by ~begs/hru");
MODULE_VERSION("0.0");

// Params
int buffer_size = 2048;
char* fill_with = "hru\0";

module_param(buffer_size, int, S_IRUGO);
module_param(fill_with, charp, S_IRUGO);
