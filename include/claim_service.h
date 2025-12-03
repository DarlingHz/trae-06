#ifndef CLAIM_SERVICE_H
#define CLAIM_SERVICE_H

#include <vector>
#include <optional>
#include <string>
#include "dto.h"

class ClaimService {
private:
    ClaimService() = default;
    
public:
    static ClaimService& instance() {
        static ClaimService instance;
        return instance;
    }
    
    std::optional<ClaimDTO> create_claim(const CreateClaimRequest& request, int user_id);
    std::vector<ClaimDTO> get_claims(int user_id, const std::optional<std::string>& status = std::nullopt);
    std::optional<ClaimDTO> get_claim_by_id(int id);
    bool approve_claim(int id, const UserDTO& admin_user);
    bool reject_claim(int id, const UserDTO& admin_user);
    bool is_claim_possible(int lost_item_id, int found_item_id);
    int get_claims_7d_count();
};

#endif // CLAIM_SERVICE_H