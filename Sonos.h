using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <map>
#include <syslog.h>
#include <filesystem>
#include <string>
#include <cstring>
#include <iostream>
#include <sys/stat.h>
#include <curl/curl.h>

extern char *pProgramPath;

#include "CReadFile.h"
#include "CJson.h"
#include "CSonos.h"
