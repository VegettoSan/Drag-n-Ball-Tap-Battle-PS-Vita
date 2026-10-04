// Standalone input probe only: TeaVM reserves up to 8 MiB plus GC metadata.
// This is the process Newlib heap, distinct from TeaVM's managed heap limit.
unsigned int _newlib_heap_size_user = 16u * 1024u * 1024u;
