#include "thread.h"
#include "std_int.h"
#include "string.h"
#include "global.h"
#include "memory.h"

#define PG_SIZE 4096

static void kernel_thread(thread_func* function, void* func_arge) {
    function(func_arge);
}

//初始化线程栈 thread_stack 
//将待执行的函数和参数放到thread_stack中相应位置
void thread_create(struct task_struct* pthread, thread_func func, void* func_arge) {
    //预留中断栈空间
    pthread->self_kstack -= sizeof(struct intr_stack);
    //预留线程栈空间
    pthread->self_kstack -= sizeof(struct thread_stack);
    
    struct thread_stack* kthread_stack = (struct thread_stack*)pthread->self_kstack;
    kthread_stack->eip = kernel_thread;
    kthread_stack->function = func;
    kthread_stack->func_arge = func_arge;
    kthread_stack->ebp = kthread_stack->ebx = kthread_stack->esi = kthread_stack->edi = 0;
}
void init_thread(struct task_struct* pthread, char* name, int prio) {
    memset(pthread, 0, sizeof(*pthread));
    strcpy(pthread->name, name);
    pthread->status = TASK_RUNING;
    pthread->priority = prio;
    pthread->self_kstack = (uint32_t*)((uint32_t)pthread + PG_SIZE);    //  指向栈顶
    pthread->stack_magic = 0x19870916;
}
struct task_struct* thread_start(char* name, int prio, thread_func func, void* func_agre) {
    //从物理内存中申请一个物理页
    struct task_struct* thread = get_kernel_pages(1);
    init_thread(thread, name, prio);
    thread_create(thread, func, func_agre);
    asm volatile ( "movl %0, %%esp; \
                  pop %%ebp;     \
                  pop %%ebx;      \
                  pop %%edi;      \
                  pop %%esi;      \
                ret" : : "g"(thread->self_kstack) : "memory"
    );
    return thread;
}
