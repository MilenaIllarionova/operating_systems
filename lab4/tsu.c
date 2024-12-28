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
        .tm_year = 1961 - 1900, // Год (tm_year — годы с 1900)
        .tm_mon = 3,            // Апрель (месяцы от 0 до 11)
        .tm_mday = 12,          // День месяца
        .tm_hour = 10,          // Время старта в 10:00
        .tm_min = 0,
        .tm_sec = 0,
    };

    struct timespec64 current_time;
    struct timespec64 gagarin_time;
    s64 diff;

    // Получение текущего времени
    ktime_get_real_ts64(&current_time);

    // Преобразование даты приземления Гагарина в время
    gagarin_time.tv_sec = mktime64(gagarin_date.tm_year + 1900, gagarin_date.tm_mon + 1, gagarin_date.tm_mday,
                                   gagarin_date.tm_hour, gagarin_date.tm_min, gagarin_date.tm_sec);
    gagarin_time.tv_nsec = 0; // Устанавливаем наносекунды в 0

    // Разница в секундах между текущим временем и временем приземления Гагарина
    diff = current_time.tv_sec - gagarin_time.tv_sec;

    // Вычисляем количество полетов (108 минут = 6480 секунд)
    pr_info("Current time (seconds): %lld\n", (long long)current_time.tv_sec);
    pr_info("Gagarin landing time (seconds): %lld\n", (long long)gagarin_time.tv_sec);
    pr_info("Difference in seconds: %lld\n", (long long)diff);

    return diff > 0 ? diff / 6480 : 0; // Если разница отрицательная, возвращаем 0
}

// Чтение данных из файла /proc/gagarin
static ssize_t proc_read(struct file *file, char __user *buf, size_t count, loff_t *ppos) {
    char message[64];
    int flights = gagarin_flights_possible();
    int len;

    // Формируем сообщение для вывода
    len = snprintf(message, sizeof(message), "Gagarin could fly %d times since 1961.\n", flights);

    // Если пользователь уже прочитал данные или запрашивает меньше, чем длина сообщения
    if (*ppos > 0 || count < len)
        return 0;

    // Копируем данные в пользовательский буфер
    if (copy_to_user(buf, message, len))
        return -EFAULT;

    *ppos = len; // Устанавливаем позицию чтения
    return len;
}

// Операции с файлом /proc
static const struct file_operations proc_file_ops = {
    .owner = THIS_MODULE,
    .read = proc_read,
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
