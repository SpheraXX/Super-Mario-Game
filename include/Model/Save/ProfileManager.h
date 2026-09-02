#ifndef MODEL_SAVE_PROFILEMANAGER_H
#define MODEL_SAVE_PROFILEMANAGER_H

#include <string>
#include <vector>

namespace model {

struct Profile {
    std::string name = "EMPTY";
    int total_score = 0;
    int passed_levels = 0;
    bool is_empty = true;
};

class ProfileManager {
public:
    static ProfileManager& instance();

    ProfileManager(const ProfileManager&) = delete;
    ProfileManager& operator=(const ProfileManager&) = delete;

    static constexpr const char* DefaultProfilePath = "assets/data/profiles.json";

    void load(const std::string& path = DefaultProfilePath);
    void save(const std::string& path = DefaultProfilePath) const;

    const std::vector<Profile>& getProfiles() const;
    void updateProfile(int index, const Profile& profile);
    void deleteProfile(int index);

    int getActiveProfileIndex() const;
    void setActiveProfileIndex(int index);

private:
    ProfileManager();
    std::vector<Profile> profiles;
    int activeProfileIndex = 0;
};

} // namespace model

#endif // MODEL_SAVE_PROFILEMANAGER_H
