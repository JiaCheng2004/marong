#include <marong/music.h>

void handle_streaming(dpp::voiceconn* voiceConn, std::string& url) {
  std::string command = YTDLP_CMD " -f 251 -o - " + url +
                        " | " + FFMPEG_CMD +
                        " -acodec opus -i - -f s16le -ar 48000 -ac 2 -";

  FILE *read_stream = popen(command.c_str(), "r");


  // should always be this size when using send_audio_raw to avoid
  // distortion
  constexpr size_t bufsize = dpp::send_audio_raw_max_length;

  char buf[bufsize];
  ssize_t buf_read = 0;
  ssize_t current_read = 0;

  while ((current_read = fread(buf + buf_read, 1, bufsize - buf_read, read_stream)) > 0) {
    buf_read += current_read;

    // queue buffer only when it's exactly `bufsize` size
    if (buf_read == bufsize) {
      voiceConn->voiceclient->send_audio_raw((uint16_t *)buf, buf_read);
      buf_read = 0;
    }
  }

  // queue the last buffer if any, usually size less than `bufsize`
  if (buf_read > 0) {
    voiceConn->voiceclient->send_audio_raw((uint16_t *)buf, buf_read);
    buf_read = 0;
  }

  // done processing all stream buffer, clean up
  // "done" here means done queueing audio buffer
  // not done streaming the queued audio data to discord
  pclose(read_stream);
  read_stream = NULL;

  // wait until voice client done streaming all queued buffer
  while (voiceConn->voiceclient->is_playing()) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }

  // insert marker to disconnect
  // this will fire a voice_track_marker event
  // you can also insert any metadata you need for
  // other purpose eg. play next song in your playlist
  voiceConn->voiceclient->insert_marker("disconnect");
}
