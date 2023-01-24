/*Verifying Fileoperations through internel memory with userspace application*/
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/slab.h>                 //kmalloc()
#include <linux/uaccess.h>              //copy_to/from_user()
#include <linux/err.h>
#include <linux/semaphore.h>
#include<linux/wait.h>
#include <linuxkthread.h>

#define mem_size        1024           //Memory Size
 static struct task_struct *wait_thread;
dev_t dev = 0;
static struct class *dev_class;
static struct cdev driver2_cdev;
static char kernel_buffer[mem_size];
struct semaphore my_sema;
DECLARE_WAIT_QUEUE_HEAD(wait_queue_wq);
int wait_queue_flag;

/*Function Prototypes*/

static int      __init driver2_init(void);
static void     __exit driver2_exit(void);
static int      file_open(struct inode *inode, struct file *file);
static int      file_release(struct inode *inode, struct file *file);
static ssize_t  file_read(struct file *filp, char __user *buf, size_t len,loff_t * off);
static ssize_t  file_write(struct file *filp, const char *buf, size_t len, loff_t * off);


/*File Operations structure*/

static struct file_operations fops =
{
        .owner          = THIS_MODULE,
        .read           = file_read,
        .write          = file_write,
        .open           = file_open,
        .release        = file_release,
};
 static int wait_function(void *unused)
{
    while(1)
    {
        pr_info("waiting for event...\n");
        wait_event_interruptible(wait_queue_wq, wait_queue_flag != 0);
        
        if(wait_queue_flag ==1)
        {
            pr_info("event came from read function -%d\n",++read_count);
            wait_queue_flag=0;
            return 0;
        }
        if(wait_queue_flag ==3)
        {
            pr_info("event came from write funciton -%d\n",++write_count);
            wait_queue_flag=0;
            return 0;
        }
    }
    do_exit(1);
    return 0;
}
/*This function will be called when we open the Device file*/

static int file_open(struct inode *inode, struct file *file)
{
        pr_info("Device File Opened...!!!\n");
        return 0;
}

/*This function will be called when we close the Device file*/

static int file_release(struct inode *inode, struct file *file)
{
        pr_info("Device File Closed...!!!\n");
        return 0;
}

/*This function will be called when we read the Device file*/

static ssize_t file_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
        //Copy the data from the kernel space to the user-space
        up(&my_sema);
        wait_queue_flag=1;
        wake_up_interruptible(&wait_queue_wq);
        if( copy_to_user(buf, kernel_buffer, mem_size) )
        {
                pr_err("Data Read : Err!\n");
        }
        pr_info("Data Read : Done!\n");
        return mem_size;
}

/*This function will be called when we write the Device file*/

static ssize_t file_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
        //Copy the data to kernel space from the user-space
        down(&my_sema);
        if( copy_from_user(kernel_buffer, buf, len) )
        {
                pr_err("Data Write : Err!\n");
        }
        pr_info("Data Write Done : %s\n",kernel_buffer);
        wait_queue_flag=2;
        wake_up_interruptible(&wait_queue_wq);
        return len;
}

/*Module Init function*/

static int __init driver2_init(void)
{		
        /*Allocating Major number*/
        if((alloc_chrdev_region(&dev, 0, 2, "driver2_Dev")) <0){
                pr_info("Cannot allocate major number\n");
                return -1;
        }
        pr_info("Major = %d Minor = %d \n",MAJOR(dev), MINOR(dev));
 
        /*Creating cdev structure*/
        cdev_init(&driver2_cdev,&fops);
 
        /*Adding character device to the system*/
        if((cdev_add(&driver2_cdev,dev,2)) < 0){
            pr_info("Cannot add the device to the system\n");
            goto r_class;
        }
 
        
        /*Creating Physical memory*/
       /*if((kernel_buffer = kmalloc(mem_size , GFP_KERNEL)) == 0){
            pr_info("Cannot allocate memory in kernel\n");
            goto r_device;
        }
        */
         wait_thread = kthread_create(wait_function,NULL, "waitthread");
        if(wait_thread)
        {
            pr_info("thread created successfully\n");
            wake_up_process(wait_thread);
        }
        else
            pr_info("thread creation failed\n");
        strcpy(kernel_buffer, "Hello_World");
        sema_init(&my_sema,1);
        pr_info("Device Driver Insert...Done!!!\n");
        return 0;
 

r_class:
        unregister_chrdev_region(dev,1);
        return -1;
}

/*Module exit function*/

static void __exit driver2_exit(void)
{
	/*kfree(kernel_buffer);*/
        device_destroy(dev_class,dev);
        class_destroy(dev_class);
        cdev_del(&driver2_cdev);
        unregister_chrdev_region(dev, 2);
        pr_info("Device Driver Remove...Done!!!\n");
}
 
module_init(driver2_init);
module_exit(driver2_exit);
 
MODULE_LICENSE("GPL");
MODULE_AUTHOR("shivakumar");
MODULE_DESCRIPTION("module exam");