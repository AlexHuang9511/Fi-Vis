#include "log.h"
#include <raylib.h>
#include <stdio.h>

LOG_INIT

const unsigned int palette[] = {0x000000ff,
0x181818ff,
0x282828ff,
0x383838ff,
0x474747ff,
0x565656ff,
0x646464ff,
0x717171ff,
0x7e7e7eff,
0x8c8c8cff,
0x9b9b9bff,
0xabababff,
0xbdbdbdff,
0xd1d1d1ff,
0xe7e7e7ff,
0xffffffff};

typedef struct {
  Vector2 pos;
  float size;
  Color color;
} Square;

void drawSquare(Square s) {
  DrawRectangle(s.pos.x, s.pos.y, s.size, s.size, s.color);
}

Color getColorFromInt(int i) {
  Color ret = GetColor(palette[i]);
  return ret;
}

Color getColorFromByte(unsigned char byte) {
  int right = byte % 16;
  Color r = getColorFromInt(right);
  int left = (byte / 16) % 16;
  Color l = getColorFromInt(left);

  Color ret = {0};
  ret.r = (r.r * 1) + l.r;
  ret.g = (r.g * 1) + l.g;
  ret.b = (r.b * 1) + l.b;
  ret.a = 0xff;
  // ret.r = r.r;
  // ret.g = r.g;
  // ret.b = r.b;
  // ret.a = r.a;

  return ret;
}

int main(int argc, char *argv[]) {
  l_init();
  // FILE *fp = fopen("test.txt", "rw");
  if (argc != 2) {
    l_log(L_INFO, "Usage: ./vis {file name}");
  }
  FILE *fp = fopen(argv[1], "r");
  if (fp == NULL) {
    return -1;
  }

  // get file size
  fseek(fp, 0, SEEK_END);
  long fileSize = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  l_logf(L_INFO, "file size: %d", fileSize);
  char mainBuf[fileSize + 1];

  // 16 across

  // display in hex
  size_t read = fread(mainBuf, 1, sizeof(mainBuf), fp);
  l_logf(L_INFO, "read: %d", read);
  int counter = 0;
  for (size_t i = 0; i < read; i++) {
    printf("%02x", (unsigned)(unsigned char)mainBuf[i]);
    printf(" ");
    if (counter == 7) {
      printf(" ");
    }
    counter++;
    if (counter == 16) {
      printf("\n");
      counter = 0;
    }
  }

  printf("\n");

  counter = 0;
  for (size_t i = 0; i < read; i++) {
    printf("%u", (unsigned)(unsigned char)mainBuf[i]);
    printf(" ");
    if (counter == 7) {
      printf(" ");
    }
    counter++;
    if (counter == 16) {
      printf("\n");
      counter = 0;
    }
  }

  //-------------------------------------------
  // RayLib

  // Rectangle squares[fileSize];
  Square squares[fileSize];
  float count = 0.0;
  const int size = 50;
  const int span = 64;
  const int padding = 5;

  for (int i = 0; i < fileSize; i++) {
    squares[i].size = size;
    squares[i].pos.x = (i % span) * (size + padding);
    if (i % span == 0 && i != 0) {
      count = count + size + padding;
    }
    squares[i].pos.y = count;
    squares[i].color = getColorFromByte((unsigned)(unsigned char)mainBuf[i]);
    // printf("x: %f, y: %f\n", squares[i].pos.x, squares[i].pos.y);
  }

  const int width = 800;
  const int height = 600;
  const float maxZoom = 10.0f;
  const float minZoom = 0.01f;
  InitWindow(width, height, "File Visualizer");

  Camera2D camera = {0};
  camera.target = (Vector2){width / 2.0f, height / 2.0f};
  camera.offset = (Vector2){width / 2.0f, height / 2.0f};
  camera.rotation = 0.0f;
  camera.zoom = 1.0f;

  SetTargetFPS(60);

  while (!WindowShouldClose()) {
    // Keyboard scrolling
    float speed = 1000.0f * GetFrameTime();

    if (IsKeyDown(KEY_D))
      camera.target.x += speed;
    if (IsKeyDown(KEY_A))
      camera.target.x -= speed;
    if (IsKeyDown(KEY_S))
      camera.target.y += speed;
    if (IsKeyDown(KEY_W))
      camera.target.y -= speed;

    // Mouse wheel zoom
    float wheel = GetMouseWheelMove();

    if (wheel != 0) {
      if (IsKeyDown(KEY_LEFT_SHIFT) || IsKeyDown(KEY_RIGHT_SHIFT)) {
        Vector2 mouseWorldBefore = GetScreenToWorld2D(GetMousePosition(), camera);

        camera.zoom += wheel * 0.05f;

        if (camera.zoom < minZoom)
          camera.zoom = minZoom;

        if (camera.zoom > maxZoom)
          camera.zoom = maxZoom;

        Vector2 mouseWorldAfter = GetScreenToWorld2D(GetMousePosition(), camera);

        camera.target.x += mouseWorldBefore.x - mouseWorldAfter.x;
        camera.target.y += mouseWorldBefore.y - mouseWorldAfter.y;
      } else {
        camera.target.y -= wheel * 100.0f / camera.zoom;
      }
    }

    // Mouse drag scrolling
    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
      Vector2 delta = GetMouseDelta();

      delta.x /= camera.zoom;
      delta.y /= camera.zoom;

      camera.target.x -= delta.x;
      camera.target.y -= delta.y;
    }

    BeginDrawing();
    ClearBackground(BLACK);
    BeginMode2D(camera);

    for (int i = 0; i < fileSize; i++) {
      drawSquare(squares[i]);
    }

    EndMode2D();
    EndDrawing();
  }

  CloseWindow();

  return 0;
}
