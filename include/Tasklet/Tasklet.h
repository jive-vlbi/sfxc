#ifndef TASKLET_HH
#define TASKLET_HH

#include <string>

typedef enum {TASKLET_FINISHED,
              TASKLET_CONTINUE,
              TASKLET_BLOCKED} TaskletState;

class Tasklet
{

public:
    Tasklet(const std::string& name);
    virtual ~Tasklet();
    virtual void do_task() = 0;

    virtual bool may_block() = 0;
    virtual bool is_terminated() = 0;

    const std::string& desc();
protected:
    std::string m_name;
    std::string m_desc;
private:
};


typedef Tasklet* pTasklet;



#endif // TASKLET_HH
