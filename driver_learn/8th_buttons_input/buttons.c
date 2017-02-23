
/* �ο�linux-2.6.22.6\drivers\input\keyboard\Gpio_keys.c */

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


struct pin_desc{
	int irq;
	char *name;
	unsigned int pin;
	unsigned int key_val;
};

struct pin_desc	pins_desc[4]={
	{IRQ_EINT0,  "S2", S3C2410_GPF0,  KEY_L},
	{IRQ_EINT2,  "S3", S3C2410_GPF2,  KEY_S},
	{IRQ_EINT11, "S4", S3C2410_GPG3,  KEY_ENTER},
	{IRQ_EINT19, "S5", S3C2410_GPG11, KEY_LEFTSHIFT},
};

static struct input_dev *buttons_dev;
static struct pin_desc *irq_pd;
static struct timer_list buttons_timer;

static irqreturn_t buttons_irp(int irq, void *dev_id)
{
	irq_pd = (struct pin_desc *)dev_id;
	/*�趨��ʱ����ʱʱ��Ϊ10����*/
	mod_timer(&buttons_timer, jiffies+HZ/100);			//�޸Ķ�ʱ����ʱʱ��, 1HZΪ1��ĵ���������1�����100Ϊ10����
	return IRQ_RETVAL(IRQ_HANDLED);
}

//��ʱ��������
static void buttons_timer_function(unsigned long data)
{
	unsigned int pinval;
	struct pin_desc *pindesc = irq_pd;
	if(!pindesc)
		return;
	
	/*��ȡpinֵ*/
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	if(pinval)
	{
		/*�ɿ������һ��������0��ʾ�ɿ���1��ʾ����*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 0);
		input_sync(buttons_dev);
	}
	else	
	{
		/*����: ���һ��������0��ʾ�ɿ���1��ʾ����*/
		input_event(buttons_dev, EV_KEY, pindesc->key_val, 1);
		input_sync(buttons_dev);
	}
}



static int buttons_init(void)
{
	int i;
	int request_irq_return_value[4];
	/*1.����һ��input_dev�ṹ��*/
	buttons_dev = input_allocate_device();
	
	/*2������*/
		/*2.1 ���ÿ��Բ�����һ���¼� */
	set_bit(EV_KEY, buttons_dev->evbit);	//���ò����������¼�
	set_bit(EV_REP, buttons_dev->evbit);
		/*2.2 �����ܲ��������������Щ�¼�: L, S,  ENTER,  LEFTSHIFT*/
	set_bit(KEY_L, buttons_dev->keybit);
	set_bit(KEY_S, buttons_dev->keybit);
	set_bit(KEY_ENTER, buttons_dev->keybit);
	set_bit(KEY_LEFTSHIFT, buttons_dev->keybit);
	
	/*3��ע��*/
	input_register_device(buttons_dev);

	/*4��Ӳ����ز���*/
	init_timer(&buttons_timer);
	buttons_timer.function = buttons_timer_function;
	add_timer(&buttons_timer);

	//ע���ж�
	for(i=0; i<4; i++)
	{
		request_irq_return_value[i] = \
			request_irq(pins_desc[i].irq,  buttons_irp, IRQT_BOTHEDGE, pins_desc[i].name, &pins_desc[i]);
		
		if(request_irq_return_value[i]!=0)
		{
			printk("request_irq():%d failed\n", i);
			return -1;
		}
	}

	return 0;
}

static void buttons_exit(void)
{
	int i;
	for(i=0; i<4; i++)
	{
		free_irq(pins_desc[i].irq, &pins_desc[i]);
	}

	del_timer(&buttons_timer);
	input_unregister_device(buttons_dev);
	input_free_device(buttons_dev);
	return;
}

module_init(buttons_init);
module_exit(buttons_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Tomxin in 15#351");
MODULE_DESCRIPTION("buttons_input");

