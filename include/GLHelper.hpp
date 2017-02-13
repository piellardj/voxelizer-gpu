#ifndef GLHELPER_HPP_INCLUDED
#define GLHELPER_HPP_INCLUDED


void gl_CheckError(const char* file, unsigned int line, const char* expression);

#define DEBUG
#ifdef DEBUG
    #define GLCHECK(expr) do { expr; gl_CheckError(__FILE__, __LINE__, #expr); } while (false)
#else
    #define GLCHECK(expr) (expr)
#endif // DEBUG


#endif // GLHELPER_HPP_INCLUDED
