#ifndef _BUILD_INFO_PATTERN
#define _BUILD_INFO_PATTERN

#include <string>

extern const uint32_t _buildTime{ BUILD_TIME };
extern const uint32_t _buildNumber{ BUILD_NUMBER };
extern const uint32_t _gitTime{ GIT_TIME };
extern const std::string _buildCommit{ "BUILD_COMMIT" };
extern const std::string _gitCommit{ "GIT_COMMIT" };

#endif // _BUILD_INFO_PATTERN
