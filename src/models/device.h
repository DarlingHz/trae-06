#pragma once

#include <string>
#include <ctime>
#include <stdexcept>
#include <vector>
#include <map>

class Device {
public:
    enum class Category {
        Phone,
        Laptop,
        Tablet,
        Other
    };

    int id = 0;
    int ownerUserId = 0;
    Category category = Category::Other;
    std::string brand;
    std::string model;
    std::string serialNumber;
    time_t purchaseDate = 0;
    time_t warrantyExpireAt = 0;
    time_t createdAt = 0;

    Device() = default;

    Device(int id, int ownerUserId, Category category, const std::string& brand,
           const std::string& model, const std::string& serialNumber, time_t purchaseDate,
           time_t warrantyExpireAt, time_t createdAt)
        : id(id), ownerUserId(ownerUserId), category(category), brand(brand),
          model(model), serialNumber(serialNumber), purchaseDate(purchaseDate),
          warrantyExpireAt(warrantyExpireAt), createdAt(createdAt) {}

    static Category categoryFromString(const std::string& str) {
        static const std::map<std::string, Category> categoryMap = {
            {"phone", Category::Phone},
            {"laptop", Category::Laptop},
            {"tablet", Category::Tablet},
            {"other", Category::Other}
        };
        auto it = categoryMap.find(str);
        if (it == categoryMap.end()) {
            throw std::runtime_error("Invalid device category: " + str);
        }
        return it->second;
    }

    static std::string categoryToString(Category category) {
        static const std::map<Category, std::string> categoryMap = {
            {Category::Phone, "phone"},
            {Category::Laptop, "laptop"},
            {Category::Tablet, "tablet"},
            {Category::Other, "other"}
        };
        auto it = categoryMap.find(category);
        if (it == categoryMap.end()) return "other";
        return it->second;
    }

    bool isValid() const {
        return ownerUserId > 0 && !brand.empty() && !model.empty() && !serialNumber.empty();
    }

    bool isUnderWarranty() const {
        time_t now = time(nullptr);
        return warrantyExpireAt > now;
    }
};
