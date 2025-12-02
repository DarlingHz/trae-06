#ifndef CONTROLLER_H
#define CONTROLLER_H

#include <memory>

class Controller {
public:
    /**
     * 启动控制器
     */
    virtual void start() = 0;
    
    /**
     * 停止控制器
     */
    virtual void stop() = 0;
    
    /**
     * 析构函数
     */
    virtual ~Controller() = default;
};

#endif // CONTROLLER_H