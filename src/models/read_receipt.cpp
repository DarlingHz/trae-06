#include "models/read_receipt.h"

namespace models {

ReadReceipt::ReadReceipt(int id, int announcement_id, int user_id, std::time_t read_at,
                         const std::optional<std::string>& client_ip,
                         const std::optional<std::string>& user_agent,
                         const std::optional<std::string>& extra_metadata)
    : id_(id), announcement_id_(announcement_id), user_id_(user_id), read_at_(read_at),
      client_ip_(client_ip), user_agent_(user_agent), extra_metadata_(extra_metadata) {
}

} // namespace models
