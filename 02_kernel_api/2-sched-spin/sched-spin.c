/*
 * Kernel API lab
 *
 * sched-spin.c: Sleeping in atomic context
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/spinlock.h>

MODULE_DESCRIPTION("Sleep while atomic");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");
MODULE_INFO(intree, "Y");

static int sched_spin_init(void)
{
  printk(KERN_INFO "Loading Module\n");

  DEFINE_SPINLOCK(lock);
  // alternative
	// spinlock_t lock;
	// spin_lock_init(&lock);

	/* TODO 0: Use spin_lock to acquire the lock */
  printk(KERN_INFO "Acquiring lock\n");
      spin_lock(&lock);

	set_current_state(TASK_INTERRUPTIBLE);
	/* Try to sleep for 5 seconds. */
  // The code that runs in the critical region protected by a spinlock is not allowed to suspend the current process
  // BUG: scheduling while atomic: insmod/383/0x00000002
	// schedule_timeout(5 * HZ);

	/* TODO 0: Use spin_unlock to release the lock */
  printk(KERN_INFO "Releasing lock\n");
  spin_unlock(&lock);

	return 0;
}

static void sched_spin_exit(void)
{
  printk(KERN_INFO "Removing Module\n");
}

module_init(sched_spin_init);
module_exit(sched_spin_exit);
