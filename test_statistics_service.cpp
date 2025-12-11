#include "statistics_service.h"

int main() {
    auto service = std::make_shared<recruitment::StatisticsServiceImpl>();
    return 0;
}