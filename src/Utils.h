#pragma once
#ifndef _B865_EMU_UTIL_H
#define _B865_EMU_UTIL_H

#include "Common.h"

#ifdef _WIN32
#include <windows.h>
typedef HANDLE M_PROCESS_OUT;
typedef HANDLE M_PROCESS;
#define M_PROCESS_INVALID INVALID_HANDLE_VALUE
#else
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <sched.h>
typedef FILE* M_PROCESS_OUT;
typedef bool M_PROCESS;
#define M_PROCESS_INVALID 0
#endif

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif


template<typename tVal> tVal map_value(std::pair<tVal,tVal> a, std::pair<tVal, tVal> b, tVal inVal);

void initExeBasePath();
std::string getExeBasePath();
std::string getFilePathFromExeRelative(std::string relPath);

std::string getFnWithoutPath(std::string filename);
std::string getPath(std::string filename);

bool isExtEqual(std::string filename, std::string ext);
std::string getExt(std::string filename);

M_PROCESS_OUT startProgram(const std::string& cwd, const std::string& cmd, M_PROCESS& process);
bool pollProgramOutput(M_PROCESS_OUT process, std::string& outPutLines);
unsigned long programExitCode(M_PROCESS process, M_PROCESS_OUT out, bool* running);

void pinThreadToCore(std::thread::native_handle_type handle, int core_id);
void setThreadPriority(std::thread::native_handle_type handle, bool high_priority);

#endif