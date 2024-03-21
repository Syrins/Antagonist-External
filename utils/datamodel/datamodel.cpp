#include "datamodel.hpp"
#include "../xorstr/xorstr.hpp"

std::uint64_t antagonist::utils::datamodel::get_game_address()
{
    auto latest_log = antagonist::utils::log::get_latest_log();

    std::ifstream rbx_log(latest_log);
    std::string rbx_log_line;
    std::vector<std::string> lines;

    while (std::getline(rbx_log, rbx_log_line))
    {
        lines.push_back(rbx_log_line);
    }

    for (auto it = lines.rbegin(); it != lines.rend(); ++it)
    {
        if (it->find("start dataModel(") != std::string::npos)
        {
            std::string rbx_log_line = *it;
            rbx_log_line = rbx_log_line.substr(rbx_log_line.find("start dataModel(") + 16);
            rbx_log_line = rbx_log_line.substr(0, rbx_log_line.find(')'));

            std::uint64_t game = std::strtoull(rbx_log_line.c_str(), nullptr, 16);
            return game + 0x148;
        }
    }
}

std::uint64_t antagonist::utils::datamodel::get_place_id()
{
    auto latest_log = antagonist::utils::log::get_latest_log();

    std::ifstream rbx_log(latest_log);
    std::stringstream log_content;
    log_content << rbx_log.rdbuf();

    std::string rbx_log_data = log_content.str();

    std::regex placeIdRegex(R"("placeId":(\d+),)");

    std::smatch match;
    if (std::regex_search(rbx_log_data, match, placeIdRegex))
    {
        if (match.size() > 1)
        {
            std::uint64_t placeId = std::stoull(match[1].str());
            return placeId;
        }
    }
}