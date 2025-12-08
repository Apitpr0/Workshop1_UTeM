#pragma once
#include <tuple>
#include <string>

// Returns {role, username}: role = 0=user, 1=admin, 2=runner, -1=failed
std::tuple<int, std::string> user_login();
std::tuple<int, std::string> admin_login();
std::tuple<int, std::string> runner_login();

