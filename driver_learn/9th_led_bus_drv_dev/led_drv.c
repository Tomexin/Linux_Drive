/*����/����/ע��һ��platform_driver�ṹ��*/

#include <linux/module.h>
#include <linux/version.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/irq.h>
#include <linux/gpio_keys.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/kdev_t.h>
#include <linux/poll.h>
#include <linux/sched.h>
#include <linux/device.h>

#include <asm/gpio.h>
#include <asm/uaccess.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/arch/regs-gpio.h>
#include <asm/hardware.h>
#include <asm/atomic.h>

static int major = 0;
static struct class *led_class;
static struct class_device	*led_class_dev;

static volatile unsigned long *gpio_con;
static volatile unsigned long *gpio_dat;
static int pin;

static int led_open(struct inode *inode, struct file *file)
{
	//printk("first_drv_open\n");
	/*����Ϊ���״̬*/
	*gpio_con &= ~(0x3<<(pin*2));	//�Ĵ�����Ӧλ����
	*gpio_con |= (0x1<<(pin*2));	//������Ӧλ

	return 0;
}

static ssize_t led_write(struct file *file, const char __user *buf, size_t count, loff_t * ppos)
{
	int val;
	copy_from_user(&val, buf, count);			//�ں˿ռ���û��ռ��ȡ����

	if( val == 1)
	{
		/*���*/
		*gpio_dat &= ~(1<<pin);
	}
	else
	{
		/*�ص�*/
		*gpio_dat |= (1<<pin);
	}

	//printk("first_drv_write\n");

	return 0;
}

static struct file_operations led_fops = {
	 /* ����һ���꣬�������ģ��ʱ�Զ�������__this_module���� */
	.owner  =   THIS_MODULE,   
    .open   =   led_open,        
	.write	=	led_write,
};

static int  led_probe(struct platform_device *pdev)
{
	struct resource 			*res;
	/*����platform_device����Դ����ioremap*/
	res = platform_get_resource(pdev,IORESOURCE_MEM, 0);
	gpio_con = ioremap(res->start, res->end - res->start + 1);
	gpio_dat = gpio_con + 1;

	res = platform_get_resource(pdev,IORESOURCE_IRQ, 0);
	pin = res->start;

	/*ע���ַ��豸����*/

	printk("led_remove() is called\n");

	major = register_chrdev(0, "myled", &led_fops);
	//�Զ������豸�ڵ�
	led_class = class_create((struct module *)THIS_MODULE, \
		(const char *)"myled");
	led_class_dev = class_device_create(led_class, NULL,\
		MKDEV(major, 0), NULL, "myled");/*/dev/myled*/

	printk("led_probe() is called\n");
	return 0;
}


static int __devexit led_remove(struct platform_device *pdev)
{
	/*����platform_device����Դ����iounremap*/
	iounmap(gpio_con);

	/* ж���ַ��豸����*/
	class_device_destroy(led_class, MKDEV(major, 0));
	class_destroy(led_class);
	unregister_chrdev(major, "myled");
	
	return 0;
}

struct platform_driver led_drv = {
	.probe		= led_probe,
	.remove 	= led_remove,
	.driver 	={
		.name 		= "myled",		//ƽ̨������.driver�µ�nameҪ��ƽ̨�豸�㱣��һ��
	},
};


static int led_drv_init(void)
{
	platform_driver_register(&led_drv);
	return 0;
}

static void led_drv_exit(void)
{
	platform_driver_unregister(&led_drv);
}

module_init(led_drv_init);
module_exit(led_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomxin in 15#351");

