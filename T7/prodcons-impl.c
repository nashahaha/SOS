/* Implemente aqui el driver para /dev/prodcons */

/* Necessary includes for device drivers */
#include <linux/init.h>
/* #include <linux/config.h> */
#include <linux/module.h>
#include <linux/kernel.h> /* printk() */
#include <linux/slab.h> /* kmalloc() */
#include <linux/fs.h> /* everything... */
#include <linux/errno.h> /* error codes */
#include <linux/types.h> /* size_t */
#include <linux/proc_fs.h>
#include <linux/fcntl.h> /* O_ACCMODE */
#include <linux/uaccess.h> /* copy_from/to_user */

#include "kmutex.h"

MODULE_LICENSE("Dual BSD/GPL");
#define TRUE 1
#define FALSE 0
#define MAX_SIZE 8192 // del buffer

static int prodcons_open(struct inode *inode, struct file *filp);    // inicializar el dispositivo
static int prodcons_release(struct inode *inode, struct file *filp); // finalizar el dispositivo
static ssize_t prodcons_read(struct file *filp, char *buf, size_t count, loff_t *f_pos); // que hacer en lectura
static ssize_t prodcons_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos); // que hacer en escritura 

void prodcons_exit(void); 
int prodcons_init(void);  

// Access functions
struct file_operations prodcons_fops = {
    read: prodcons_read,
    write: prodcons_write,
    open: prodcons_open,
    release: prodcons_release
};

// Declaracion de las funciones init y exit
module_init(prodcons_init);
module_exit(prodcons_exit);

// Variables globales
int prodcons_major = 61;

static char *prodcons_buffer;
static int in, out, size;

// Se asume que existe un unico escritor y varios lectores
static int readers; // número de lectores en espera
static int turno;   // número del turno actual

static KMutex mutex;
static KCondition cond;

int prodcons_init(void){
    int rc;

    /* Registering device */
    rc = register_chrdev(prodcons_major, "prodcons", &prodcons_fops);
    if (rc < 0) {
        printk(
        "<1>prodcons: cannot obtain major number %d\n", prodcons_major);
        return rc;
    }

    in = out = size= 0;
    m_init(&mutex);
    c_init(&cond);
    readers=0;
    turno=0;

    /* Allocating prodcons_buffer */
    prodcons_buffer = kmalloc(MAX_SIZE, GFP_KERNEL);
    if (prodcons_buffer==NULL) {
        prodcons_exit();
        return -ENOMEM;
    }
    memset(prodcons_buffer, 0, MAX_SIZE);

    printk("<1>Inserting prodcons module\n");
    return 0;
}

void prodcons_exit(void){
    /* Freeing the major number */
    unregister_chrdev(prodcons_major, "prodcons");

    /* Freeing buffer prodcons */
    if (prodcons_buffer) {
        kfree(prodcons_buffer);
    }

    printk("<1>Removing prodcons module\n");
}

static int prodcons_open(struct inode *inode, struct file *filp) {
    char *mode=   filp->f_mode & FMODE_WRITE ? "write" :
                    filp->f_mode & FMODE_READ ? "read" :
                    "unknown";
    printk("<1>open %p for %s\n", filp, mode);
    return 0;
}

static int prodcons_release(struct inode *inode, struct file *filp) {
    printk("<1>release %p\n", filp);
    return 0;
}

static ssize_t prodcons_read(struct file *filp, char *buf,
                    size_t ucount, loff_t *f_pos) {
  int count = ucount;

  printk("<1>read %p %d\n", filp, count);
  m_lock(&mutex);
  
  int my_turn = readers;

  while (size==0 || my_turn!=turno) {
    /* si no hay nada en el buffer o no es su turno, el lector espera */
    if (c_wait(&cond, &mutex)) {
      printk("<1>read interrupted\n");
      count = -EINTR;
      goto epilog;
    }
  }

  if (count > size) {
    count = size;
  }

  /* Transfiriendo datos hacia el espacio del usuario */
  for (int k=0; k<count; k++) {
    if (copy_to_user(buf+k, prodcons_buffer+out, 1)!=0) {
      /* el valor de buf es una direccion invalida */
      count = -EFAULT;
      goto epilog;
    }
    printk("<1>read byte %c (%d) from %d\n",
            prodcons_buffer[out], prodcons_buffer[out], out);
    out= (out+1)%MAX_SIZE;
    size--;
  }

epilog:
  c_broadcast(&cond);
  m_unlock(&mutex);
  return count;
}

static ssize_t prodcons_write( struct file *filp, const char *buf,
                      size_t ucount, loff_t *f_pos) {
  int count= ucount;

  printk("<1>write %p %d\n", filp, count);
  m_lock(&mutex);

  for (int k= 0; k<count; k++) {
    while (size==MAX_SIZE) {
      /* si el buffer esta lleno, el escritor espera */
      if (c_wait(&cond, &mutex)) {
        printk("<1>write interrupted\n");
        count= -EINTR;
        goto epilog;
      }
    }

    if (copy_from_user(prodcons_buffer+in, buf+k, 1)!=0) {
      /* el valor de buf es una direccion invalida */
      count= -EFAULT;
      goto epilog;
    }
    printk("<1>write byte %c (%d) at %d\n",
           prodcons_buffer[in], prodcons_buffer[in], in);
    in= (in+1)%MAX_SIZE;
    size++;
    c_broadcast(&cond);
  }

epilog:
  m_unlock(&mutex);
  return count;
}