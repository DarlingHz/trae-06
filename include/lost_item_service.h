#ifndef LOST_ITEM_SERVICE_H
#define LOST_ITEM_SERVICE_H

#include <vector>
#include <optional>
#include <string>
#include "dto.h"

class LostItemService {
private:
    LostItemService() = default;
    
public:
    static LostItemService& instance() {
        static LostItemService instance;
        return instance;
    }
    
    std::optional<LostItemDTO> create_lost_item(const CreateLostItemRequest& request, int user_id);
    std::vector<LostItemDTO> get_lost_items(int page = 1, int limit = 10,
                                            const std::optional<std::string>& category = std::nullopt,
                                            const std::optional<std::string>& keyword = std::nullopt,
                                            const std::optional<std::string>& status = std::nullopt);
    std::optional<LostItemDTO> get_lost_item_by_id(int id);
    bool update_lost_item(int id, const UpdateLostItemRequest& request, const UserDTO& user);
    bool delete_lost_item(int id, const UserDTO& user);
    int get_total_lost_items();
    int get_open_lost_items_count();
    int get_lost_items_7d_count();
};

#endif // LOST_ITEM_SERVICE_H