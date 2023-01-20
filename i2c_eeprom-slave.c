/*I2C EEPROM Slave Device Driver*/

#include<linux/module.h>
#include<linux/init.h>
#include<linux/fs.h>
#include<linux/i2c.h>
#include<linux/kdev_t.h>
#include<linux/cdev.h>
#include<linux/kernel.h>

/*available i2c bus number*/
#define I2C_BUS (1)
/*eeprom ic address*/
#define SLAVE_ADDRESS (0x50)
/*our slave device name*/
#define SLAVE_DEVICE_NAME "eeprom_slave"
/*Kernel(EEPROM) Buffer Size*/
#define MEM_SIZE 256
/*Kernel(EEPROM) Buffer*/
char* eeprom_buffer;

/*device number*/
static dev_t eeprom_dev;
/*cdev structure*/
static struct cdev eeprom_cdev;
/*class structure*/
static struct class* eeprom_class;
/*I2C adapter structure*/
static struct i2c_adapter* eeprom_adapter;
/*I2C client structure*/
static struct i2c_client* eeprom_client;

/*eeprom open function*/
static int eeprom_open(struct inode* inode,struct file* file){
  eeprom_buffer = kzalloc(MEM_SIZE,GFP_KERNEL);
  if(eeprom_buffer == NULL){
    pr_err("Failed to allocate memory");
    return -1;
  }
  printk("I2C EEPROM Slave Device Opened...\n");
  return 0;
}

/*eeprom close function*/
static int eeprom_release(struct inode* inode,struct file* file){
  kfree(eeprom_buffer);
  printk("I2C EEPROM Slave Device Closed\n");
  return 0;
}

/*eeprom read function*/
static ssize_t eeprom_read(struct file* file,char* __user buf,size_t len,loff_t* off){
  int ret = 0;
  printk("Read from EEPROM Slave..\n");

  eeprom_buffer[0] = off[0];

  i2c_master_send(eeprom_client,eeprom_buffer,1);

  printk("EEPROM_BUFFER[0] = %d\n",eeprom_buffer[0]);
  
  ret = i2c_master_recv(eeprom_client,eeprom_buffer,len);

  if(ret < 0){
    pr_err("cannot read from eeprom:");
    return -1;
  }

  printk("Reading %d number of bytes\n",ret);

  if(copy_to_user(buf,eeprom_buffer,len) > 0){
    pr_err("Couldn't read all the bytes to user\n");
    return -1;
  }
  printk("Reading: %s\n",eeprom_buffer);

  return ret;
}

/*eeprom write function*/
static ssize_t eeprom_write(struct file* file,const char* __user buf,size_t len,loff_t* off){
  int ret=0;
  printk("Writing to EEPROM Slave..\n");

  eeprom_buffer[0] = off[0];
  printk("EEPROM_BUFFER[0] = %d\n",eeprom_buffer[0]);
  
  ret = i2c_master_send(eeprom_client,eeprom_buffer,1);
  printk("wrote %d number of bytes to eeprom\n",ret);
 
  if(copy_from_user(eeprom_buffer + 1,buf,len) > 0){
    pr_err("couldn't copy all the bytes from user\n");
    return -1;
  }
  printk("Writing: %s\n",buf);

  ret = i2c_master_send(eeprom_client,eeprom_buffer,len+1);
  
  if(ret < 0){
    pr_err("cannot write to eeprom\n");
    return -1;
  }

  printk("Writing %d number of bytes\n",ret);

  return ret;
}


/*eeprom file operations structure*/
static struct file_operations fops = {
  .owner = THIS_MODULE,
  .open = eeprom_open,
  .read = eeprom_read,
  .write = eeprom_write,
  .release = eeprom_release,
};


/*eeprom driver probe function*/
static int eeprom_probe(struct i2c_client* eeprom_client,const struct i2c_device_id* eeprom_ids){
  int ret = alloc_chrdev_region(&eeprom_dev,0,1,SLAVE_DEVICE_NAME);

  if(ret < 0){
    pr_err("Failed to register i2c eeprom slave device\n");
    return -1;
  }

  cdev_init(&eeprom_cdev,&fops);

  if(cdev_add(&eeprom_cdev,eeprom_dev,1) < 0){
    pr_err("Failed i2c eeprom slave cdev_add\n");
    unregister_chrdev_region(eeprom_dev,1);
    return -1;
  }

  eeprom_class = class_create(THIS_MODULE,"eeprom_slave_class");
  if(eeprom_class == NULL){
    pr_err("Failed to create i2c eeprom slave class\n");
    cdev_del(&eeprom_cdev);
    unregister_chrdev_region(eeprom_dev,1);
    return -1;
  }


  if(device_create(eeprom_class,NULL,eeprom_dev,NULL,SLAVE_DEVICE_NAME) == NULL){
    pr_err("Failed to create i2c eeprom slave device file\n");
    class_destroy(eeprom_class);
    cdev_del(&eeprom_cdev);
    unregister_chrdev_region(eeprom_dev,1);
    return -1;
  }

  printk("I2C EEPROM Salve Driver Probed\n");
  return 0;
}

/*eeprom driver remove function*/
static int eeprom_remove(struct i2c_client* eeprom_client){
  device_destroy(eeprom_class,eeprom_dev);
  class_destroy(eeprom_class);
  cdev_del(&eeprom_cdev);
  unregister_chrdev_region(eeprom_dev,1);
  printk("I2C EEPROM Salve Driver Removed\n");
  return 0;
}

/*i2c slave(eeprom) device id structure*/
static const struct i2c_device_id eeprom_ids[] = {
  {SLAVE_DEVICE_NAME, 0},
  {} 
};

MODULE_DEVICE_TABLE(i2c,eeprom_ids);


/*i2c driver structure*/
static struct i2c_driver eeprom_driver  = {
  .driver = {
    .name = SLAVE_DEVICE_NAME,
    .owner = THIS_MODULE,
  },
  .probe = eeprom_probe,
  .remove = eeprom_remove,
  .id_table = eeprom_ids,
};

/*EEPROM IC information structure*/
static const struct i2c_board_info eeprom_ic_info = {
  I2C_BOARD_INFO(SLAVE_DEVICE_NAME,SLAVE_ADDRESS)  
};


/*module init function*/
static int __init eeprom_init(void){
  eeprom_adapter = i2c_get_adapter(I2C_BUS);

  if(eeprom_adapter == NULL){
    pr_err("I2C adapter failed to allocate\n");
    return -1;
  }

  eeprom_client = i2c_new_client_device(eeprom_adapter,&eeprom_ic_info);

  if(eeprom_client == NULL){
    pr_err("Failed to create new client device for i2c eeprom slave\n");
    return -1;
  } 

  i2c_add_driver(&eeprom_driver);
  printk("I2C EEPROM Salve Driver loaded successsfully\n");
  return 0;
}

/*module exit function*/
static void __exit eeprom_cleanup(void){
  i2c_unregister_device(eeprom_client);
  i2c_del_driver(&eeprom_driver);
  printk("I2C EEPROM Salve Driver unloaded successfully\n");
}



module_init(eeprom_init);
module_exit(eeprom_cleanup);


/*Meta information*/
MODULE_AUTHOR("Ram");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("A simple I2C EEPROM Slave Device Driver");
MODULE_VERSION("1.0");
