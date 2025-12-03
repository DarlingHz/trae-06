#ifndef DASHBOARD_SERVICE_H
#define DASHBOARD_SERVICE_H

#include <vector>
#include <string>
#include <map>
#include "dto.h"

class DashboardService {
private:
    DashboardService() = default;
    
public:
    static DashboardService& instance() {
        static DashboardService instance;
        return instance;
    }
    
    StatData get_stat_data();
    std::vector<std::pair<std::string, int>> get_top_categories(int limit = 5);
};

#endif // DASHBOARD_SERVICE_H