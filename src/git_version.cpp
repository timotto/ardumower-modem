#include "git_version.h"

#ifdef GIT_TAG
const char *git_hash = GIT_HASH;
const char *git_time = GIT_TIME;
const char *git_tag = GIT_TAG;
#else
const char *git_hash = "@GIT_HASH@";
const char *git_time = "@GIT_TIME@";
const char *git_tag = "@GIT_TAG@";
#endif
