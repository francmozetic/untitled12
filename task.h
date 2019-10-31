#ifndef TASK_H
#define TASK_H

class task_system {
    void run(unsigned i);

public:
    task_system();
    ~task_system();

    template<typename F>
    void async(F&& f);
};

#endif // TASK_H
