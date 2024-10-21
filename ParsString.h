
#include <iostream>
#include <algorithm>
#include <string>
#include <filesystem>
#include <vector>
#include <string>
using namespace std::filesystem;
typedef unsigned int uint;
using namespace std;
namespace MParsing
{
class ParsString
{
    private:
    std::string   _String;
    public:
    ParsString(void);
    ParsString(const char* str, uint SizeStr);
    ParsString(string& Str);
    string cut(uint BeginIndex, uint EndIndex);
    string replace(uint BeginIndex, uint EndIndex, const char* str, uint SizeStr);
    string replace(pair<uint, uint> PairIndex, string& Str);
    int LengthString();
    vector<uint>find(char Symbol);
    std::pair<uint, uint> find(const char* Findstr, uint SizeStr);
    std::pair<uint, uint> find(std::string& FindString);
    pair<uint, uint> find_next(pair<uint, uint> PairIndex, string& FindStr);
    void push_back(const char* str, uint SizeStr);
    void push_back(string& Str);
    string delete_symbol(uint IndexElem);
    string delete_range(pair<uint, uint> Range);
    string ShowString();
    ~ParsString(void);

};

}
