#pragma once

#ifndef MUSIC_H_
#define MUSIC_H_

#include <dpp/dpp.h>
#include <dpp/nlohmann/json.hpp>
#include <iostream>

#define YTDLP_CMD "../lib/yt-dlp/yt-dlp.sh"
#define FFMPEG_CMD "ffmpeg"

void handle_streaming(dpp::voiceconn* voiceConn, std::string& url);

#endif
