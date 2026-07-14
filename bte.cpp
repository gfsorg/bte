#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <algorithm>
#include <cctype>
#include <ncurses.h>

// ============================================================================
// RAII Session Management
// ============================================================================
class NcursesSession {
public:
    NcursesSession() {
        initscr();
        raw();
        noecho();
        keypad(stdscr, TRUE);   // Let ncurses decode arrows, system keys, and resize events
        set_escdelay(25);       // Faster ESC vs Escape Sequence detection
    }
    ~NcursesSession() { endwin(); }
};

// ============================================================================
// BTE Editor Class
// ============================================================================
enum class Mode { Insert, Normal };

class BasicTextEditor {
    std::vector<std::string> buffer;
    std::string filename;
    bool is_dirty = false;
    Mode mode = Mode::Insert;
    int cursor_y = 0, cursor_x = 0, row_offset = 0;
    std::string cmd;
    bool command_mode = false;

public:
    explicit BasicTextEditor(const std::string& f) : filename(f) {
        if (!filename.empty()) loadFile();
        else buffer.push_back("");
    }

    void run() {
        NcursesSession session;
        while (handleInput());
    }

private:
    void loadFile() {
        std::ifstream f(filename);
        std::string l;
        while (std::getline(f, l)) buffer.push_back(l);
        if (buffer.empty()) buffer.push_back("");
    }

    bool saveFile(const std::string& save_path) {
        if (save_path.empty()) return false;
        std::ofstream f(save_path);
        if (!f.is_open()) return false;

        for (size_t i = 0; i < buffer.size(); ++i) {
            f << buffer[i];
            if (i < buffer.size() - 1) f << "\n";
        }
        filename = save_path;
        is_dirty = false;
        return true;
    }

    bool promptAndSave() {
        int my, mx; getmaxyx(stdscr, my, mx);
        mvprintw(my - 1, 0, "Save as: ");
        echo();
        char input_buf[256];
        getnstr(input_buf, sizeof(input_buf) - 1);
        noecho();
        std::string entered_name(input_buf);
        if (!entered_name.empty()) {
            return saveFile(entered_name);
        }
        return false;
    }

    // Dynamic left margin calculation for line numbers (Nano style)
    int getGutterWidth() const {
        int total_lines = static_cast<int>(buffer.size());
        int digits = 0;
        while (total_lines > 0) {
            digits++;
            total_lines /= 10;
        }
        return std::max(1, digits) + 2; 
    }

    void draw() {
        int my, mx; getmaxyx(stdscr, my, mx);
        erase();
        int th = my - 2;
        int gutter_width = getGutterWidth();
        int text_width = mx - gutter_width;

        if (cursor_y < row_offset) row_offset = cursor_y;
        if (cursor_y >= row_offset + th) row_offset = cursor_y - th + 1;

        for (int i = 0; i < th; ++i) {
            int bi = row_offset + i;
            if (bi < (int)buffer.size()) {
                // Render Nano-Style Line Numbers (Dynamic margin, no placeholders)
                std::string line_num_str = std::to_string(bi + 1);
                int padding = gutter_width - 2 - line_num_str.length();
                std::string gutter = std::string(padding, ' ') + line_num_str + "  ";
                
                attron(A_DIM);
                mvaddstr(i, 0, gutter.c_str());
                attroff(A_DIM);

                // Render Text Content
                if (text_width > 0) {
                    mvaddnstr(i, gutter_width, buffer[bi].c_str(), text_width);
                }
            }
        }

        // Status Bar
        attron(A_REVERSE);
        std::string bar = (mode == Mode::Insert ? " [INSERT] " : " [NORMAL] ") + (filename.empty() ? " New " : " " + filename + (is_dirty ? "*" : ""));
        mvaddnstr(my - 2, 0, bar.c_str(), mx);
        attroff(A_REVERSE);

        if (command_mode) mvprintw(my - 1, 0, "/%s", cmd.c_str());
        
        // Position Cursor with dynamic offset adjustment
        if (command_mode) {
            move(my - 1, (int)cmd.length() + 1);
        } else {
            move(cursor_y - row_offset, cursor_x + gutter_width);
        }
        refresh();
    }

    bool handleInput() {
        draw();
        int ch = getch();

        // Handle window resizing dynamically
        if (ch == KEY_RESIZE) {
            int my, mx;
            getmaxyx(stdscr, my, mx);
            resizeterm(my, mx);  // Tell ncurses to update its internal dimensions
            return true;
        }

        // 1. Unified Native Keycodes (When ncurses intercepts modified arrows)
        if (ch == 543 || ch == 545 || ch == 547 || ch == 548 || ch == 561) {
            navigateWordLeft();
            return true;
        }
        if (ch == 544 || ch == 560 || ch == 562 || ch == 563 || ch == 565) {
            navigateWordRight();
            return true;
        }

        // Standard arrow keys
        if (ch == KEY_UP) { moveY(-1); return true; }
        if (ch == KEY_DOWN) { moveY(1); return true; }
        if (ch == KEY_LEFT) { moveX(-1); return true; }
        if (ch == KEY_RIGHT) { moveX(1); return true; }

        // 2. Escape / Alt Sequence Parser (Fallback for multi-byte raw streams)
        if (ch == 27) { 
            nodelay(stdscr, TRUE);
            int next = getch();
            
            if (next == ERR) { 
                // A true ESC keypress (switch to Normal mode)
                nodelay(stdscr, FALSE);
                mode = Mode::Normal; 
                command_mode = false; 
                return true; 
            }

            // Consume rest of sequence in the queue
            std::vector<int> seq;
            seq.push_back(next);
            int seq_ch;
            while ((seq_ch = getch()) != ERR) {
                seq.push_back(seq_ch);
                if ((seq_ch >= 'A' && seq_ch <= 'Z') || (seq_ch >= 'a' && seq_ch <= 'z') || seq_ch == '~') {
                    break;
                }
            }
            nodelay(stdscr, FALSE);

            // Match sequence types
            if (seq.size() == 1) {
                if (seq[0] == 'b') navigateWordLeft();       
                else if (seq[0] == 'f') navigateWordRight();  
            } else if (seq.size() >= 2 && seq[0] == '[') {
                int last = seq.back();
                if (last == 'D') navigateWordLeft();         
                else if (last == 'C') navigateWordRight();        
                else if (last == 'A') moveY(-1);
                else if (last == 'B') moveY(1);
            }
            return true;
        }

        if (command_mode) return handleCmd(ch);

        if (mode == Mode::Normal) {
            if (ch == '/') command_mode = true;
            else if (ch == 'i') mode = Mode::Insert;
            return true;
        }

        // Insert Mode Keys
        // 127 & KEY_BACKSPACE act as single-character Backspace
        if (ch == KEY_BACKSPACE || ch == 127) {
            if (cursor_x > 0) { 
                buffer[cursor_y].erase(--cursor_x, 1); 
                is_dirty = true; 
            }
            else if (cursor_y > 0) {
                cursor_x = (int)buffer[cursor_y - 1].length();
                buffer[cursor_y - 1] += buffer[cursor_y];
                buffer.erase(buffer.begin() + cursor_y--);
                is_dirty = true;
            }
        } 
        // 8 is generated by Ctrl+Backspace (Ctrl+H) in terminals
        else if (ch == 8) { 
            deleteWordLeft();
        } 
        else if (ch == '\n' || ch == KEY_ENTER) {
            std::string rem = buffer[cursor_y].substr(cursor_x);
            buffer[cursor_y] = buffer[cursor_y].substr(0, cursor_x);
            buffer.insert(buffer.begin() + ++cursor_y, rem);
            cursor_x = 0;
            is_dirty = true;
        } else if (ch >= 32 && ch < 127) { 
            buffer[cursor_y].insert(cursor_x++, 1, (char)ch); 
            is_dirty = true; 
        }
        return true;
    }

    bool handleCmd(int ch) {
        if (ch == '\n' || ch == KEY_ENTER) {
            std::string active_cmd = cmd;
            cmd = ""; 
            command_mode = false;
            return executeCommand(active_cmd);
        } else if ((ch == 127 || ch == KEY_BACKSPACE || ch == 8) && !cmd.empty()) {
            cmd.pop_back();
        } else if (ch >= 32 && ch < 127) {
            cmd += (char)ch;
        }
        return true;
    }

    // ============================================================================
    // Command Engine
    // ============================================================================
    bool executeCommand(const std::string& active_cmd) {
        if (active_cmd == "edit") {
            mode = Mode::Insert;
        } 
        else if (active_cmd == "dw") {
            deleteWordAtCursor();
        }
        else if (active_cmd == "s") {
            if (filename.empty()) {
                promptAndSave();
            } else {
                saveFile(filename);
            }
        } 
        else if (active_cmd.rfind("sa ", 0) == 0) { 
            std::string new_name = active_cmd.substr(3);
            saveFile(new_name);
        } 
        else if (active_cmd == "qs") {
            if (filename.empty()) {
                if (promptAndSave()) return false; 
            } else {
                saveFile(filename);
                return false; 
            }
        } 
        else if (active_cmd.rfind("qsa ", 0) == 0) { 
            std::string new_name = active_cmd.substr(4);
            saveFile(new_name);
            return false;
        } 
        else if (active_cmd == "q") {
            return false; 
        }
        return true;
    }

    void moveY(int d) { 
        cursor_y = std::clamp(cursor_y + d, 0, (int)buffer.size() - 1); 
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length()); 
    }

    void moveX(int d) { 
        if (d == -1) {
            if (cursor_x > 0) cursor_x--;
            else if (cursor_y > 0) {
                cursor_y--;
                cursor_x = (int)buffer[cursor_y].length();
            }
        } else if (d == 1) {
            if (cursor_x < (int)buffer[cursor_y].length()) cursor_x++;
            else if (cursor_y < (int)buffer.size() - 1) {
                cursor_y++;
                cursor_x = 0;
            }
        }
    }

    // Deletes the word to the left of the cursor (for Ctrl+Backspace in Insert mode)
    void deleteWordLeft() {
        if (cursor_x > 0) {
            int original_x = cursor_x;
            int i = cursor_x - 1;
            while (i > 0 && isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i--;
            while (i > 0 && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i--;
            
            int start = (i == 0 && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) ? 0 : i + 1;
            buffer[cursor_y].erase(start, original_x - start);
            cursor_x = start;
            is_dirty = true;
        } else if (cursor_y > 0) {
            cursor_x = (int)buffer[cursor_y - 1].length();
            buffer[cursor_y - 1] += buffer[cursor_y];
            buffer.erase(buffer.begin() + cursor_y--);
            is_dirty = true;
        }
    }

    // Deletes the word starting at the cursor position (Vim dw style)
    void deleteWordAtCursor() {
        if (buffer[cursor_y].empty() || cursor_x >= (int)buffer[cursor_y].length()) return;
        
        int i = cursor_x;
        if (isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) {
            while (i < (int)buffer[cursor_y].length() && isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) {
                i++;
            }
        } else {
            while (i < (int)buffer[cursor_y].length() && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) {
                i++;
            }
            while (i < (int)buffer[cursor_y].length() && isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) {
                i++;
            }
        }
        
        buffer[cursor_y].erase(cursor_x, i - cursor_x);
        is_dirty = true;
        cursor_x = std::min(cursor_x, (int)buffer[cursor_y].length());
    }

    void navigateWordLeft() {
        if (cursor_x > 0) {
            int i = cursor_x - 1;
            while (i > 0 && isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i--;
            while (i > 0 && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i--;
            cursor_x = (i == 0 && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) ? 0 : i + 1;
        } else if (cursor_y > 0) {
            cursor_y--;
            cursor_x = (int)buffer[cursor_y].length();
        }
    }

    void navigateWordRight() {
        if (cursor_x < (int)buffer[cursor_y].length()) {
            int i = cursor_x;
            while (i < (int)buffer[cursor_y].length() && !isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i++;
            while (i < (int)buffer[cursor_y].length() && isspace(static_cast<unsigned char>(buffer[cursor_y][i]))) i++;
            cursor_x = i;
        } else if (cursor_y < (int)buffer.size() - 1) {
            cursor_y++;
            cursor_x = 0;
        }
    }
};

int main(int argc, char** argv) {
    BasicTextEditor(argc > 1 ? argv[1] : "").run();
    return 0;
}