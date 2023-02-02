#pragma once

#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <map>
# include <set>
# include <vector>
# include <cstdlib>
# include <iostream>

using namespace std;

class Bot {
    private:
        set<string> _menuList;

        Bot(const Bot& src);
        Bot& operator=(const Bot& src);

    public:
        Bot(void);
        ~Bot();
        void addMenu(vector<string> params);
        void deleteMenu(vector<string> params);
        const string showMenu(void) const;
        const string pickMenu(void) const;
};

#endif
