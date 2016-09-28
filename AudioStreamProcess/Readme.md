# AudioStreamProcess

This project handles the encoding and streaming of audio from the rover. This is built as a standalone executable to isolate it from the main rover process should any gstreamer errors arise (which they do).

When the rover process receives a request to start an audio stream, it will execute this process and pass it the necessary configuration parameters as arguments. The rover process monitors the full lifecycle of the audio stream process, and will terminate it when the audio stream should be stopped.
