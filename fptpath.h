#ifndef FPTPATH_H
#define FPTPATH_H
#include <QDir>

//this path was hidden with this command
// attrib +s +h "./.fpt"
//to unhide use
//attrib -s -h "./.fpt"
#define _FPT_PATH_ "./.fpt"
//#define _FPT_IMG_PATH(fn)  (_FPT_PATH_ "/temp/" fn)
#define _TEMP_FPT_PATH(fn) (_FPT_PATH_ "/temp/" fn)

#endif // FPTPATH_H
