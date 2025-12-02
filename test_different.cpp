#include "statistics_service.h"
#include <memory>

int main() {
    auto service = std::shared_ptr<recruitment::StatisticsServiceImpl>(new recruitment::StatisticsServiceImpl());
    return 0;
}