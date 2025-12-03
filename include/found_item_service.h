#ifndef FOUND_ITEM_SERVICE_H
#define FOUND_ITEM_SERVICE_H

#include <vector>
#include <optional>
#include <string>
#include "dto.h"

class FoundItemService {
private:
    FoundItemService() = default;
    
public:
    static FoundItemService& instance() {
        static FoundItemService instance;
        return instance;
    }
    
    std::optional<FoundItemDTO> create_found_item(const CreateFoundItemRequest& request, int user_id);
    std::vector<FoundItemDTO> get_found_items(int page = 1, int limit = 10,
                                            const std::optional<std::string>& category = std::nullopt,
                                            const std::optional<std::string>& keyword = std::nullopt,
                                            const std::optional<std::string>& status = std::nullopt);
    std::optional<FoundItemDTO> get_found_item_by_id(int id);
    bool update_found_item(int id, const UpdateFoundItemRequest& request, const UserDTO& user);
    bool delete_found_item(int id, const UserDTO& user);
    int get_total_found_items();
    int get_open_found_items_count();
    int get_found_items_7d_count();
};

#endif // FOUND_ITEM_SERVICE_H