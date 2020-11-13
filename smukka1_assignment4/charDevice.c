#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/sched.h>
#include <linux/semaphore.h>
#include <linux/miscdevice.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/rtc.h>
#include <linux/fs.h>
#include <asm/uaccess.h>

#define row_size 100


MODULE_LICENSE("GPL");

static struct semaphore full;
static struct semaphore empty;
static struct semaphore mutex;

//static struct miscdevice charDevice;

char **device_buffer;
int buffer_size=100;
static int read_row=0,write_row=0;

static int device_open(struct inode *inode, struct file *file){

        printk(KERN_INFO "Character device is opened \n");
        return 0;

}

static ssize_t device_read(struct file *file,char *buffer,size_t size,loff_t *offset){

//      printk(KERN_INFO "Inside Read function");
        int j;
        unsigned int res=0;
        if(down_interruptible(&full) < 0)
                return -1;
        if(down_interruptible(&mutex) < 0)
                return -1;
        read_row%=buffer_size;
        for(j=0;j<size; j++){
        res=copy_to_user(&buffer[j],&device_buffer[read_row][j],1);
        if(res<0){
                printk(KERN_ALERT "Error in copy_to_user");
                return res;
        }
//      bytes_read++;
//      count--;
        }
//      buffer_size--;
        read_row++;
        up(&mutex);
        up(&empty);

        return j;
}

static ssize_t device_write(struct file *file,const char *buffer, size_t size, loff_t *offset){

        int i;
        unsigned int result=0;
//      printk(KERN_INFO "Inside write function");

        if(down_interruptible(&empty) < 0)
                return -1;
        if(down_interruptible(&mutex) < 0)
                return -1;
        write_row%=buffer_size;
        for(i=0; i<size; i++){
        result=copy_from_user(&device_buffer[write_row][i],&buffer[i],1);
        if(result<0){
                printk(KERN_ALERT "Error in copy_from_user");
                return result;
        }
//      write_row++;
//      count++;
        }
        write_row++;
        up(&mutex);
        up(&full);
        return i;
}

static int device_release(struct inode *inode, struct file *file){

        printk(KERN_INFO"Character device is closed \n");
        return 0;

}

static struct file_operations fops={

        .open=device_open,
        .read=device_read,
        .write=device_write,
        .release=device_release

};

static struct miscdevice charDevice={

        .minor=MISC_DYNAMIC_MINOR,
        .name="charDevice",
        .fops=&fops
};

int init_module(){

        int row_count=0;
        int register_result=misc_register(&charDevice);
        if(register_result < 0){
                printk(KERN_ERR"Device cant be registered");
                misc_deregister(&charDevice);
                return register_result;
        }

        printk(KERN_INFO "Character device registerd with minor number %d\n",charDevice.minor );

//      int row_count=0;

        device_buffer=(char **)kmalloc((buffer_size)*sizeof(char *),GFP_KERNEL);
//      int row_count=0;
        while(row_count < buffer_size){
                device_buffer[row_count]=(char *)kmalloc((row_size)*sizeof(char),GFP_KERNEL);
                row_count++;
        }

        sema_init(&full,0);
        sema_init(&empty,buffer_size);
        sema_init(&mutex,1);

        return 0;
}

void cleanup_module(){

        int k=0;
        while(k<buffer_size){
                kfree(device_buffer[k]);
                k++;
        }
        misc_deregister(&charDevice);
        printk(KERN_INFO"Character device unregistered");
}
