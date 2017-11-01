#include "main.h"
#include "math/float4.h"
#include "math/mat4.h"
#include "graphics/gfx.h"
#include "memory/memory.h"
#include "shaders.h"
#include "textures/tga.h"

#ifdef WINDOWS
#define GET_PROC_ADDRESS_GL wglGetProcAddress
#define GET_PROC_ADDRESS_GL_TYPE const char*
#else
#define GET_PROC_ADDRESS_GL glXGetProcAddress
#define GET_PROC_ADDRESS_GL_TYPE const GLubyte*
#endif

PFNGLGENBUFFERSPROC glGenBuffers = NULL;
PFNGLBINDBUFFERPROC glBindBuffer = NULL;
PFNGLBUFFERDATAPROC glBufferData = NULL;
PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;
PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
PFNGLCREATESHADERPROC glCreateShader = NULL;
PFNGLSHADERSOURCEPROC glShaderSource = NULL;
PFNGLCOMPILESHADERPROC glCompileShader = NULL;
PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
PFNGLATTACHSHADERPROC glAttachShader = NULL;
PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
PFNGLUSEPROGRAMPROC glUseProgram = NULL;
PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
PFNGLUNIFORM1IPROC glUniform1i = NULL;
PFNGLACTIVETEXTUREPROC glActiveTexture = NULL;

/* GTK stuff */
GtkBuilder* gtk_builder;
GtkWidget* gtk_window;
GtkWidget* gtk_viewport;
GtkWidget *gtk_glarea = NULL;

bool gl_initialized = false;

/* OpenGL output */
bool gl_texture_exist = false;
unsigned int gl_texture;
unsigned int gl_vbo;
unsigned int gl_vao;
unsigned int gl_vs;
unsigned int gl_fs;
unsigned int gl_shader;
unsigned int width = 640, height = 480;

unsigned int image_width = 0, image_height = 0;

/* GFX stuff */
unsigned int gfx_backbuffer;			// Backbuffer framebuffer
unsigned int gfx_backbuffer_texture;	// Color attachment
unsigned int gfx_backbuffer_depth;		// Depth attachment

unsigned int gfx_texture;				// Diffuse map

unsigned int gfx_shader;				// Shader

unsigned int gfx_cube;					// Cube buffer
float4* cube_points;					// Vertices & their count
unsigned int cube_points_count;

mat4 gfx_projection;					// Projection matrix
mat4 gfx_model;							// Modelview matrix

// Logic (motion) stuff
float value = 0.0005f;

bool up_pressed = false;
bool down_pressed = false;
bool left_pressed = false;
bool right_pressed = false;
bool pgup_pressed = false;
bool pgdn_pressed = false;

float speed_y = 0.0f;
float angle_y = 0.0f;

float speed_x = 0.0f;
float angle_x = 0.0f;

float position = -10.0f;

// Callback for destroy window
G_MODULE_EXPORT gboolean destroy_callback(GtkWidget *widget, GdkEvent  *event, gpointer user_data)
{
	gtk_widget_destroy(gtk_window);
	gtk_main_quit();
	return FALSE;
}

// Callback for keypress
gboolean glarea_key_press_event(GtkWidget *widget, GdkEvent *e, gpointer user_data)
{
	if(e->type == GDK_KEY_PRESS)  
	{
		if(e->key.keyval == 65361)
		{
			left_pressed = true;
		}
		
		if(e->key.keyval == 65362)
		{
			up_pressed = true;
		}
		
		if(e->key.keyval == 65363)
		{
			right_pressed = true;
		}
		
		if(e->key.keyval == 65364)
		{
			down_pressed = true;
		}
		
		if(e->key.keyval == 65365)
		{
			pgup_pressed = true;
		}
		
		if(e->key.keyval == 65366)
		{
			pgdn_pressed = true;
		}
	}

	return TRUE;
}

// Callback for keyrelease
gboolean glarea_key_release_event(GtkWidget *widget, GdkEvent *e, gpointer user_data)
{
	if(e->type == GDK_KEY_RELEASE)  
	{
		if(e->key.keyval == 65361)
		{
			left_pressed = false;
		}
		
		if(e->key.keyval == 65362)
		{
			up_pressed = false;
		}
		
		if(e->key.keyval == 65363)
		{
			right_pressed = false;
		}
		
		if(e->key.keyval == 65364)
		{
			down_pressed = false;
		}
		
		if(e->key.keyval == 65365)
		{
			pgup_pressed = false;
		}
		
		if(e->key.keyval == 65366)
		{
			pgdn_pressed = false;
		}
	}
	
	return TRUE;
}

// Viewport resize
void gl_resize(GtkGLArea* area, gint w, gint h, gpointer user_data)
{
	width = w;
	height = h;
}

// Configure GL context
void gl_init(GtkWidget* widget, gpointer user_data)
{
	gtk_gl_area_make_current(GTK_GL_AREA(widget));

	if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
	{
		return;
	}

	glGenBuffers = (PFNGLGENBUFFERSPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glGenBuffers");
	glBindBuffer = (PFNGLBINDBUFFERPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glBufferData");
	glGenVertexArrays = (PFNGLGENVERTEXARRAYSPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glGenVertexArrays");
	glBindVertexArray = (PFNGLBINDVERTEXARRAYPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glBindVertexArray");
	glEnableVertexAttribArray = (PFNGLENABLEVERTEXATTRIBARRAYPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glEnableVertexAttribArray");
	glVertexAttribPointer = (PFNGLVERTEXATTRIBPOINTERPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glVertexAttribPointer");
	glCreateShader = (PFNGLCREATESHADERPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glCreateShader");
	glShaderSource = (PFNGLSHADERSOURCEPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glShaderSource");
	glCompileShader = (PFNGLCOMPILESHADERPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glCompileShader");
	glCreateProgram = (PFNGLCREATEPROGRAMPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glCreateProgram");
	glAttachShader = (PFNGLATTACHSHADERPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glAttachShader");
	glLinkProgram = (PFNGLLINKPROGRAMPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glLinkProgram");
	glUseProgram = (PFNGLUSEPROGRAMPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glUseProgram");
	glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glGetUniformLocation");
	glUniform1i = (PFNGLUNIFORM1IPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glUniform1i");
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)GET_PROC_ADDRESS_GL((GET_PROC_ADDRESS_GL_TYPE)"glActiveTexture");

	float vbo[] = { -1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, -1.0f, 0.0f, 1.0f,
		1.0f, 1.0f, 0.0f, 1.0f,
		-1.0f, 1.0f, 0.0f, 1.0f };

	glGenBuffers(1, &gl_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, vbo, GL_STATIC_DRAW);

	glGenVertexArrays(1, &gl_vao);
	glBindVertexArray(gl_vao);
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, NULL);

	const char* vertex_shader =
		"#version 400\n"
		"in vec3 vp;"
		"out vec2 texCoord;"
		"void main() {"
		"  gl_Position = vec4(vp, 1.0);"
		"  texCoord = vp.xy * 0.5 + 0.5;"
		"}";

	const char* fragment_shader =
		"#version 400\n"
		"in vec2 texCoord;"
		"uniform sampler2D image;"
		"out vec4 color;"
		"void main() {"
		"  color = texture(image, texCoord);"
		"}";

	gl_vs = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(gl_vs, 1, &vertex_shader, NULL);
	glCompileShader(gl_vs);
	gl_fs = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(gl_fs, 1, &fragment_shader, NULL);
	glCompileShader(gl_fs);
	gl_shader = glCreateProgram();
	glAttachShader(gl_shader, gl_fs);
	glAttachShader(gl_shader, gl_vs);
	glLinkProgram(gl_shader);

	glClearColor(0.0, 0.0, 0.0, 0.0);

	glEnable(GL_DEPTH_TEST);
	glClearDepth(1.0);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	GdkGLContext *glcontext = gtk_gl_area_get_context(GTK_GL_AREA(gtk_glarea));
	GdkWindow *glwindow = gdk_gl_context_get_window(glcontext);
	GdkFrameClock *frame_clock = gdk_window_get_frame_clock(glwindow);
	gdk_frame_clock_begin_updating(frame_clock);
}

// Free output GL context
void gl_fini(GtkWidget* widget, gpointer user_data)
{
	gtk_gl_area_make_current(GTK_GL_AREA(widget));

	if (gtk_gl_area_get_error(GTK_GL_AREA(widget)) != NULL)
	{
		return;
	}
}

// Render callback
gboolean gl_draw(GtkGLArea *area, GdkGLContext *context, gpointer user_data)
{
	if(up_pressed)
	{
		speed_y -= value;
	}
	
	if(down_pressed)
	{
		speed_y += value;
	}
	
	if(left_pressed)
	{
		speed_x += value;
	}
	
	if(right_pressed)
	{
		speed_x -= value;
	}
	
	if(pgup_pressed)
	{
		position += 0.1f;
	}
	
	if(pgdn_pressed)
	{
		position -= 0.1f;
	}
	
	angle_x += speed_x;
	angle_y += speed_y;
	
	mat4 rotate_x;
	rotate_x.fp.m1 = _mm_setr_ps(cos(angle_x), 0.0f, sin(angle_x), 0.0f);
	rotate_x.fp.m2 = _mm_setr_ps(0.0f, 1.0f, 0.0f, 0.0f);
	rotate_x.fp.m3 = _mm_setr_ps(-sin(angle_x), 0.0f, cos(angle_x), 0.0f);
	rotate_x.fp.m4 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
	
	mat4 rotate_y;
	rotate_y.fp.m1 = _mm_setr_ps(1.0f, 0.0f, 0.0f, 0.0f);
	rotate_y.fp.m2 = _mm_setr_ps(0.0f, cos(angle_y), sin(angle_y), 0.0f);
	rotate_y.fp.m3 = _mm_setr_ps(0.0f, -sin(angle_y), cos(angle_y), 0.0f);
	rotate_y.fp.m4 = _mm_setr_ps(0.0f, 0.0f, 0.0f, 1.0f);
	
	gfx_model = mul_mat_mat(rotate_x, rotate_y);
	gfx_model.fp.m4 = _mm_setr_ps(0.0f, 0.0f, position, 1.0f);
	
	if(image_width != width || image_height != height)
	{
		// Has image been resized? If so, resize buffers & recompute projection matrix
		
		if(!gl_texture_exist)
		{
			glGenTextures(1, &gl_texture);
			gl_texture_exist = true;
		}
		
		image_width = width;
		image_height = height;

		glBindTexture(GL_TEXTURE_2D, gl_texture);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_FLOAT, 0);

		if(gfx_backbuffer_texture)
		{
			gfxDeleteTexture(gfx_backbuffer_texture);
		}

		gfxGenTexture(&gfx_backbuffer_texture);
		gfxBindTexture(gfx_backbuffer_texture);
		gfxTexImage2D(GFX_RGBA8, image_width, image_height, NULL);
	
		if(gfx_backbuffer_depth)
		{
			gfxDeleteTexture(gfx_backbuffer_depth);
		}
		
		gfxGenTexture(&gfx_backbuffer_depth);
		gfxBindTexture(gfx_backbuffer_depth);
		gfxTexImage2D(GFX_DEPTH32F, image_width, image_height, NULL);
		
		gfx_projection = projection(60.0f, (float)image_width / (float)image_height, 0.1f, 1000.0f);
	}
	
	// Bind framebuffer
	gfxBindFramebuffer(gfx_backbuffer);
	gfxFramebufferAttachment(GFX_COLOR_ATTACHMENT, gfx_backbuffer_texture);
	gfxFramebufferAttachment(GFX_DEPTH_ATTACHMENT, gfx_backbuffer_depth);
	
	// Clear color & depth
	gfxClearColor(0, 0, 0, 255);
	gfxClearDepth(1000.0f);
	
	gfxClear();

	// Bind shader & set uniforms
	gfxUseShader(gfx_shader);
	gfxUniform1f(0, 0.5f);
	gfxUniform1i(0, 0);
	gfxUniformMatrix4fv(0, (float*)&gfx_projection);
	gfxUniformMatrix4fv(1, (float*)&gfx_model);
	
	// Bind buffer & set attributes
	gfxBindBuffer(gfx_cube);
	gfxAttribPointer(0, sizeof(float4) * 2, 0);
	gfxAttribPointer(1, sizeof(float4) * 2, 1);
	
	// Set texture
	gfxActiveTexture(0);
	gfxBindTexture(gfx_texture);
	
	// Draw triangles
	gfxDrawArrays(GFX_TRIANGLES, 0, cube_points_count);
	
	// End of shader usage
	gfxUseShader(0);
	
	/* Now just output the color texture to backbuffer from OpenGL & render it to screen */
	unsigned int gfx_w, gfx_h;
	void* gfx_data;
	gfxSwapBuffers(&gfx_w, &gfx_h, &gfx_data);

	if(gl_texture_exist)
	{
		glBindTexture(GL_TEXTURE_2D, gl_texture);
		glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, image_width, image_height, GL_RGBA, GL_UNSIGNED_BYTE, gfx_data);
	}
	
	glViewport(0, 0, image_width, image_height);
		
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture);
	glEnable(GL_TEXTURE_2D);

	glUseProgram(gl_shader);
	glUniform1i(glGetUniformLocation(gl_shader, "image"), 0);
	glBindVertexArray(gl_vao);
	glBindBuffer(GL_ARRAY_BUFFER, gl_vbo);
	glDrawArrays(GL_TRIANGLES, 0, 6);
	
	glDisable(GL_TEXTURE_2D);

	return TRUE;
}

gboolean gl_force_redraw(gpointer data)
{
	gtk_gl_area_queue_render((GtkGLArea*)gtk_glarea);

	return TRUE;
}

/* Main */
int main(int argc, char *argv[])
{
	// GTK stuff
	gtk_init(&argc, &argv);

	gtk_builder = gtk_builder_new();
	gtk_builder_add_from_file(gtk_builder, "data/gui/gWindow.glade", NULL);
	
	gtk_window = GTK_WIDGET(gtk_builder_get_object(gtk_builder, "window1"));
	gtk_viewport = GTK_WIDGET(gtk_builder_get_object(gtk_builder, "box1"));

	gtk_glarea = gtk_gl_area_new();
	gtk_gl_area_set_has_depth_buffer(GTK_GL_AREA(gtk_glarea), TRUE);
	gtk_gl_area_set_has_stencil_buffer(GTK_GL_AREA(gtk_glarea), TRUE);
	gtk_gl_area_set_has_alpha(GTK_GL_AREA(gtk_glarea), TRUE);

	gtk_widget_set_events(GTK_WIDGET(gtk_glarea), GDK_ALL_EVENTS_MASK);
	g_signal_connect(gtk_glarea, "realize", G_CALLBACK(gl_init), NULL);
	g_signal_connect(gtk_glarea, "unrealize", G_CALLBACK(gl_fini), NULL);
	g_signal_connect(gtk_glarea, "render", G_CALLBACK(gl_draw), NULL);
	g_signal_connect(gtk_glarea, "resize", G_CALLBACK(gl_resize), NULL);
	g_signal_connect(gtk_window, "key_press_event", G_CALLBACK(glarea_key_press_event), NULL);
	g_signal_connect(gtk_window, "key_release_event", G_CALLBACK(glarea_key_release_event), NULL);

	gtk_box_pack_start(GTK_BOX(gtk_viewport), gtk_glarea, true, true, 0);
	gtk_widget_show(GTK_WIDGET(gtk_glarea));

	gtk_builder_connect_signals(gtk_builder, NULL);

	g_object_unref(G_OBJECT(gtk_builder));
	
	// Generate framebuffer
	gfxGenFramebuffer(&gfx_backbuffer);
	
	// Load textures
	tex_t* texture_temp;
	tgaLoadFile("data/crate.tga", &texture_temp);
	
	gfxGenTexture(&gfx_texture);
	gfxBindTexture(gfx_texture);
	gfxTexImage2D(GFX_RGBA8, texture_temp->w, texture_temp->h, texture_temp->data);
		
	// Setup cube points
	cube_points_count = 36;
	cube_points = (float4*)gfx_alloc(sizeof(float4) * cube_points_count * 2, 16);
	
	cube_points[0].xmm = _mm_setr_ps(-1, -1, 1, 1);
	cube_points[1].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[2].xmm = _mm_setr_ps(1, -1, 1, 1);
	cube_points[3].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[4].xmm = _mm_setr_ps(1, 1, 1, 1);
	cube_points[5].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[6].xmm = _mm_setr_ps(-1, -1, 1, 1);
	cube_points[7].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[8].xmm = _mm_setr_ps(1, 1, 1, 1);
	cube_points[9].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[10].xmm = _mm_setr_ps(-1, 1, 1, 1);
	cube_points[11].xmm = _mm_setr_ps(0, 1, 0, 0);
	
	cube_points[12].xmm = _mm_setr_ps(1, -1, 1, 1);
	cube_points[13].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[14].xmm = _mm_setr_ps(1, -1, -1, 1);
	cube_points[15].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[16].xmm = _mm_setr_ps(1, 1, -1, 1);
	cube_points[17].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[18].xmm = _mm_setr_ps(1, -1, 1, 1);
	cube_points[19].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[20].xmm = _mm_setr_ps(1, 1, -1, 1);
	cube_points[21].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[22].xmm = _mm_setr_ps(1, 1, 1, 1);
	cube_points[23].xmm = _mm_setr_ps(0, 1, 0, 0);
	
	cube_points[24].xmm = _mm_setr_ps(1, -1, -1, 1);
	cube_points[25].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[26].xmm = _mm_setr_ps(-1, -1, -1, 1);
	cube_points[27].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[28].xmm = _mm_setr_ps(-1, 1, -1, 1);
	cube_points[29].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[30].xmm = _mm_setr_ps(1, -1, -1, 1);
	cube_points[31].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[32].xmm = _mm_setr_ps(-1, 1, -1, 1);
	cube_points[33].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[34].xmm = _mm_setr_ps(1, 1, -1, 1);
	cube_points[35].xmm = _mm_setr_ps(0, 1, 0, 0);
	
	cube_points[36].xmm = _mm_setr_ps(-1, -1, -1, 1);
	cube_points[37].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[38].xmm = _mm_setr_ps(-1, -1, 1, 1);
	cube_points[39].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[40].xmm = _mm_setr_ps(-1, 1, 1, 1);
	cube_points[41].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[42].xmm = _mm_setr_ps(-1, -1, -1, 1);
	cube_points[43].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[44].xmm = _mm_setr_ps(-1, 1, 1, 1);
	cube_points[45].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[46].xmm = _mm_setr_ps(-1, 1, -1, 1);
	cube_points[47].xmm = _mm_setr_ps(0, 1, 0, 0);
	
	cube_points[48].xmm = _mm_setr_ps(-1, 1, 1, 1);
	cube_points[49].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[50].xmm = _mm_setr_ps(1, 1, 1, 1);
	cube_points[51].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[52].xmm = _mm_setr_ps(1, 1, -1, 1);
	cube_points[53].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[54].xmm = _mm_setr_ps(-1, 1, 1, 1);
	cube_points[55].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[56].xmm = _mm_setr_ps(1, 1, -1, 1);
	cube_points[57].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[58].xmm = _mm_setr_ps(-1, 1, -1, 1);
	cube_points[59].xmm = _mm_setr_ps(0, 1, 0, 0);
	
	cube_points[60].xmm = _mm_setr_ps(-1, -1, -1, 1);
	cube_points[61].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[62].xmm = _mm_setr_ps(1, -1, -1, 1);
	cube_points[63].xmm = _mm_setr_ps(1, 0, 0, 0);
	cube_points[64].xmm = _mm_setr_ps(1, -1, 1, 1);
	cube_points[65].xmm = _mm_setr_ps(1, 1, 0, 0);
	
	cube_points[66].xmm = _mm_setr_ps(-1, -1, -1, 1);
	cube_points[67].xmm = _mm_setr_ps(0, 0, 0, 0);
	cube_points[68].xmm = _mm_setr_ps(1, -1, 1, 1);
	cube_points[69].xmm = _mm_setr_ps(1, 1, 0, 0);
	cube_points[70].xmm = _mm_setr_ps(-1, -1, 1, 1);
	cube_points[71].xmm = _mm_setr_ps(0, 1, 0, 0);

	// Create cube mesh
	gfxGenBuffer(&gfx_cube);
	gfxBindBuffer(gfx_cube);
	gfxBufferData(GFX_ARRAY_BUFFER, sizeof(float4) * cube_points_count * 2, (void*)cube_points);
	
	// Create shader & set shader source
	gfx_shader = gfxCreateShader();
	gfxUseShader(gfx_shader);
	gfxShaderSource(gfx_shader, &vsh_main, &fsh_main);
	gfxUseShader(0);
	
	// Rest of GTK stuff
	g_object_unref(G_OBJECT(gtk_builder));
	
	gtk_widget_queue_draw(gtk_viewport);

	gtk_widget_show(gtk_window);

	g_timeout_add(1, gl_force_redraw, NULL);
	
	gtk_main();
	
	return 0;
}
