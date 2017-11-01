#ifndef _MAIN_H
#define _MAIN_H

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include <assert.h>

#ifndef WINDOWS
#include <pthread.h>
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glext.h>

#endif
