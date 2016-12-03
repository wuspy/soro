# audio_streamer

This is a standalone executable which encodes and streams audio across a network. The main rover processes will execute audio_streamer as a child process for each stream to prevent any gstreamer errors from crashing the entire program.

### Requires

- libsoro
- libsorogst
