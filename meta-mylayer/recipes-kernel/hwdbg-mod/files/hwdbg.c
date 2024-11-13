#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sysfs.h>
#include <linux/kobject.h>
#include <linux/uaccess.h>
#include <linux/hw_breakpoint.h>
#include <linux/perf_event.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>

struct perf_event *__percpu *w_hbp = NULL;
struct perf_event *__percpu *r_hbp = NULL;

static unsigned long target_variable = 0;

unsigned long counter = 0xdeadbeef;
unsigned long dummy = 0xacabacab;

static void test_hbp_write_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs)
{

	printk("HWDBG write target_variable = %lx ", *(unsigned long *)target_variable);
}

static void test_hbp_read_handler(struct perf_event *bp, struct perf_sample_data *data, struct pt_regs *regs)
{
	printk("HWDBG read-write target_variable = %lx ", *(unsigned long *)target_variable);
}

static ssize_t target_variable_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	// TODO: investigate why first assign doesnt triggers w_hbp
	++counter;
	dummy = counter;
	return sprintf(buf, "%lx\n", target_variable); // Format the target_variable as a hex string
}

static ssize_t target_variable_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	unsigned long value;
	if (sscanf(buf, "%lx", &value) == 1)
	{
		target_variable = value; // Update the target_variable with the new value
	}

	if (target_variable % HW_BREAKPOINT_LEN_4 != 0)
	{
		pr_err("HWDBG Error, provided adress should be alligned by 4.\n");
		return -1;
	}

	if (w_hbp)
	{
		unregister_wide_hw_breakpoint(w_hbp);
		w_hbp = NULL;
	}
	struct perf_event_attr wpe_attr;
	hw_breakpoint_init(&wpe_attr);
	wpe_attr.type = PERF_TYPE_BREAKPOINT;
	wpe_attr.size = sizeof(struct perf_event_attr);
	wpe_attr.bp_type = HW_BREAKPOINT_W;
	wpe_attr.bp_addr = (unsigned long)target_variable;
	wpe_attr.bp_len = HW_BREAKPOINT_LEN_4;
	wpe_attr.sample_period = 1;

	w_hbp = register_wide_hw_breakpoint(&wpe_attr, test_hbp_write_handler, NULL);
	if (IS_ERR((void __force *)w_hbp))
	{
		int retval = PTR_ERR((void __force *)w_hbp);
		pr_err("HWDBG Error write register_wide_hw_breakpoint:%d\n", retval);
		return -1;
	}

	if (r_hbp)
	{
		unregister_wide_hw_breakpoint(r_hbp);
		r_hbp = NULL;
	}
	struct perf_event_attr rpe_attr;
	hw_breakpoint_init(&rpe_attr);
	rpe_attr.type = PERF_TYPE_BREAKPOINT;
	rpe_attr.size = sizeof(struct perf_event_attr);
	rpe_attr.bp_type = HW_BREAKPOINT_W | HW_BREAKPOINT_R;
	rpe_attr.bp_addr = (unsigned long)target_variable;
	rpe_attr.bp_len = HW_BREAKPOINT_LEN_4;
	rpe_attr.sample_period = 1;

	r_hbp = register_wide_hw_breakpoint(&rpe_attr, test_hbp_read_handler, NULL);
	if (IS_ERR((void __force *)r_hbp))
	{
		int retval = PTR_ERR((void __force *)r_hbp);
		pr_err("HWDBG Error read register_wide_hw_breakpoint:%d\n", retval);
		return -1;
	}

	pr_info("HWDBG New Value is:0x%lx! dummy:%lx\n", target_variable, dummy);

	return count;
}

// Define the sysfs attribute for target_variable
static struct kobj_attribute target_variable_attribute = __ATTR(target_variable, 0660, target_variable_show, target_variable_store);

// Define the kobject
static struct kobject *hwdbg_kobject;

static int __init hwdbg_init(void)
{
	int retval;

	// Create a kobject under /sys/kernel/
	hwdbg_kobject = kobject_create_and_add("hwdbg", kernel_kobj);
	if (!hwdbg_kobject)
		return -ENOMEM;

	retval = sysfs_create_file(hwdbg_kobject, &target_variable_attribute.attr);
	if (retval)
		kobject_put(hwdbg_kobject);

	pr_info("HWDBG Dummy Value is:%lx:0x%px\n", dummy, &dummy);
	return 0;
}

static void __exit hwdbg_exit(void)
{
	kobject_put(hwdbg_kobject);

	if (w_hbp)
	{
		unregister_wide_hw_breakpoint(w_hbp);
		w_hbp = NULL;
	}
	if (r_hbp)
	{
		unregister_wide_hw_breakpoint(r_hbp);
		r_hbp = NULL;
	}
	pr_info("HWDBG unload!\n");
}

module_init(hwdbg_init);
module_exit(hwdbg_exit);
MODULE_LICENSE("GPL");
