#include "mtpch.h"
#include "MTLibrary/Task.h"
#include "MTLibrary/List/TMPDoublyList.hpp"
#include "MTLibrary/MemoryCounter.h"
#include "MTLibrary/Logger.h"
#include "MTLibrary/Tool.h"

class TaskManager : public TaskComponent
{
	TaskInterface* pTaskInterface;
	TMPDoublyList< TaskNode >  TaskList;

public:
	void Create(TaskInterface* interface_link, UINT task_max);

	virtual void vRelease();

	virtual TaskNode* vAddTask(UINT task_number, UINT delay_time, INT run_count, void* user_data);
	virtual UINT vGetTaskCount() { return TaskList.NodeLinkCount; }
	virtual void vRunTask();
};

TaskComponent* TaskComponent::sCreate(TaskInterface* interface_link, UINT task_max)
{
	TaskManager* object = NULL;
	MY_NEW(object, TaskManager());
	object->Create(interface_link, task_max);
	return object;
}

void TaskManager::vRelease()
{
	TaskManager* _this = this;
	MY_DELETE(_this);
}

void TaskManager::Create(TaskInterface* interface_link, UINT task_max)
{
	pTaskInterface = interface_link;
	TaskList.Create(task_max);
}

TaskNode* TaskManager::vAddTask(UINT task_number, UINT delay_time, INT run_count, void* user_data)
{
	TDNode< TaskNode >* node = TaskList.GetNode();
	node->TaskNumber = task_number;
	node->DelayTime = delay_time;
	node->RunCount = run_count;
	node->pUserData = user_data;
	TaskList.Link(node);

	return node;
}

void TaskManager::vRunTask()
{
	TDNode< TaskNode >* node = TaskList.pHeadNode, * del_node = NULL;

	while (node)
	{
		if (Tool::sDelayTime(node->StartTime, node->EndTime, node->DelayTime))
		{
			if (pTaskInterface)
				pTaskInterface->vOnRunTask(node);

			if (node->RunCount != TASK_RUN_COUNT_UNLIMITED && --node->RunCount <= 0)
			{
				del_node = node;
				node = node->pNext;
				TaskList.Unlink(del_node);
				TaskList.FreeNode(del_node);
				continue;
			}
		}
		node = node->pNext;
	}
}