#ifndef __THRERAD_THREAD_H
#define __THRERAD_THREAD_H
#include "std_int.h"

typedef void thread_func(void*);

enum task_status {
    TASK_RUNING,
    TASK_READY,
    TASK_BLOCKED,
    TASK_WAITING,
    TASK_HANGING,
    TASK_DIED
};
/***********   中断栈intr_stack   ***********
* 此结构用于中断发生时保护程序(线程或进程)的上下文环境:
* 进程或线程被外部中断或软中断打断时,会按照此结构压入上下文
* 寄存器,  intr_exit中的出栈操作是此结构的逆操作
* 此栈在线程自己的内核栈中位置固定,所在页的最顶端
********************************************/
struct intr_stack {
    uint32_t    vec_no;     // kernel.S 中宏VECTOR push %1 压入的中断号
    uint32_t    edi;    
    uint32_t    esi;
    uint32_t    ebp;
    uint32_t    esp_dummy;
    uint32_t    ebx;
    uint32_t    edx;
    uint32_t    ecx;
    uint32_t    eax;
    uint32_t    gs;
    uint32_t    fs;
    uint32_t    es;
    uint32_t    ds;

    //以下由cpu从低特权级 进入高特权级时压入
    uint32_t    err_code;
    void        (*eip) (void);
    uint32_t    cs;
    uint32_t    eflags;
    void*       esp;
    uint32_t    ss;

};

struct thread_stack {
    uint32_t    ebp;
    uint32_t    ebx;
    uint32_t    edi;
    uint32_t    esi;

    //线程第一次执行时，eip指向待调用的函数 kernel_thread
    //switch_to 时，eip保存任务切换后的新任务返回地址
    void (*eip)(thread_func* func, void* func_arge);

    // 以下仅第一次被调度上cpu时使用
    void (*unused_retaddr);         //unused_retaddr 为占位置
    thread_func* function;          //kernel_thread 所调用的函数名
    void* func_arge;                // kernel_thread 所调用的函数参数

};

// 进程 或者线程的pcb
struct task_struct {
    uint32_t*   self_kstack;        //内核线程 内核栈
    enum task_status    status;     //
    uint8_t priority;               //优先级
    char name[16];                  //线程名字
    uint32_t stack_magic;           //栈的边界标记 用于检测栈的溢出
};


void thread_create(struct task_struct* pthread, thread_func function, void* func_arg);
void init_thread(struct task_struct* pthread, char* name, int prio);
struct task_struct* thread_start(char* name, int prio, thread_func function, void* func_arg);

#endif
