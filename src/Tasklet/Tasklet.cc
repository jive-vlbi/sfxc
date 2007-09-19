#include "Tasklet/Tasklet.h"

Tasklet::Tasklet(const std::string& name)
{
    m_desc = "No description available";
    m_name = name;
}

Tasklet::~Tasklet()
{
    //dtor
}

const std::string& Tasklet::desc(){ return m_desc; }

