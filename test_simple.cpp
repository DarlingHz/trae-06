#include "statistics_service.h"
#include <memory>

int main() {
    auto service = std::make_shared<recruitment::StatisticsServiceImpl>();
    return 0;
}