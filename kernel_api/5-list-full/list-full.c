/*
 * Kernel API lab
 * 
 * list-full.c: Working with lists (advanced)
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/list.h>
#include <linux/sched/signal.h>

MODULE_DESCRIPTION("Full list processing");
MODULE_AUTHOR("SO2");
MODULE_LICENSE("GPL");
MODULE_INFO(intree, "Y");

struct task_info {
	pid_t pid;
	unsigned long timestamp;
	atomic_t count;
	struct list_head list;
};

static struct list_head head;

static struct task_info *task_info_alloc(int pid)
{
	struct task_info *ti;

	ti = kmalloc(sizeof(*ti), GFP_KERNEL);
	if (ti == NULL)
		return NULL;
	ti->pid = pid;
	ti->timestamp = jiffies;
	atomic_set(&ti->count, 0);

	return ti;
}

static struct task_info *task_info_find_pid(int pid)
{
	struct list_head *p;
	struct task_info *ti;

	/* TODO 1: Look for pid and return task_info or NULL if not found */
	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		if (ti->pid == pid) {
			return ti;
		}
	}

	return NULL;
}

static void task_info_add_to_list(int pid)
{
	struct task_info *ti;

	ti = task_info_find_pid(pid);
	if (ti != NULL) {
		ti->timestamp = jiffies;
		atomic_inc(&ti->count);
		return;
	}

	ti = task_info_alloc(pid);
	list_add(&ti->list, &head);
}

static void task_info_add_for_current(void)
{
	task_info_add_to_list(current->pid);
	task_info_add_to_list(current->parent->pid);
	task_info_add_to_list(next_task(current)->pid);
	task_info_add_to_list(next_task(next_task(current))->pid);
}

static void task_info_print_list(const char *msg)
{
	struct list_head *p;
	struct task_info *ti;

	pr_info("%s: [ ", msg);
	list_for_each(p, &head) {
		ti = list_entry(p, struct task_info, list);
		pr_info("(%d, %lu) ", ti->pid, ti->timestamp);
	}
	pr_info("]\n");
}

#define EXPIRATION_TIMEOUT (3 * HZ)
#define EXPIRATION_COUNT 5
#define IS_EXPIRED(ti)                                       \
	((jiffies - (ti)->timestamp > EXPIRATION_TIMEOUT) && \
	 atomic_read(&(ti)->count) < EXPIRATION_COUNT)

static void task_info_remove_expired(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		// Expiration Condition
		if (IS_EXPIRED(ti)) {
			list_del(p);
			kfree(ti);
		}
	}
}

static void task_info_purge_list(void)
{
	struct list_head *p, *q;
	struct task_info *ti;

	list_for_each_safe(p, q, &head) {
		ti = list_entry(p, struct task_info, list);
		list_del(p);
		kfree(ti);
	}
}

static int list_full_init(void)
{
	INIT_LIST_HEAD(&head);

	task_info_add_for_current();
	task_info_print_list("after first add");

	set_current_state(TASK_INTERRUPTIBLE);
	schedule_timeout(5 * HZ);

	return 0;
}

static void list_full_exit(void)
{
	struct task_info *ti;

	/* TODO 2: Ensure that at least one task is not deleted */
	if (!list_empty(&head)) {
		ti = list_entry(head.next, struct task_info, list);
		if (IS_EXPIRED(ti)) {
			atomic_set(&ti->count, EXPIRATION_COUNT);
		}
	}

	task_info_remove_expired();
	task_info_print_list("after removing expired");
	task_info_purge_list();
}

module_init(list_full_init);
module_exit(list_full_exit);
