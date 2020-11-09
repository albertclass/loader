#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <sstream>
#include <fstream>
#include <filesystem>

#include <process.h>
#include <string.h>
#include <nlohmann/json.hpp>
using namespace nlohmann;
namespace fs = std::filesystem;

int main(int argc, char **argv, char** envs);
int parseCommandLine(const char* cmd, char **args, int size)
{
	int argc = 0;
	auto cur = cmd, end = cmd + strlen(cmd);

	while (*cur && cur < end && argc < size) {
		while (*cur == ' ') ++cur;

		auto seg = strpbrk(cur, " \"'");

		if (seg == nullptr) {
			args[argc++] = _strdup(cur);
			seg = end;
		}
		else if (*seg == ' ') {
			auto arg = (char*)malloc(seg - cur + 1);
			strncpy(arg, cur, seg - cur);
			arg[seg - cur] = 0;
			args[argc++] = arg;
		}
		else if (*seg == '\"' || *seg == '\'') {
			cur = seg + 1;
			seg = strchr(cur, *seg);
			if (seg != nullptr) {
				auto arg = (char*)malloc(seg - cur + 1);
				strncpy(arg, cur, seg - cur);
				arg[seg - cur] = 0;
				args[argc++] = arg;
			}
		}

		cur = seg + 1;
	}

	return argc;
}

int WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	//_ASSERT(false);

	LPSTR environment_string = GetEnvironmentStrings();
	LPSTR environments[100] = { 0 };
	ULONG nIndex = 0;
	while (*environment_string) {
		environments[nIndex++] = environment_string;
		while (*environment_string++);
	};

	char *argv[100];
	auto argc = parseCommandLine(GetCommandLineA(), argv, _countof(argv));
	return main(argc, argv, environments);
}

int main(int argc, char **argv, char** envs) {
	fs::path execute(argv[0]);
	fs::path execute_directory = execute.parent_path();

	fs::path file = execute_directory / "msys2.conf";

	if (!fs::exists(file))
		return -1;

	if ((fs::status(file).permissions() & (fs::perms::owner_read | fs::perms::group_read | fs::perms::others_read)) == fs::perms::none)
		return -2;

	std::ifstream fs;
	fs.open(file);
	if (!fs.is_open())
		return -3;

	json conf = json::parse(fs, nullptr, false);
	std::stringstream ss;
	if (!conf.is_object())
		return -4;

	auto env = conf.value("env", json());
	char *_envs[256] = { 0 };
	long n = 0;
	if (env.is_object()) {
		for (auto item : env.items()) {
			auto key = item.key();
			auto val = item.value().get<std::string>();

			ss.str("");
			ss << key << "=" << val;
			_envs[n++] = _strdup(ss.str().c_str());
		}
	}

	long t = 0;
	while (envs[t]) {
		_envs[n++] = envs[t++];
	}

	_envs[n++] = NULL;

	auto cmd = conf.value("cmd", std::string());
	auto cwd = conf.value("cwd", std::string());
	auto args = conf.value("arguments", json::array());

	fs::path exepath(cmd);

	char *_args[256] = { 0 };

	n = 0;
	_args[n++] = _strdup(exepath.filename().string().c_str());

	for (auto &arg : args) {
		_args[n++] = _strdup(arg.get<std::string>().c_str());
	}

	_args[n++] = NULL;

	if (argc > 1 && fs::exists(argv[1]))
		fs::current_path(argv[1]);
	else if (fs::exists(cwd))
		fs::current_path(cwd);

	_execvpe(cmd.c_str(), _args, _envs);
}