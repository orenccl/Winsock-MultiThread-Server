#pragma once

#define TASK_RUN_COUNT_UNLIMITED   (-1234) // UNLIMITED times

struct TaskNode
{
	UINT	TaskNumber;
	UINT	DelayTime; 
	INT		RunCount;  
	void*	pUserData; 
	
	INT64	StartTime; 
	INT64	EndTime;   
};


class TaskInterface
{
public:
	virtual void vOnRunTask(TaskNode* node) {}
};

class TaskComponent
{
public:
	static TaskComponent* sCreate(TaskInterface* interface_link, UINT task_max = 1000);
	virtual void vRelease() = 0;

	virtual TaskNode* vAddTask(UINT task_number, UINT delay_time = 0, INT run_count = 1, void* user_data = NULL) = 0;
	virtual UINT vGetTaskCount() = 0;
	virtual void vRunTask() = 0;
};
