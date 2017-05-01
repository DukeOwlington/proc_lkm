#include <linux/init.h> /* Macros used to mark up functions e.g. __init __exit */
#include <linux/module.h> /* Core header for loading LKMs into the kernel */
#include <linux/kernel.h> /* Contains types, macros, functions for the kernel */
#include <linux/proc_fs.h> /* Header for the Linux proc filesystem support */
#include <asm/uaccess.h>   /* Required for the copy to user function */

MODULE_LICENSE("GPL"); /* The license type -- this affects available functionality */
MODULE_AUTHOR("MadMax"); /* The author -- visible when you use modinfo */
MODULE_DESCRIPTION("A simple proc filesystem lkm"); /* The description -- see modinfo */
MODULE_VERSION("0.1");               /* A version number to inform users */

#define PROCFS_MAX_SIZE 1024
#define PROCFS_NAME "proc_module"

static ssize_t procfile_read(struct file *, char *, size_t, loff_t *);
static ssize_t procfile_write(struct file *, const char *, size_t, loff_t *);

/* The buffer used to store character for this module */
static char procfs_buffer[PROCFS_MAX_SIZE] = {0};

/* The size of the buffer */
static unsigned long procfs_buffer_size = 0;

struct proc_dir_entry *Our_Proc_File;

static struct file_operations cmd_file_ops = {
    .owner = THIS_MODULE, .write = procfile_write, .read = procfile_read,
};

static int __init proc_init(void) {
  Our_Proc_File =
      proc_create(PROCFS_NAME, S_IFREG | S_IRUGO, NULL, &cmd_file_ops);

  if (Our_Proc_File == NULL) {
    remove_proc_entry(PROCFS_NAME, NULL);

    printk(KERN_ALERT "Error: Could not initialize /proc/%s\n", PROCFS_NAME);
    return -ENOMEM;
  }

  /*
   * KUIDT_INIT is a macro defined in the file 'linux/uidgid.h'. KGIDT_INIT also
   * appears here.
   */
  proc_set_user(Our_Proc_File, KUIDT_INIT(0), KGIDT_INIT(0));
  proc_set_size(Our_Proc_File, 37);

  printk(KERN_INFO "/proc/%s created\n", PROCFS_NAME);
  return 0;
}

static void __exit proc_exit(void) {
  // remove_proc_entry(procfs_name, &proc_root);
  remove_proc_entry(PROCFS_NAME, NULL);
  printk(KERN_INFO "/proc/%s removed\n", PROCFS_NAME);
}

/* This function is called with the /proc file is read */
static ssize_t procfile_read(struct file *file, char *buffer, size_t length,
                             loff_t *offset) {
  int error_count = 0;
  /*
   * copy_to_user has the format ( * to, *from, size)
   * and returns 0 on success
   */
  error_count = copy_to_user(buffer, procfs_buffer, procfs_buffer_size);

  if (error_count == 0) { /* if true then have success */
    printk(KERN_INFO "proc: Sent %lu characters to the user\n",
           procfs_buffer_size);
    return (procfs_buffer_size =
                0); /* clear the position to the start and return 0 */
  } else {
    printk(KERN_INFO "proc: Failed to send %d characters to the user\n",
           error_count);
    return -EFAULT; /* Failed -- return a bad address message (i.e. -14) */
  }
}

/* This function is called with the /proc file is written */
static ssize_t procfile_write(struct file *file, const char *buffer,
                              size_t length, loff_t *offset) {
  sprintf(procfs_buffer, "%s (%zu letters)", buffer,
          length); /* appending received string with its length */
  procfs_buffer_size =
      strlen(procfs_buffer); /* store the length of the stored message */
  printk(KERN_INFO "chardev: Received %zu characters from the user\n", length);
  return length;
}

/*
 * A module must use the module_init() module_exit() macros from linux/init.h,
 * which
 *  identify the initialization function at insertion time and the cleanup
 * function (as
 *  listed above)
 */
module_init(proc_init);
module_exit(proc_exit);
