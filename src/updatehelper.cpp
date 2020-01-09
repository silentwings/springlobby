/* This file is part of the Springlobby (GPL v2 or later), see COPYING */

#include "updatehelper.h"

#include <time.h>
#include <cerrno>

#include "httpfile.h"
#include "utils/conversion.h"
#include "utils/slconfig.h"
#include "utils/version.h"

SLCONFIG("/General/LastUpdateCheck", 0L, "Last time springlobby checked for an update");
SLCONFIG("/General/UpdateChannel", "release", "update channel to use (release or develop)");
#define VERSION_CHECK_INTERVAL 1 * 60 * 60
#define BASE_URL "https://springlobby.springrts.com/dl/"

static bool isReleaseChannel()
{
	const wxString channel = cfg().ReadString(_T("/General/UpdateChannel"));
	return channel != _T("develop"); //typos will make use release channel
}

std::string GetDownloadUrl(const std::string& version)
{
	if (isReleaseChannel()) {
		return stdprintf( BASE_URL "stable/springlobby-%s-win32.zip", version.c_str());
	}
	return stdprintf( BASE_URL "develop/springlobby-%s-win32.zip", version.c_str());
}

std::string GetLatestVersionUrl()
{
	if (isReleaseChannel()) {
		return std::string(BASE_URL "stable/version.txt");
	}
	return std::string(BASE_URL "develop/version-develop.txt");
}

static time_t GetTime()
{
	time_t now = time(nullptr);
	if (static_cast<time_t>(-1) == now) {
		std::string msg = "time() broke: ";
		msg += strerror(errno);
		throw std::runtime_error(msg.c_str());
	}
	return now;
}

std::string GetLatestVersion(bool use_cached)
{
	static std::string latest_version = GetSpringlobbyVersion();
	const time_t now = GetTime();
	const time_t last_check_time = cfg().ReadLong(_T("/General/LastUpdateCheck"));

	if (latest_version.empty() || !use_cached || now > (last_check_time + VERSION_CHECK_INTERVAL)) {
		latest_version = GetHttpFile(GetLatestVersionUrl());
		latest_version = STD_STRING(TowxString(latest_version).Trim().Trim(false));
		cfg().Write(_T("/General/LastUpdateCheck"), (long)now);
	}
	return latest_version;
}
