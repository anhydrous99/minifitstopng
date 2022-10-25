//
// Created by sao on 10/25/22.
//

#include "stk.h"

#include <filesystem>
#include <sstream>
#include <fstream>
#include <glob.h>

std::vector<std::string> split(const std::string& s, char delim) {
    std::vector<std::string> result;
    std::stringstream ss (s);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back (item);
    }

    return result;
}

std::vector<std::string> get_file(const std::string& file_path) {
    std::ifstream stream(file_path);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line))
        lines.emplace_back(std::move(line));
    stream.close();
    return lines;
}

std::string filter(const std::string& file_path, size_t init_pos) {
    auto pos = file_path.find('[');
    return file_path.substr(init_pos, pos);
}

std::vector<std::string> handle_glob(const std::string& pattern) {
    glob_t glob_result;
    memset(&glob_result, 0, sizeof(glob_result));

    // do the glob operation
    int return_value = glob(pattern.c_str(), GLOB_NOSORT | GLOB_TILDE, nullptr, &glob_result);

    if (return_value == GLOB_NOMATCH) {
        globfree(&glob_result);
        return {};
    }

    if (return_value != 0) {
        globfree(&glob_result);
        std::stringstream stream;
        stream << "glob() failed with return_value " << return_value << std::endl;
        throw std::runtime_error(stream.str());
    }

    // Collect all the filenames into a std::vector<std::string>
    std::vector<std::string> filenames;
    for (size_t i = 0; i < glob_result.gl_pathc; ++i)
        filenames.emplace_back(glob_result.gl_pathv[i]);

    // cleanup
    globfree(&glob_result);

    return filenames;
}

std::vector<std::string> stack_build(const std::string& list) {
    std::vector<std::string> lists = split(list, ' ');
    std::vector<std::string> output;
    auto size = lists.size();
    for (std::vector<std::string>::size_type i = 0; i < size; i++) {
        std::string item = lists[i];
        bool has_star = item.find('*') != std::string::npos;
        if (item[0] == '@') {
            bool recurse = item[1] == '+';
            bool remove_path = item[1] == '-';

            size_t init_pos = 1;
            if (recurse || remove_path)
                init_pos++;
            item = filter(item, init_pos);

            auto lines = get_file(item);

            if (recurse) {
                for (auto const& line : lines) {
                    if (line[0] == '@') {
                        lists.push_back(line);
                        size++;
                    }
                }
            }

            if (remove_path) {
                std::for_each(lines.begin(), lines.end(), [](const std::string& line) {
                    size_t pos = line.rfind(std::filesystem::path::preferred_separator);
                    return line.substr(pos + 1);
                });
            }

            output.insert(output.end(), std::make_move_iterator(lines.begin()), std::make_move_iterator(lines.end()));
        } else if (has_star) {
            auto glob_ret = handle_glob(item);
            output.insert(output.end(), std::make_move_iterator(glob_ret.begin()), std::make_move_iterator(glob_ret.end()));
        } else
            output.push_back(item);
    }

    return output;
}
