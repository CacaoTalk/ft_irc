#pragma once

#ifndef BOT_HPP
# define BOT_HPP

# include <string>
# include <map>
# include <set>
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
        void addMenu(const string& menu);
        void delteMenu(const string& menu);
        const string showMenu(void) const;
        const string pickMenu(const set<string> menu) const;
};

#endif
