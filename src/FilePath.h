#pragma once
#ifndef _FILEPATH_H
#define _FILEPATH_H
#include <string>
void setExeBasePath(char* argv0);
std::string getFilePathFromExeRelative(std::string relPath);
#endif