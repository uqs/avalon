
#include <linux/module.h>
#include <linux/init.h>
#include <linux/proc_fs.h>
#include <linux/string.h>
#include <linux/ctype.h>
#include <linux/errno.h>
#include <linux/delay.h>
#include <linux/sched.h>

#include <asm/hardware.h>
#include <asm/arch/pxa-regs.h>
#include <asm/uaccess.h>

static struct proc_dir_entry *proc_sht1x_parent;

#define CLK_PHASE	10	// sleep ten microseconds per high/low cycle, 50 kHz
#define MEASUREMENT_TIME	300 * HZ / 1000
#define CONTROL_MEASURE_TEMP	0x03
#define CONTROL_MEASURE_HUM	0x05

#define WAIT()		udelay(CLK_PHASE)
#define CLOCK_HIGH	PORT_HIGH(clock_port)
#define CLOCK_LOW	PORT_LOW(clock_port)
#define DATA_HIGH	PORT_INPUT(data_port)
//#define DATA_HIGH	PORT_HIGH(data_port)
#define DATA_LOW	do{ PORT_OUTPUT(data_port); PORT_LOW(data_port); } while(0)

#define PORT_OUTPUT(p)	GPDR(p) |= GPIO_bit(p)
#define PORT_INPUT(p)	GPDR(p) &= ~GPIO_bit(p)
#define PORT_HIGH(p)	GPSR(p) = GPIO_bit(p)
#define PORT_LOW(p)	GPCR(p) = GPIO_bit(p)

#define PORT_DATA(p)	GPLR(p) & GPIO_bit(p)

static int clock_port = 113;
static int data_port  =  75;
module_param(clock_port, int, S_IRUGO);
module_param(data_port,  int, S_IRUGO);



static void send_transmission_start(void)
{
	// PRE: clock low, data high
	// POST: clock low, data high
	CLOCK_HIGH;
	WAIT();
	DATA_LOW;
	WAIT();
	CLOCK_LOW;
	WAIT();
	CLOCK_HIGH;
	WAIT();
	DATA_HIGH;
	WAIT();
	CLOCK_LOW;
	WAIT();
}
	

static int send_byte(uint8_t byte)
{
	uint8_t m;
	int ret = 0;
	for(m = 0x80; m > 0; m >>= 1) {
		if(byte & m) {
			DATA_HIGH;
		}
		else {
			DATA_LOW;
		}
		WAIT();
		CLOCK_HIGH;	// <-- sampling point
		WAIT();
		CLOCK_LOW;
	}
	PORT_INPUT(data_port);
	WAIT();
	CLOCK_HIGH;
	if(PORT_DATA(data_port)) {
		ret = -1;
	}
	WAIT();
	CLOCK_LOW;
	
	return ret;
}

static uint8_t read_byte(int acknowledge)
{
	uint8_t m, value = 0;
	PORT_INPUT(data_port);	
	for(m = 0x80; m > 0; m >>= 1) {
		WAIT();
		CLOCK_HIGH;	// <-- sampling point
		if(PORT_DATA(data_port)) {
			value |= m;
		}
		WAIT();
		CLOCK_LOW;
	}
	if(acknowledge) {
		DATA_LOW;
	}
	WAIT();
	CLOCK_HIGH;
	WAIT();
	CLOCK_LOW;
	PORT_INPUT(data_port);	

	return value;
}

static void reset_protocol(void)
{
	int i;
	PORT_OUTPUT(clock_port);
	DATA_HIGH;
	for(i = 0; i < 10; i ++) {
		WAIT();
		CLOCK_HIGH;
		WAIT();
		CLOCK_LOW;
	}
	WAIT();
}


static int proc_data_read(char *page, char **start, off_t off,
			int count, int *eof, void *data)
{
	char *p = page;
	int len;
	int value = 0;
	int af;
	int loops;
	// check for GPIO
	// set ports to GPIO
	af = (GAFR(clock_port) >> ((clock_port & 0x0f) << 0x01)) & 0x03;
	if(af != 0) {
		printk(KERN_ERR "clock port %d was reconfigured while in use!", clock_port);
		return -ENOENT;
	}
	af = (GAFR(data_port) >> ((data_port & 0x0f) << 0x01)) & 0x03;
	if(af != 0) {
		printk(KERN_ERR "data port %d was reconfigured while in use!", data_port);
		return -ENOENT;
	}

	// assume we don't need to reset the protocol
	// if the data line is high and clock line low
	// and both are outputs
	if(!PORT_DATA(data_port) || PORT_DATA(clock_port)) {
		printk(KERN_DEBUG "ports weren't in the state i left them, resetting protocol.");
		reset_protocol();
	}
	
	send_transmission_start();
	if(send_byte((uint8_t)((int)data & 0xff)) != 0) {
		printk(KERN_DEBUG "didn't receive an acknowledgement after sending command, resetting protocol.");
		reset_protocol();
		if(send_byte((uint8_t)((int)data & 0xff)) != 0) {
			printk(KERN_DEBUG "still didn't receive an acknowledgement, giving up.");
			return -ENOENT;
		}
	}
	
	set_current_state(TASK_UNINTERRUPTIBLE);
	schedule_timeout(MEASUREMENT_TIME);
	
	if(PORT_DATA(data_port)) {
		printk(KERN_WARNING "woke up to find data line still high.");
		return -ENOENT;
	}
	
	value = read_byte(1) << 8;
	value |= read_byte(0);
	printk(KERN_INFO "sht1x driver read %d", value);
	len = scnprintf(page, count, "%d\n", value);
	*eof = 1;
/*	
	p += scnprintf(p, count, "%d\n", value);

	len = (p - page) - off;

	if(len < 0)
	{
		len = 0;
	}

	*eof = (len <= count) ? 1 : 0;
	*start = page + off;
*/
	return len;
}

static int __init sht1x_init(void)
{
	int af;
	printk(KERN_INFO "sht1x driver, using clock port %d and data port %d", clock_port, data_port);
	// set ports to GPIO
	af = (GAFR(clock_port) >> ((clock_port & 0x0f) << 0x01)) & 0x03;
	if(af != 0) {
		printk(KERN_NOTICE "clock port %d is not configured as GPIO, reconfiguring...", clock_port);
		GAFR(clock_port) &= ~(0x3 << ((clock_port & 0xf) << 0x01));
	}
	af = (GAFR(data_port) >> ((data_port & 0x0f) << 0x01)) & 0x03;
	if(af != 0) {
		printk(KERN_NOTICE "data port %d is not configured as GPIO, reconfiguring...", data_port);
		GAFR(data_port) &= ~(0x3 << ((data_port & 0xf) << 0x01));
	}
	
	proc_sht1x_parent = proc_mkdir("sht1x", NULL);

	if(!proc_sht1x_parent)
		return -ENXIO;

	// read only
	create_proc_read_entry("temperature", 0444, proc_sht1x_parent, proc_data_read, (void *)CONTROL_MEASURE_TEMP);
	create_proc_read_entry("humidity", 0444, proc_sht1x_parent, proc_data_read, (void *)CONTROL_MEASURE_HUM);

	reset_protocol();
	return 0;
}

static void sht1x_exit(void)
{
	remove_proc_entry("temperature", proc_sht1x_parent);
	remove_proc_entry("humidity", proc_sht1x_parent);


	if (proc_sht1x_parent)
	{
		if (!proc_sht1x_parent->subdir)
		{
			// Only remove /proc/sht1x if it's empty.
			remove_proc_entry("sht1x", NULL );
		}
	}

}

module_init(sht1x_init);
module_exit(sht1x_exit);
MODULE_LICENSE("GPL");
