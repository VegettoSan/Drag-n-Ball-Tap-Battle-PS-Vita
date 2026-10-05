// Process Newlib heap for the full AOT engine build.
// TeaVM is generated with an 8 MiB minimum / 48 MiB maximum managed heap and
// eagerly reserves its max heap plus GC metadata on Vita. The remaining space
// covers decoded resources, audio, C++ containers and platform services.
unsigned int _newlib_heap_size_user = 96u * 1024u * 1024u;
