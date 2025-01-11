#pragma once
#ifndef _FILEPATH_H
#define _FILEPATH_H
#include <string>
void initExeBasePath();
std::string getExeBasePath();
std::string getFilePathFromExeRelative(std::string relPath);
std::string getPath(std::string filename);
bool isExtEqual(std::string filename, std::string ext);
std::string getExt(std::string filename);
void setCwdArgv0(char* argv0);
std::string getCwd();
#endif