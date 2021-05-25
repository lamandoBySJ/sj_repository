#ifndef _STRING_HELPER_H
#define _STRING_HELPER_H
#include <Arduino.h>
#include <iostream>
#include <regex>
#include <vector>
#include <string>
#include "platform_debug.h"
using namespace std;
class StringHelper
{
public:
    StringHelper(){

    }

    //没有使用C++11特性
    /*static vector<string> split(string srcStr, const string& delim)
    {
        int nPos = 0;
        vector<string> vec;
        nPos = srcStr.find(delim.c_str());
        while(-1 != nPos)
        {
            string temp = srcStr.substr(0, nPos);
            vec.push_back(temp);
            srcStr = srcStr.substr(nPos+1);
            nPos = srcStr.find(delim.c_str());
        }
        vec.push_back(srcStr);
        return vec;
    }
    void split2(vector<String>& out,const string& in, const string& delim)
    {
        try
        { 
            regex re{delim};

            for (auto it = sregex_token_iterator(in.begin(), in.end(), re, -1) ; it != sregex_token_iterator(); ++it)
            {
               // cout << "TOKEN: " << (string) *it << endl;
                out.push_back(it->str().c_str());
            }
                
        }
        catch(const std::exception& e)
        {
            //cout<<"error:"<<e.what()<<std::endl;
            PlatformDebug::println(e.what());
        }

    }*/
    //使用C++11特性
   static void split(vector<String>& out,const string& in, const string& delim)
    {
        try
        { 
            out.clear();
            regex re{delim};
            for (auto it = sregex_token_iterator(in.begin(), in.end(), re, -1) ; it != sregex_token_iterator(); ++it)
            {
               // cout << "TOKEN: " << (string) *it << endl;
                out.push_back(it->str().c_str());
            }  
        }
        catch(const std::exception& e)
        {
            //cout<<"error:"<<e.what()<<std::endl;
            PlatformDebug::println(e.what());
        }

    }

};

#endif