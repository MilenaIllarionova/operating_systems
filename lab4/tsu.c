#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/time.h>

#define PROCFS_NAME "gagarin"

static struct proc_dir_entry *our_proc_file = NULL;

// Функция для вычисления количества полетов Гагарина
static int gagarin_flights_possible(void) {
    struct tm gagarin_date = {
        .tm_year = 1961 - 1900,
        .tm_mon = 3,
        .tm_mday = 12,
        .tm_hour = 10,
        .tm_min = 0,
        .tm_sec = 0,
    };

    struct timespec64 current_time;
    struct timespec64 gagarin_time;
    s64 diff;

    ktime_get_real_ts64(&current_time);

    gagarin_time.tv_sec = mktime64(gagarin_date.tm_year + 1900, gagarin_date.tm_mon + 1, gagarin_date.tm_mday,
                                   gagarin_date.tm_hour, gagarin_date.tm_min, gagarin_date.tm_sec);
    gagarin_time.tv_nsec = 0;

    diff = current_time.tv_sec - gagarin_time.tv_sec;

    pr_info("Current time (seconds): %lld\n", (long long)current_time.tv_sec);
    pr_info("Gagarin landing time (seconds): %lld\n", (long long)gagarin_time.tv_sec);
    pr_info("Difference in seconds: %lld\n", (long long)diff);

    return diff > 0 ? diff / 6480 : 0;
}

// Чтение данных из файла /proc/gagarin
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char message[64];
    int flights = gagarin_flights_possible();
    int len;

    len = snprintf(message, sizeof(message), "Gagarin could fly %d times since 1961.\n", flights);

    if (*ppos > 0 || count < len)
        return 0;

    if (copy_to_user(buf, message, len))
        return -EFAULT;

    *ppos = len;
    return len;
}

// Операции с файлом /proc
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read,
};

// Инициализация модуля
static int __init gagarin_init(void) {
    our_proc_file = proc_create(PROCFS_NAME, 0444, NULL, &proc_file_ops);
    if (!our_proc_file) {
        pr_err("Error: Could not initialize /proc/%s\n", PROCFS_NAME);
        return -ENOMEM;
    }
    pr_info("Module gagarin loaded: /proc/%s created\n", PROCFS_NAME);
    return 0;
}

// Завершение работы модуля
static void __exit gagarin_exit(void) {
    proc_remove(our_proc_file);
    pr_info("Module gagarin unloaded: /proc/%s removed\n", PROCFS_NAME);
}

module_init(gagarin_init);
module_exit(gagarin_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Модуль для вычисления количества возможных полетов Гагарина.");
