#include <emscripten.h>
#include <emscripten/html5.h>
#include <vector>
#include <cstdlib>
#include <ctime>
#include <string>

int width = 20, height = 20, blockSize = 20;
std::vector<std::pair<int, int>> snake;
std::pair<int, int> food;
int dx = 1, dy = 0;
bool gameOver = false;
bool gameStarted = false;
int score = 0;

// JS binding to draw blocks
EM_JS(void, drawBlock, (int x, int y, const char* color), {
    var ctx = Module.ctx;
    ctx.fillStyle = UTF8ToString(color);
    ctx.fillRect(x * Module.blockSize, y * Module.blockSize, Module.blockSize, Module.blockSize);
});

// JS binding to draw text
EM_JS(void, drawText, (const char* text, int x, int y, const char* color, int size), {
    var ctx = Module.ctx;
    ctx.fillStyle = UTF8ToString(color);
    ctx.font = size + "px Arial";
    ctx.fillText(UTF8ToString(text), x, y);
});

// Reset game
void resetGame() {
    snake = {{10, 10}};
    dx = 1; dy = 0;
    food = {rand() % width, rand() % height};
    score = 0;
    gameOver = false;
    gameStarted = true;
}

// Update game state
void gameLoop() {
    EM_ASM({
        Module.ctx.clearRect(0, 0, Module.canvas.width, Module.canvas.height);
    });

    if (!gameStarted) {
        drawText("snake game", 100, 180, "green", 28);
        drawText("Press SPACE to Start", 120, 220, "white", 20);
        return;
    }

    if (gameOver) {
        drawText("GAME OVER!", 100, 180, "red", 28);
        std::string s = "Score: " + std::to_string(score);
        drawText(s.c_str(), 120, 220, "white", 20);
        drawText("Press SPACE to Restart", 60, 260, "yellow", 20);
        return;
    }

    // Move snake
    auto head = snake.front();
    head.first += dx;
    head.second += dy;

    // Check walls
    if (head.first < 0 || head.first >= width || head.second < 0 || head.second >= height) {
        gameOver = true; return;
    }

    // Check self collision
    for (auto s : snake) {
        if (s == head) { gameOver = true; return; }
    }

    snake.insert(snake.begin(), head);

    // Eat food
    if (head == food) {
        score++;
        food = {rand() % width, rand() % height};
    } else {
        snake.pop_back();
    }

    // Draw snake
    for (auto s : snake) {
        drawBlock(s.first, s.second, "green");
    }

    // Draw food
    drawBlock(food.first, food.second, "red");

    // Draw score
    std::string s = "Score: " + std::to_string(score);
    drawText(s.c_str(), 5, 20, "white", 16);
}

// Keyboard input
EM_BOOL key_callback(int eventType, const EmscriptenKeyboardEvent* e, void* userData) {
    if (e->keyCode == 37 && dx != 1) { dx = -1; dy = 0; } // Left
    if (e->keyCode == 38 && dy != 1) { dx = 0; dy = -1; } // Up
    if (e->keyCode == 39 && dx != -1) { dx = 1; dy = 0; } // Right
    if (e->keyCode == 40 && dy != -1) { dx = 0; dy = 1; } // Down
    if (e->keyCode == 32) { // SPACE = Start/Restart
        if (!gameStarted || gameOver) {
            resetGame();
        }
    }
    return true;
}

int main() {
    srand(time(NULL));

    EM_ASM({
        var canvas = document.getElementById('canvas');
        if (!canvas) {
            canvas = document.createElement('canvas');
            canvas.id = 'canvas';
            document.body.style.background = "#111";
            document.body.style.display = "flex";
            document.body.style.justifyContent = "center";
            document.body.style.alignItems = "center";
            document.body.style.height = "100vh";
            canvas.style.border = "2px solid white";
            document.body.appendChild(canvas);
        }
        canvas.width = $0 * $2;
        canvas.height = $1 * $2;
        Module.canvas = canvas;
        Module.ctx = canvas.getContext('2d');
        Module.blockSize = $2;
    }, width, height, blockSize);

    emscripten_set_keydown_callback(EMSCRIPTEN_EVENT_TARGET_WINDOW, 0, 1, key_callback);
    emscripten_set_main_loop(gameLoop, 8, 1); // ~8 FPS
}
