#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/ctype.h>
#include <linux/types.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/regmap.h>
#include <linux/mfd/syscon.h>
#include <linux/mfd/syscon/imx6q-iomuxc-gpr.h>
#include <linux/pinctrl/consumer.h>
#include <linux/regulator/consumer.h>
#include <linux/fsl_devices.h>
#include <linux/mipi_csi2.h>
#include <media/v4l2-chip-ident.h>
#include <media/v4l2-int-device.h>



#define DEV_NAME "gpio-test"

#define DEV_MAJOR 255
//there is DEV_MAJOR func it is able to sourch empty major number

#define BASE_GPIO_ADDR 0x020E0000
#define IOMUXC_SW_MUX_CTL_PAD_GPIO19 0x254
#define IOMUXC_SW_PAD_CTL_PAD_GPIO19 0x624



int mgpio_open(struct inode *inode, struct file *fp)
{
	struct regmap *gpr;
	unsigned int addr = 0 ;

	printk("\nDD mgpio_open\n");
	gpr = syscon_regmap_lookup_by_compatible("fsl,imx6q-iomuxc-gpr");

    if(!IS_ERR(gpr)){
   		 printk("syscon_regmap_lookup_by_compatible() gpr 	= 0x%x\n",gpr);  
		 regmap_read(gpr,0,&addr);
		 printk("gpr BASE ADDR VALUE				= 0x%x\n",addr);
    }

	return 0;
}

ssize_t mgpio_write(struct file *fp, void *buf, size_t count, loff_t *f_pos)
{
	unsigned int buffer;
	printk("DD write\n");
	//copy_from_user(buffer,(int *)buf,sizeof(buffer));

	return 0;
}

int mgpio_release(struct inode *inode, struct file *fp)
{
	printk("gpio driver close sdjtei\n");

	return 0;
}
int mgpio_unlocked_ioctl(struct file *fp, unsigned int cmd, unsigned long arg)
{
	struct regmap *gpr;
	unsigned int addr = 0 ;
	gpr = syscon_regmap_lookup_by_compatible("fsl,imx6q-iomuxc-gpr");

	switch(cmd)
	{
		case 0://IOMUXC_SW_PAD_CTL_PAD_GPIO19 DEF - > disable any point
			printk("IOMUXC_SW_PAD_CTL_PAD_GPIO19 before\n");
			regmap_read(gpr,IOMUXC_SW_PAD_CTL_PAD_GPIO19,&addr);
			printk("read REGISTER VALUE: 0x%x\n",addr);

			regmap_update_bits(gpr, IOMUXC_SW_PAD_CTL_PAD_GPIO19, 1 << 12, 0 << 12); //pull/keeper disabled
			regmap_update_bits(gpr, IOMUXC_SW_PAD_CTL_PAD_GPIO19, 1 << 16, 0 << 16); //Disabled-CMOS input

			printk("IOMUXC_SW_PAD_CTL_PAD_GPIO19 after\n");
			regmap_read(gpr,IOMUXC_SW_PAD_CTL_PAD_GPIO19,&addr);
			printk("read REGISTER VALUE: 0x%x\n",addr);
			break;
		case 1:
			printk("CASE 1\n");
			break;
	}
	 return 0;

}

struct file_operations mgpio_fops =
{
	owner : THIS_MODULE,
	write :  mgpio_write,
	open  : mgpio_open,
	unlocked_ioctl : mgpio_unlocked_ioctl,
	release :  mgpio_release,
};

int mgpio_init(void)
{
	int result;
	printk("call gpio_init\n");
	result = register_chrdev(DEV_MAJOR,DEV_NAME,&mgpio_fops);
	if(result <0 )
	{
		printk("gpio-test is not register\n");
		return result;
	}
	return 0;
}

int mgpio_exit(void)
{
	printk("call gpio_exit\n");
	unregister_chrdev(DEV_MAJOR,DEV_NAME);
	return 0;
}

module_init(mgpio_init);
module_exit(mgpio_exit);
MODULE_LICENSE("Dual BSD/GPL");

