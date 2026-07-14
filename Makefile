mac:
	clang -o vis main.c -I/opt/homebrew/include -L/opt/homebrew/lib -lraylib -framework CoreVideo -framework IOKit -framework Cocoa -framework GLUT -framework OpenGL

windows:
	gcc -o vis.exe main.c -Iinclude -Llib -lraylib -lgdi32 -lwinmm
