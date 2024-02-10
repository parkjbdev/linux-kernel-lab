/*
 * Character device drivers lab
 *
 * All tasks
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/kdev_t.h>

#include "../include/so2_cdev.h"

MODULE_DESCRIPTION("SO2 character device");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");
MODULE_INFO(intree, "Y");

#define LOG_LEVEL KERN_INFO

#define MY_MAJOR 42
#define MY_MINOR 0
#define NUM_MINORS 1
#define MODULE_NAME "so2_cdev"
#define MESSAGE "hello\n"
#define IOCTL_MESSAGE "Hello ioctl"

#ifndef BUFSIZ
#define BUFSIZ 4096
#endif

void simulate_time(size_t seconds)
{
	/* Simulate long term operation for testing TODO 3 */
	pr_info("Sleeping for 10 seconds\n");
	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(seconds * HZ);
}

dev_t *dev;
static struct class *cls;

struct so2_device_data {
	/* TODO 2: add cdev member */
	struct cdev cdev;
	/* TODO 4: add buffer with BUFSIZ elements */
	char buffer[BUFSIZ];
	/* TODO 7: extra members for home */
	/* TODO 3: add atomic_t access variable to keep track if file is opened */
	atomic_t access;
};

struct so2_device_data devs[NUM_MINORS];

static int so2_cdev_open(struct inode *inode, struct file *file)
{
	struct so2_device_data *data;

	/* TODO 2: print message when the device file is open. */
	pr_info("Device %s with major number %d and minor number %d was opened\n",
		MODULE_NAME, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	/* TODO 3: inode->i_cdev contains our cdev struct, use container_of to obtain a pointer to so2_device_data */
	data = container_of(inode->i_cdev, struct so2_device_data, cdev);
	file->private_data = data;

#ifndef EXTRA
	/* TODO 3: return immediately if access is != 0, use atomic_cmpxchg */
	if (atomic_cmpxchg(&data->access, 0, 0) != 0) {
		return -EBUSY;
	}
	atomic_inc(&data->access);
#endif

	return 0;
}

static int so2_cdev_release(struct inode *inode, struct file *file)
{
	/* TODO 2: print message when the device file is closed. */
	pr_info("Device %s with major number %d and minor number %d was closed\n",
		MODULE_NAME, MAJOR(inode->i_rdev), MINOR(inode->i_rdev));

	struct so2_device_data *data = file->private_data;

#ifndef EXTRA
	/* TODO 3: reset access variable to 0, use atomic_set */
	/* maybe this is better */
	atomic_dec(&data->access);
#endif
	return 0;
}

static ssize_t so2_cdev_read(struct file *file, char __user *user_buffer,
			     size_t size, loff_t *offset)
{
	pr_info("device_read called. *offset: %lld, size: %zu", *offset, size);

	struct so2_device_data *data = file->private_data;
	size_t to_read = (*offset + size > BUFSIZ) ? BUFSIZ - *offset : size;

	pr_debug("to_read: %zu", to_read);

#ifdef EXTRA
	/* TODO 7: extra tasks for home */
#endif

	/* TODO 4: Copy data->buffer to user_buffer, use copy_to_user */
	int fail_to_copy = 0;
	if ((fail_to_copy = copy_to_user(user_buffer, data->buffer + *offset,
					 to_read))) {
		pr_err("copy_to_user: failed to copy %d\n", fail_to_copy);

		return -EFAULT;
	}

	*offset += to_read;

	return to_read;
}

static ssize_t so2_cdev_write(struct file *file, const char __user *user_buffer,
			      size_t size, loff_t *offset)
{
	pr_info("device_write called. *offset: %lld, size: %zu", *offset, size);

	struct so2_device_data *data = file->private_data;

	/* TODO 5: copy user_buffer to data->buffer, use copy_from_user */
	size_t to_copy = copy_from_user(data->buffer, user_buffer, size);
	if (to_copy != 0) {
		return -EFAULT;
	}
	*offset += size;
	/* TODO 7: extra tasks for home */

	return size;
}

static long so2_cdev_ioctl(struct file *file, unsigned int cmd,
			   unsigned long arg)
{
	struct so2_device_data *data =
		(struct so2_device_data *)file->private_data;
	int ret = 0;
	int remains;

	switch (cmd) {
	/* TODO 6: if cmd = MY_IOCTL_PRINT, display IOCTL_MESSAGE */
	/* TODO 7: extra tasks, for home */
	default:
		ret = -EINVAL;
	}

	return ret;
}

static const struct file_operations so2_fops = {
	.owner = THIS_MODULE,
	/* TODO 2: add open and release functions */
	.open = so2_cdev_open,
	.release = so2_cdev_release,
	/* TODO 4: add read function */
	.read = so2_cdev_read,
	/* TODO 5: add write function */
	.write = so2_cdev_write,
	/* TODO 6: add ioctl function */
};

static int so2_cdev_init(void)
{
	int err;

	/* TODO 1: register char device region for MY_MAJOR and NUM_MINORS starting at MY_MINOR */

	/* Static Allocation */
	dev = kmalloc(sizeof(dev_t), GFP_KERNEL);
	*dev = MKDEV(MY_MAJOR, MY_MINOR);
	// dev_t dev = MKDEV(MY_MAJOR, MY_MINOR);
	err = register_chrdev_region(*dev, NUM_MINORS, MODULE_NAME);
	if (err != 0) {
		pr_err("register_chrdev_region failed. Returned %d\n", err);
		return err;
	}

	/* Dynamic Allocation */
	// dev = kmalloc(sizeof(dev_t), GFP_KERNEL);
	// err = alloc_chrdev_region(dev, MY_MINOR, NUM_MINORS, MODULE_NAME);

	// if (err != 0) {
	// 	pr_err("alloc_chrdev_region failed. Returned %d", err);
	// 	return err;
	// } else {
	// 	pr_info("alloc_chrdev_region: major number %d, minor number %d\n",
	// 		MAJOR(*dev), MINOR(*dev));
	// }

	for (int i = 0; i < NUM_MINORS; i++) {
#ifdef EXTRA
		/* TODO 7: extra tasks, for home */
#else
		/* TODO 4: initialize buffer with MESSAGE string */
		strcpy(devs[i].buffer, MESSAGE);

		/* TODO 3: set access variable to 0, use atomic_set */
		atomic_set(&devs[i].access, 0);
#endif
		/* TODO 7: extra tasks for home */
		/* TODO 2: init and add cdev to kernel core */
		cdev_init(&devs[i].cdev, &so2_fops);
		pr_info("cdev_init device %s with major number %d and minor number %d\n",
			MODULE_NAME, MAJOR(*dev), i);

		cdev_add(&devs[i].cdev, MKDEV(MY_MAJOR, i), NUM_MINORS);
		pr_info("cdev_add device %s with major number %d and minor number %d\n",
			MODULE_NAME, MAJOR(*dev), i);
	}

	// Automatically Create Device
	cls = class_create(MODULE_NAME);
	if (IS_ERR(cls)) {
		pr_err("class_create failed. Returned %ld", PTR_ERR(cls));
		return PTR_ERR(cls);
	} else {
		pr_info("class_create: major number %d, minor number %d\n",
			MAJOR(*dev), MINOR(*dev));
	}

	struct device *device =
		device_create(cls, NULL, *dev, NULL, MODULE_NAME);
	if (IS_ERR(device)) {
		pr_err("device_create failed. Returned %ld", PTR_ERR(device));
		return PTR_ERR(device);
	} else {
		pr_info("device_create: major number %d, minor number %d\n",
			MAJOR(*dev), MINOR(*dev));
	}

	return 0;
}

static void so2_cdev_exit(void)
{
	int i;

	for (i = 0; i < NUM_MINORS; i++) {
		/* TODO 2: delete cdev from kernel core */
		cdev_del(&devs[i].cdev);
		pr_info("cdev_del device %s with major number %d and minor number %d\n",
			MODULE_NAME, MAJOR(*dev), i);
	}

	/* TODO 1: unregister char device region, for MY_MAJOR and NUM_MINORS starting at MY_MINOR */
	unregister_chrdev_region(*dev, NUM_MINORS);
	device_destroy(cls, *dev);
	class_destroy(cls);
	pr_info("Unregistered device %s with major number %d and minor numbers %d to %d\n",
		MODULE_NAME, MAJOR(*dev), MY_MINOR, MY_MINOR + NUM_MINORS - 1);
	kfree(dev);
}

module_init(so2_cdev_init);
module_exit(so2_cdev_exit);
