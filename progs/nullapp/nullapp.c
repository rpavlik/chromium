/**
 * Null application.  Just create/show an empty window.
 *
 * This can be useful for sort-last rendering with Renderserver.
 * Use the windowtracker SPU to render into this window with
 * binary swap:
 *
 * Run "nullapp -title DestWindow"
 * In config file:
 *   s = SPU('windowtracker')
 *   s.Conf('window_title', 'DestWindow')
 *
 */

#define GL_GLEXT_PROTOTYPES
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <GL/glut.h>

static int Win = 0;

static void
printstring(void *font, const char *string)
{
	const char *c;
	for (c = string; *c; c++)
		glutBitmapCharacter(font, *c);
}

static void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT);
#if defined(GLX)
	glWindowPos2iARB(10, 10);
#endif /*GLX*/
	printstring(GLUT_BITMAP_8_BY_13, "Chromium nullapp");
	glutSwapBuffers();
}

static void
keyboard(unsigned char key, int x, int y)
{
	(void) x;
	(void) y;
	if (key == 27) {
		glutDestroyWindow(Win);
		exit(0);
	}
}

static void
usage(void)
{
	printf("Usage:\n");
	printf("   nullapp [options]:\n");
	printf("Options:\n");
	printf("   -title TITLE    specify window title\n");
	printf("   -help           show this information\n");
}

int
main(int argc, char **argv)
{
	const char *title = "nullapp";
	int i;
	glutInit(&argc, argv);
	for (i = 1; i < argc; i++) {
		if (!strcmp(argv[i],"-title") && i + 1 < argc) {
			title = argv[i + 1];
			i++;
		}
		else if (!strcmp(argv[i],"-help")) {
			usage();
			return 0;
		}
		else {
			fprintf(stderr, "nullapp: unknown option %s\n", argv[i]);
			return 1;
		}
	}
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	Win = glutCreateWindow(title);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMainLoop();
	return 0;
}
