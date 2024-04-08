#ifndef USER_H_
#define USER_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <marong/helper.h>
#include <iostream>

extern std::string users_address;

// Defines and register a new user template
void newUser(nlohmann::json& users, const std::string& userid, std::string username);

// Return true if user's status is still a member registered in users, else false
bool status_is_member(nlohmann::json& users, std::string& userid);

// Change user's status.member to true if false, to false if true
void toggle_member_status(nlohmann::json& users, std::string& userid);

// Return true if user is blacklisted, else false
bool status_blacklisted(nlohmann::json& users, std::string& userid);

// Change user's status.blacklist to true if false, to false if true
void toggle_blacklist(nlohmann::json& users, std::string& userid);

// Returns all the channelID of user's restricted channels
std::vector<dpp::snowflake> whats_restricted(nlohmann::json& users, std::string& userid);

// Return true if user have that channel in their restricted channels
bool have_restricted(nlohmann::json& users, std::string& userid, dpp::snowflake channelID);

// Register a new channel to user's restricted channels
void add_restricted(nlohmann::json& users, std::string& userid, dpp::snowflake channelID);

// Remove a channel from users's restricted channels
void remove_restricted(nlohmann::json& users, std::string& userid, dpp::snowflake channelID);

// Return true if user has a personal music channel, else false
bool has_personal_music_channel(nlohmann::json& users, std::string& userid);

//Remove the personal music channel if there have one.
void remove_personal_music_channel(nlohmann::json& users, std::string& userid);

//Reset the personal music channel to default if there have one.
void reset_personal_music_channel(nlohmann::json& users, std::string& userid);

// Change user's personal-music-channel to true if false, to false if true
void toggle_personal_music_channel(nlohmann::json& users, std::string& userid);

// Return the user's personal music channel id, return 0 means user didn't setup a music channel
dpp::snowflake get_music_channel_id(nlohmann::json& users, std::string& userid);

// Set a value to user's personal music channel id
void set_music_channel_id(nlohmann::json& users, std::string& userid, dpp::snowflake channelID);

// Return the user's personal music channel menu title
std::string get_menu_title(nlohmann::json& users, std::string& userid);

// Set a new title to the user's personal music channel menu title
std::string set_menu_title(nlohmann::json& users, std::string& userid, std::string& title);

// Return a pair of user's preference colors for their personal music channel
std::pair<std::string, std::string> get_user_colors(nlohmann::json& users, std::string& userid);

// Set a new color to the user's personal music channel menu color
void set_menu_color(nlohmann::json& users, std::string& userid, std::string& color);

// Set a new color to the user's personal music channel messages color
void set_message_color(nlohmann::json& users, std::string& userid, std::string& color);

// Return a vector of user's favorite music genre if they have any
std::vector<std::string> get_favorite_genre(nlohmann::json& users, std::string& userid);

// Add a genere to user's favorite genere.
void add_favorite_genere(nlohmann::json &users, std::string &userid, std::vector<std::string>& genres);

// Remove a genere from user's favorite genere
void remove_favorite_genere(nlohmann::json& users, std::string& userid, std::string& genre);

// Return all the playlists name, at least will return favorite
std::vector<std::string> get_all_playlists(nlohmann::json& users, std::string& userid);

// Create a new playlist in user's playlist, user can't add playlist that have the same name as their default playlist "favorite"
void create_playlist(nlohmann::json& users, std::string& userid, std::string& playlist);

// Remove a playlist in user's playlist, user can't remove their default playlist "favorite"
void remove_playlist(nlohmann::json& users, std::string& userid, std::string& playlist);

// Rename a playlist in user's playlist, user can't rename their default playlist "favorite"
void rename_playlist(nlohmann::json& users, std::string& userid, std::string& playlist, std::string& new_playlist);

// Check if user's playlist have this playlist.
bool has_playlists(nlohmann::json& users, std::string& userid, std::string& playlist);

// Return all the song from this playlist
std::vector<std::string> get_playlist_content(nlohmann::json& users, std::string& userid, std::string& playlist);

// Check if playlist has the song
bool playlist_has_song(nlohmann::json& users, std::string& userid, std::string& playlist, std::string& song);

// Add a new song to this playlist
void playlist_add_song(nlohmann::json& users, std::string& userid, std::string& playlist, std::string& song);

// Remove a new song to player's playlist
void playlist_remove_song(nlohmann::json& users, std::string& userid, std::string& playlist, std::string& song);

// Return the user's personal music channel's menu background image URL
std::string get_backgroundURL(nlohmann::json& users, std::string& userid);

// Set a new URL to the user's personal music channel's menu background image URL
void set_backgroundURL(nlohmann::json& users, std::string& userid, std::string& backgroundURL);

// Get user's supertitle (Value can't exeed 100 characters)
std::string get_supertitle(nlohmann::json users, const std::string userid); 

// Set new value to the user's supertitle value (Value can't exeed 100 characters)
void set_supertitle(nlohmann::json& users, std::string userid, std::string& superTitle);

#endif /* USER_H_ */