#include <linux/syscalls.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/seq_file.h>
#include <linux/uaccess.h>
#include <linux/kallsyms.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <asm/unistd.h>
#include <linux/cred.h>

MODULE_LICENSE("GPL");

char printbuffer[128] = { 0x0, };
char open_block_file[128] = { 0x0, } ;
int open_block_user = 0;
int kill_block_user = 0;
void ** sctable ;
int count = 0 ;
int kill_block_count = 0;
int open_block_count = 0;

asmlinkage int (*orig_sys_open)(const char __user * filename, int flags, umode_t mode) ; 
asmlinkage int (*orig_sys_kill)(pid_t pid, int sig) ;

asmlinkage int mousehole_sys_open(const char __user * filename, int flags, umode_t mode)
{
	char fname[256] ;
	copy_from_user(fname, filename, 256) ;

	if (open_block_file[0] != 0x0 && strstr(fname, open_block_file) != 0x0 && current->cred->uid.val == open_block_user) {
		printk("file open blocked. file name : %s, uid : %d", open_block_file, open_block_user);
		open_block_count++;
		return -1;
	}
	return orig_sys_open(filename, flags, mode) ;
}
asmlinkage int mousehole_sys_kill(pid_t pid, int sig){
	struct task_struct * t ;

	for_each_process(t) {
		if((t->pid == pid) && (t->cred->uid.val == kill_block_user)){
			printk("kill blocked. uid : %d, pid : %d", kill_block_user, pid);
			kill_block_count++;
			return -1;
		}
	}
	return orig_sys_kill(pid, sig);
}

static 
int mousehole_proc_open(struct inode *inode, struct file *file) {
	return 0 ;
}

static 
int mousehole_proc_release(struct inode *inode, struct file *file) {
	return 0 ;
}

static
ssize_t mousehole_proc_read(struct file *file, char __user *ubuf, size_t size, loff_t *offset) 
{
	ssize_t toread ;
	toread = strlen(printbuffer) >= *offset + size ? size : strlen(printbuffer) - *offset ;
	if (copy_to_user(ubuf, printbuffer + *offset, toread))
		return -EFAULT ;	
	*offset = *offset + toread ;
	return toread ;
}

static 
ssize_t mousehole_proc_write(struct file *file, const char __user *ubuf, size_t size, loff_t *offset) 
{
	char buf[128] ={0x0, };
	int i=0;
	
	*offset = 0;
	if (*offset != 0 || size > 128)
		return -EFAULT ;

	if (copy_from_user(buf, ubuf, size))
		return -EFAULT ;
	buf[size] = '\0';	
	
	if(buf[0] == '0'){
		if(buf[1] == '0'){
			sctable[__NR_open] = orig_sys_open ;
			sprintf(printbuffer, "disabled mousehole");
		}
		else{
			sctable[__NR_open] = mousehole_sys_open ;
			sprintf(printbuffer, "enabled mousehole");
			open_block_count =0;
		}
	}
	else if(buf[0] == '1'){
		if(buf[1] == '0'){
			sctable[__NR_kill] = orig_sys_kill ;
			sprintf(printbuffer, "disabled killhook");
		}
		else{
			sctable[__NR_kill] = mousehole_sys_kill ;
			sprintf(printbuffer, "enabled killhook");
			kill_block_count=0;
		}
	}
    else if(buf[0] == '2'){
		for(i=0; i<strlen(buf);i++){
			buf[i] = buf[i+1];
		}
		sscanf(buf, "%s", open_block_file);
		sprintf(printbuffer, "file name : %s, uid : %d", open_block_file, open_block_user);
	}
	else if(buf[0] == '3'){
		for(i=0; i<strlen(buf);i++){
                        buf[i] = buf[i+1];
                }
                sscanf(buf, "%d", &open_block_user);	
		sprintf(printbuffer, "file name : %s, uid : %d", open_block_file, open_block_user);
	}
	else if(buf[0] == '4'){
		for(i=0; i<strlen(buf);i++){
                        buf[i] = buf[i+1];
                }
                sscanf(buf, "%d", &kill_block_user);
		sprintf(printbuffer, "uid : %d", kill_block_user);
	}
	else if(buf[0] == '5'){
		sprintf(printbuffer, "file open block info : uid : %d, file name : %s, block count : %d", open_block_user, open_block_file, open_block_count);
	}
	else if(buf[0] == '6'){
		sprintf(printbuffer, "process kill block info : uid : %d, kill block count : %d", kill_block_user, kill_block_count);
	}
	else{
		sprintf(printbuffer, "wrong input");
	}
	*offset = strlen(buf) ;
	return *offset ;
}

static const struct file_operations mousehole_fops = {
	.owner = 	THIS_MODULE,
	.open = 	mousehole_proc_open,
	.read = 	mousehole_proc_read,
	.write = 	mousehole_proc_write,
	.llseek = 	seq_lseek,
	.release = 	mousehole_proc_release,
} ;

static 
int __init mousehole_init(void) {
	unsigned int level ;
    pte_t * pte ;
	proc_create("mousehole", S_IRUGO | S_IWUGO, NULL, &mousehole_fops) ;
    sctable = (void *) kallsyms_lookup_name("sys_call_table") ;
	orig_sys_open = sctable[__NR_open] ;
	orig_sys_kill = sctable[__NR_kill] ;
	pte = lookup_address((unsigned long) sctable, &level) ;
    if (pte->pte &~ _PAGE_RW) pte->pte |= _PAGE_RW ;
	return 0;
}

static 
void __exit mousehole_exit(void) {
	unsigned int level ;
    pte_t * pte ;
	remove_proc_entry("mousehole", NULL) ;

    sctable[__NR_open] = orig_sys_open ;
	sctable[__NR_kill] = orig_sys_kill ;
    pte = lookup_address((unsigned long) sctable, &level) ;
    pte->pte = pte->pte &~ _PAGE_RW ;
}

module_init(mousehole_init);
module_exit(mousehole_exit);
