#pragma once

#include "task_factory.h"

namespace job_service {

/**
 * @brief 注册示例任务类型到任务工厂
 * 
 * 注册以下任务类型：
 * - "fib": 斐波那契数列计算任务
 * - "word_count": 单词计数任务
 * 
 * @param factory 任务工厂指针
 */
void register_example_tasks(std::shared_ptr<TaskFactory> factory);

} // namespace job_service
