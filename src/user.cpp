#include <marong/user.h>

// Defines and register a new user template
void newUser(nlohmann::json &users, const std::string &userid, std::string username)
{

    // Check if User already exist in user config file
    if (users.contains(userid))
        return;

    // Create new user template
    nlohmann::json userTemplate = {
        {"status", {
            {"member", true},
            {"blacklisted", false},
        }},
        {"restricted", nlohmann::json::array()},
        {"personalConfig", {
            {"music", {
                {"personal-music-channel", false},
                {"music-channel", {
                    {"channelid", 0},
                    {"menubackground-url", "https://i.pinimg.com/originals/b8/2f/28/b82f28a7e9c8fcb3868d3d94652c107c.gif"},
                    {"menutitle", username + "的私人点歌频道"},
                    {"colors", {
                        {"menu", "#00FFFF"},
                        {"message", "#00FFFF"}
                    }},
                }},
                {"favorite-genres", nlohmann::json::array()},
                {"playlists", {
                    {"favorite", nlohmann::json::array()}
                }},
            }},
            {"voice-channel", {
                {"supertitle", "🏠｜﹒" + username + "的小屋"}
            }},
        }}
    };

    // Store the new user template inside users with userid as the key
    users[userid] = userTemplate;
}

// Return true if user's status is still a member registered in users, else false
bool status_is_member(nlohmann::json &users, std::string &userid){
    return users[userid]["status"]["member"];
}

// Change user's status.member to true if false, to false if true
void toggle_member_status(nlohmann::json &users, std::string &userid){
    users[userid]["status"]["member"] = !users[userid]["status"]["member"];
}

// Return true if user is blacklisted, else false
bool status_blacklisted(nlohmann::json &users, std::string &userid){
    return users[userid]["status"]["blacklisted"];
}

// Change user's status.blacklist to true if false, to false if true
void toggle_blacklist(nlohmann::json &users, std::string &userid){
    users[userid]["status"]["blacklisted"] = !users[userid]["status"]["blacklisted"];
}

// Returns all the channelID of user's restricted channels
std::vector<dpp::snowflake> whats_restricted(nlohmann::json &users, std::string &userid){
    return users[userid]["restricted"];
}

// Return true if user have that channel in their restricted channels
bool have_restricted(nlohmann::json &users, std::string &userid, dpp::snowflake channelID){

    // Loop through all channel in restricted channels
    for (const dpp::snowflake &id : whats_restricted(users, userid)){
        // if channel id is the one you're looking for
        if (id == channelID){
            // Found the channel in restricted channels
            return true;
        }
    }
    // Can't find channel in restricted channels
    return false;
}

// Register a new channel to user's restricted channels
void add_restricted(nlohmann::json &users, std::string &userid, dpp::snowflake channelID){
    if (!have_restricted(users, userid, channelID)){
        users[userid]["restricted"].push_back(channelID);
    }
}

// Remove a channel from users's restricted channels
void remove_restricted(nlohmann::json &users, std::string &userid, dpp::snowflake channelID){
    if (have_restricted(users, userid, channelID))
        users[userid]["restricted"].erase(channelID);
}

// Return true if user has a music_channel, else false
bool has_personal_music_channel(nlohmann::json &users, std::string &userid){
    return users[userid]["personalConfig"]["music"]["personal_music_channel"];
}

//Remove the personal music channel
void remove_personal_music_channel(nlohmann::json& users, std::string& userid){
    if (!has_personal_music_channel(users, userid))
        throw std::runtime_error("Error: The personal music channel does not exists\n");
    users[userid]["personalConfig"]["music"]["personal_music_channel"] = false;
    
}

//Reset the personal music channel
void reset_personal_music_channel(nlohmann::json& users, std::string& userid){

}


// Change user's personal-music-channel to true if false, to false if true
void toggle_personal_music_channel(nlohmann::json &users, std::string &userid){
    users[userid]["personalConfig"]["music"]["personal_music_channel"] = !users[userid]["personalConfig"]["music"]["personal_music_channel"];
}

// Return the user's personal music channel id, return 0 means user didn't setup a music channel
dpp::snowflake get_music_channel_id(nlohmann::json &users, std::string &userid){
    if (users[userid]["personalConfig"]["music"]["music-channel"].is_null())
        return 0;
    return users[userid]["personalConfig"]["music"]["music-channel"]["channelid"];
}

// Set a value to user's personal music channel id
void set_music_channel_id(nlohmann::json &users, std::string &userid, dpp::snowflake channelID){
    users[userid]["personalConfig"]["music"]["music-channel"]["channelid"] = channelID;
}

// Return the user's personal music channel menu title
std::string get_menu_title(nlohmann::json &users, std::string &userid){
    return users[userid]["personalConfig"]["music"]["music-channel"]["menutitle"];
}

// Set a new title to the user's personal music channel menu title
std::string set_menu_title(nlohmann::json &users, std::string &userid, std::string &title){
    users[userid]["personalConfig"]["music"]["music-channel"]["menutitle"] = title;
}

// Return a pair of user's preference colors for their personal music channel
std::pair<std::string, std::string> get_user_colors(nlohmann::json &users, std::string &userid){
    return users[userid]["personalConfig"]["music"]["music-channel"]["colors"];
}

// Set a new color to the user's personal music channel menu color
void set_menu_color(nlohmann::json &users, std::string &userid, std::string &color){
    users[userid]["personalConfig"]["music"]["music-channel"]["colors"]["menu"] = color;
}

// Set a new color to the user's personal music channel messages color
void set_message_color(nlohmann::json &users, std::string &userid, std::string &color){
    users[userid]["personalConfig"]["music"]["music-channel"]["colors"]["message"] = color;
}

// Return a vector of user's favorite music genre if they have any
std::vector<std::string> get_favorite_genre(nlohmann::json &users, std::string &userid){
    return users[userid]["personalConfig"]["music"]["favorite-genres"];
}

// Add a genere to user's favorite genere.
void add_favorite_genere(nlohmann::json &users, std::string &userid, std::vector<std::string>& genres){    
    if (users[userid]["personalConfig"]["music"]["favorite-genres"].size() >= 100){
        users[userid]["personalConfig"]["music"]["favorite-genres"].erase(users[userid]["personalConfig"]["music"]["favorite-genres"].begin());
    }
    users[userid]["personalConfig"]["music"]["favorite-genres"].push_back(genres);
}

// Remove a genere from user's favorite genere
void remove_favorite_genere(nlohmann::json &users, std::string &userid, std::string &genre){
    users[userid]["personalConfig"]["music"]["favorite-genres"].erase(genre);
}

// Return all the playlists name, at least will return favorite
std::vector<std::string> get_all_playlists(nlohmann::json &users, std::string &userid){
    return users[userid]["personalConfig"]["music"]["playlists"];
}

// Create a new playlist in user's playlist, user can't add playlist that have the same name as their default playlist "favorite"
void create_playlist(nlohmann::json &users, std::string &userid, std::string &playlist){   
    //if the playlist name is equal to "favorite", print the error message.
    if (playlist == "favorite")
    {
        std::cerr << "Error: You can't have the same name as the defaulst favorite playerlist\n";
        exit(1);
    }
    //Check if the player name is already exists.
    auto& userPlaylists = users[userid]["personalConfig"]["music"]["playlists"];
    for (auto i = userPlaylists.begin(); i != userPlaylists.end(); ++i) {
        // if the player name is already exists, print the error message.
        if (*i == playlist) {
            std::cerr << "Error: playlist name already exists\n";
            exit(1);
        }
    }
    //otherwise, add the playlist name into the playlists.
    users[userid]["personalConfig"]["music"]["playlists"].push_back(playlist);
}

// Remove a playlist in user's playlist, user can't remove their default playlist "favorite"
void remove_playlist(nlohmann::json &users, std::string &userid, std::string &playlist) {

    // Check if the playlists have the playlist name
    auto& userPlaylists = users[userid]["personalConfig"]["music"]["playlists"];
    for (auto i = userPlaylists.begin(); i != userPlaylists.end(); ++i) {
        // if the playerlist name exist, remove it from the playlists.
        if (*i == playlist) {
            userPlaylists.erase(i);
            exit(1);
        }
    }
    //print the error message if the playlist name is not found.
    std::cerr << "Error: Playlist '" << playlist << "' not found\n";
}

// Rename a playlist in user's playlist, user can't rename their default playlist "favorite"
void rename_playlist(nlohmann::json &users, std::string &userid, std::string &playlist, std::string &new_playlist) {
    //if the new playlist name equal to "favorite",  print the error message.
    if (new_playlist == "favorite"){
        std::cerr << "Error: You can not have the same name as the defaulst favorite playerlist\n";
        exit(1);
    }
    // Check if the playlist already have the playlist name
    auto& userPlaylists = users[userid]["personalConfig"]["music"]["playlists"];
    for (auto i = userPlaylists.begin(); i != userPlaylists.end(); ++i) {
        if (*i == playlist) {
            std::cerr << "Error: The playlist name '" << new_playlist << "' already exists. Please choose another name.\n";
            exit(1);
        }
    }
    //otherwise, rename the palylist name to a new playlist name.
    users[userid]["personalConfig"]["music"]["playlists"][playlist] = new_playlist;
}

// Check if user's playlist have this playlist.
bool has_playlists(nlohmann::json &users, std::string &userid, std::string &playlist){
    auto& userPlaylists = users[userid]["personalConfig"]["music"]["playlists"];
    for (auto i = userPlaylists.begin(); i != userPlaylists.end(); ++i) {
        if (*i == playlist)
            return true;
    }
    return false;
}

// Return all the song from this playlist
std::vector<std::string> get_playlist_content(nlohmann::json &users, std::string &userid, std::string &playlist) {
    if (!has_playlists(users, userid, playlist))
    {
        std::cerr << "playerlist doens't exists\n";
        exit(1);
    }
    return users[userid]["personalConfig"]["music"]["playlists"][playlist];
}

// Check if playlist has the song
bool playlist_has_song(nlohmann::json &users, std::string &userid, std::string &playlist, std::string &song) {
    // Check if the playlist has the song in it
    auto& specific_playlist = users[userid]["personalConfig"]["music"]["playlists"][playlist];
    for (auto i = specific_playlist.begin(); i != specific_playlist.end(); ++i) {
        if (*i == song)
            return true;
        return false;
    }
}


// Add a new song to this playlist
void playlist_add_song(nlohmann::json &users, std::string &userid, std::string &playlist, std::string &song) {
    // if the playerlist already has the song, print the error message.
    if (playlist_has_song(users, userid, playlist, song)){
        std::cerr << "This sone is already exists in '" << playlist << "'\n";
        exit(1);
    }
    //else, add the song into the playlist
    users[userid]["personalConfig"]["music"]["playlists"][playlist].push_back(song);
}

// Remove a song to player's playlist
void playlist_remove_song(nlohmann::json &users, std::string &userid, std::string &playlist, std::string &song) {
    users[userid]["personalConfig"]["music"]["playlists"][playlist].erase(song);
}

// Return the user's personal music channel's menu background image URL
std::string get_backgroundURL(nlohmann::json &users, std::string &userid) {
    return users[userid]["personalConfig"]["music"]["music-channel"]["menubackground-url"];
}

// Set a new URL to the user's personal music channel's menu background image URL
void set_backgroundURL(nlohmann::json &users, std::string &userid, std::string &backgroundURL){
    users[userid]["personalConfig"]["music"]["music-channel"]["menubackground-url"] = backgroundURL;
}

// Get user's supertitle (Value can't exeed 100 characters)
std::string get_supertitle(nlohmann::json &users, const std::string &userid){
    return users[userid]["personalConfig"]["voice-channel"]["supertitle"].get<std::string>();
}
