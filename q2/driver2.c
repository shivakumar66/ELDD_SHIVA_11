
#include<linux/init.h>              
#include<linux/kernel.h>            // for printk pr_info
#include<linux/module.h>            
#include<linux/kdev_t.h>            // for MKDEV() , MAJOR(), MINOR()
#include<linux/types.h>         
#include<linux/fs.h>                // for inode structure & File operations structure
#include<linux/cdev.h>              // for cdev structure
#include<linux/device.h>
#include<linux/uaccess.h>           // for copy_to_user and copy_from_user
#include<linux/slab.h>              // for kmalloc() function
#include<linux/ioctl.h>   
#include<linux/types.h>

#include<linux/gpio.h>
#include<linux/delay.h>

#include<linux/timer.h>
#include<linux/jiffies.h>
#include<linux/kthread.h>

#define GPIO_23 23 
#define MEM_SIZE 1024


 #define WR_VALUE _IOW('a','a',int32_t*)
 #define RD_VALUE _IOR('a','b',int32_t*)
/*Timer Variable*/
#define TIMEOUT 1000    
                                                                                                                                                                                                                                                                      


static unsigned int count = 0;
int32_t value =0;

uint8_t *kernel_buffer;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev hc_cdev;
static struct timer_list hc_timer;
static int __init hc_driver_init(void);
static void __exit hc_driver_exit(void);

/*************** Driver functions **********************/
static int hc_open(struct inode *inode, struct file *file);
static int hc_release(struct inode *inode, struct file *file);
static ssize_t hc_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t hc_write(struct file *filp, const char *buf, size_t len, loff_t * off);
static long etx_ioctl(struct file *file,unsigned int cmd,unsigned long arg);
/******************************************************/

//File operation structure 
static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = hc_read,
        .write          = hc_write,
        .open           = hc_open,
        .unlocked_ioctl = etx_ioctl,
        .release        = hc_release,
};
 /*write ioctl into kernel*/
static long etx_ioctl(struct file *file,unsigned int cmd,unsigned long arg)
{
    switch(cmd)
    {
        case WR_VALUE:
            if(copy_from_user(&value,(int32_t*)arg,sizeof(value)))
            {
                pr_err("DATA write : err!");
            }
            pr_info("value = %d\n",value);
            break;
        case RD_VALUE:
            if(copy_to_user((int32_t*) arg,&value,sizeof(value)))
            {
                pr_err("DATA read : Err\n");
            }
            break;
        default:
            pr_info("default\n");
            break;
    }
    return 0;
}


/*timer callback function*/
void timer_callback(struct timer_list * data)
{
    printk("timer call back function called[%d]\n",count++);
    mod_timer(&hc_timer,jiffies+msecs_to_jiffies(TIMEOUT));
}
/*
** This function will be called when we open the Device file
*/
static int hc_open(struct inode *inode, struct file *file)
{
    pr_info("Device File Opened...!!!\n");
    return 0;
}

/*
** This function will be called when we close the Device file
*/
static int hc_release(struct inode *inode, struct file *file)
{
    pr_info("Device File Closed...!!!\n");
    return 0;
}

/*
** This function will be called when we read the Device file
*/
static ssize_t hc_read(struct file *filp,char __user *buf, size_t len, loff_t *off)
{
    pr_info("Read Function\n");
    return 0;
}

/*
** This function will be called when we write the Device file
*/
static ssize_t hc_write(struct file *filp,const char __user *buf, size_t len, loff_t *off)
{
    pr_info("Write function\n");
    return len;
}

/*
** Module Init function
*/ 
static int __init hc_driver_init(void)
{
    /*Allocating Major number*/
    if((alloc_chrdev_region(&dev, 0, 1, "hc_Dev")) <0){
            pr_err("Cannot allocate major number\n");
            return -1;
    }
    pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
    /*Creating cdev structure*/
    cdev_init(&hc_cdev,&fops);
 
    /*Adding character device to the system*/
    if((cdev_add(&hc_cdev,dev,1)) < 0){
        pr_err("Cannot add the device to the system\n");
        goto r_class;
    }
 
    /*Creating struct class*/
    if((dev_class = class_create(THIS_MODULE,"hc_class")) == NULL){
        pr_err("Cannot create the struct class\n");
        goto r_class;
    }
 
    /*Creating device*/
    if((device_create(dev_class,NULL,dev,NULL,"ioctl_device")) == NULL){
        pr_err("Cannot create the Device 1\n");
        goto r_device;
    }
     // Creating Physical Memory
    if((kernel_buffer = kmalloc(MEM_SIZE,GFP_KERNEL)) == 0) {
        pr_err("Can not allocate memory in Kernel\n");
        goto r_device;
    }
    /*to setup a timer*/
    timer_setup(&hc_timer,timer_callback,0);
    /*to modify the timer*/
     mod_timer(&hc_timer,jiffies+msecs_to_jiffies(TIMEOUT));
    
        


 
    pr_info("Device Driver Insert...Done!!!\n");
    return 0;
r_device:
    class_destroy(dev_class);
r_class:
    unregister_chrdev_region(dev,1);
    return -1;
}


/*
** Module exit function
*/
static void __exit hc_driver_exit(void)
{
    /* remove kernel timer when unloading module */
   
    device_destroy(dev_class,dev);
    class_destroy(dev_class);
    cdev_del(&hc_cdev);
    unregister_chrdev_region(dev, 1);
    pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(hc_driver_init);
module_exit(hc_driver_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shivakumar");
MODULE_DESCRIPTION("A simple device driver - Kernel Timer");
MODULE_VERSION("1.0");