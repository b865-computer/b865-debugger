#pragma once
#ifndef _B865_EMU_UTIL_H
#define _B865_EMU_UTIL_H

#include "common.h"

#ifdef _WIN32
#include <windows.h>
#include <Windows.h>
typedef HANDLE M_PROCESS_OUT;
typedef HANDLE M_PROCESS;
#define M_PROCESS_INVALID INVALID_HANDLE_VALUE
#else
#include <fcntl.h>
#include <unistd.h>
typedef FILE* M_PROCESS_OUT;
typedef bool M_PROCESS;
#define M_PROCESS_INVALID 0
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

void initExeBasePath();
std::string getExeBasePath();
std::string getFilePathFromExeRelative(std::string relPath);

std::string getFnWithoutPath(std::string filename);
std::string getPath(std::string filename);

bool isExtEqual(std::string filename, std::string ext);
std::string getExt(std::string filename);

void setCwdArgv0(char* argv0);
std::string getCwd();

M_PROCESS_OUT startProgram(const std::string& cwd, const std::string& cmd, M_PROCESS& process);
bool pollProgramOutput(M_PROCESS_OUT process, std::string& outPutLines);
unsigned long programExitCode(M_PROCESS process, M_PROCESS_OUT out, bool* running);

#endif