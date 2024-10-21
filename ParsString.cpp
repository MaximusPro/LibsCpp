
#include "ParsString.h"
using MParsing::ParsString;

ParsString::ParsString(void){}

ParsString::ParsString(const char* str, uint SizeStr)
{
    string Str(str, str+SizeStr);
    _String += Str;
}
ParsString::ParsString(string& Str){_String += Str;}

string ParsString::cut(uint BeginIndex, uint EndIndex)
{
    string Resault = "";
    if((_String.length() > (EndIndex-BeginIndex)) and (_String.length() > EndIndex && _String.length() > BeginIndex)
    and (BeginIndex < EndIndex))
    {
        for(int i = BeginIndex; i <= EndIndex; i++)
        {
            Resault += _String[i];
        }
    }
    return Resault;
}

string ParsString::replace(uint BeginIndex, uint EndIndex, const char* str, uint SizeStr)
{
    string Resault = _String;
    if((_String.length() > (EndIndex-BeginIndex)) and (_String.length() > EndIndex && _String.length() > BeginIndex)
    and (BeginIndex < EndIndex))
    {
        string Str(str, str+SizeStr);
        Resault.replace(BeginIndex, (EndIndex-BeginIndex+1), Str);
        _String = Resault;
    }
    return Resault;
}

string ParsString::replace(pair<uint, uint> PairIndex, string& Str)
{
    string Resault = _String;
    if((_String.length() > (PairIndex.second-PairIndex.first)) and (_String.length() > PairIndex.second && _String.length() > PairIndex.first)
    and (PairIndex.first < PairIndex.second))
    {
        Resault.replace(PairIndex.first, (PairIndex.second-PairIndex.first+1), Str);
        _String = Resault;
    }
    return Resault;
}
int ParsString::LengthString(){return _String.length();}
vector<uint> ParsString::find(char Symbol)
{
    vector<uint> AllFindedSymbols;
    for(int i = 0; i < _String.length(); i++)
    {
        string StrSymbol = ""; 
        StrSymbol += Symbol;
        string Symb = "";
        Symb += _String[i];
        if(Symb == StrSymbol)
        AllFindedSymbols.push_back(i);
    }
    return AllFindedSymbols;
}

pair<uint, uint> ParsString::find(const char* Findstr, uint SizeStr)
{
    pair<uint, uint> PairIndex;
    string FindStr(Findstr, Findstr+SizeStr);
    if(FindStr.length() < _String.length())
    {
        vector<char> VectorFindStr;
        for(char S:_String)
        VectorFindStr.push_back(S);
        vector<char> ::iterator it;
        it = find_end(VectorFindStr.begin(), VectorFindStr.end(), Findstr, Findstr+SizeStr);
        if(it!=VectorFindStr.end())
        {
            PairIndex.first = it-VectorFindStr.begin();
            PairIndex.second = PairIndex.first+SizeStr-1;
        }
    }
    return PairIndex;
}

pair<uint, uint> ParsString::find(std::string& FindString)
{
    pair<uint, uint> PairIndex;
    if(FindString.length() < _String.length())
    {
        vector<char> VectorFindStr;
        for(char S:_String)
        VectorFindStr.push_back(S);
        vector<char> ::iterator it;
        it = find_end(VectorFindStr.begin(), VectorFindStr.end(), FindString.c_str(), FindString.c_str()+FindString.length());
        if(it!=VectorFindStr.end())
        {
            PairIndex.first = it-VectorFindStr.begin();
            PairIndex.second = PairIndex.first+FindString.length()-1;
        }
    }
    return PairIndex;
}

pair<uint, uint> ParsString::find_next(pair<uint, uint> PairIndex, string& FindString)
{
    pair<uint, uint> ResaultIndex;
    int Length = _String.length();
    if(FindString.length() < _String.length() && ((PairIndex.second+1) < _String.length()))
    {
        vector<char> VectorFindStr;
        for(char S:_String)
        VectorFindStr.push_back(S);
        vector<char> ::iterator it;
        it = find_end(VectorFindStr.begin()+PairIndex.first, VectorFindStr.begin()+PairIndex.second, FindString.c_str(), FindString.c_str()+FindString.length());
        if(it!=VectorFindStr.begin()+PairIndex.second)
        {
         
            ResaultIndex.first = it-VectorFindStr.begin();
            ResaultIndex.second = ResaultIndex.first+FindString.length()-1;
        }
    }
    return ResaultIndex;
}

void ParsString::push_back(const char* str, uint SizeStr)
{
    string Str(str, str+SizeStr);
    _String += Str;
}

void ParsString::push_back(string& Str){_String += Str;}

std::string ParsString::delete_symbol(uint IndexElem)
{
    string ResString = "";
    if(IndexElem <= _String.length())
    {
    vector<char> VecString;
    for(char S: _String)
    {VecString.push_back(S);}
    VecString.erase(VecString.begin()+IndexElem);
    for(char S: VecString)
    ResString += S;
    _String = ResString;
    }
    return ResString;
}

std::string ParsString::delete_range(pair<uint, uint> Range)
{
    string ResString = "";
    if(Range.second <= _String.length() and (Range.second-Range.first) < _String.length())
    {
    vector<char> VecString;
    for(char S: _String)
    {VecString.push_back(S);}
    VecString.erase(VecString.begin()+Range.first, VecString.begin()+Range.second+1);
    for(char S: VecString)
    ResString += S;
    _String = ResString;
    }
    return ResString;
}

std::string ParsString::ShowString(){return _String;}

ParsString::~ParsString(void){}
