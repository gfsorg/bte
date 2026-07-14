#!/usr/bin/env bash

# Color definitions for beautiful UI
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
BOLD='\033[1m'
NC='\033[0m' # No Color

# Clear the screen to start clean
clear

echo -e "${CYAN}${BOLD}====================================================${NC}"
echo -e "${CYAN}${BOLD}         BTE (Basic Text Editor) Installer         ${NC}"
echo -e "${CYAN}${BOLD}====================================================${NC}"
echo

# 1. Check for bte.cpp in current directory
if [ ! -f "bte.cpp" ]; then
    echo -e "${RED}[ERROR] bte.cpp not found in the current directory!${NC}"
    echo -e "Please make sure you save your C++ code as 'bte.cpp' in this folder before running."
    exit 1
fi

# 2. Check and Install Dependencies
echo -e "${YELLOW}[1/4] Checking and installing dependencies...${NC}"
echo -e "This requires sudo privileges to install g++ and ncurses libraries."
sudo apt update && sudo apt install -y build-essential libncurses5-dev libncursesw5-dev

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Dependency installation failed.${NC}"
    exit 1
fi
echo -e "${GREEN}[] Dependencies successfully set up!${NC}\n"

# 3. Compile the Editor
echo -e "${YELLOW}[2/4] Compiling bte.cpp with C++17 and optimization...${NC}"
g++ -std=c++17 -O3 bte.cpp -o bte -lncurses

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Compilation failed. Please inspect your bte.cpp code.${NC}"
    exit 1
fi
echo -e "${GREEN}[] Compilation succeeded!${NC}\n"

# 4. Install Globally
echo -e "${YELLOW}[3/4] Installing 'bte' binary globally to /usr/local/bin/...${NC}"
sudo cp bte /usr/local/bin/bte
sudo chmod +x /usr/local/bin/bte

if [ $? -ne 0 ]; then
    echo -e "${RED}[ERROR] Installation failed. Could not copy to /usr/local/bin/.${NC}"
    exit 1
fi
echo -e "${GREEN}[] BTE is now globally installed! You can run it with just 'bte' anywhere.${NC}\n"

# 5. Clean up local binary to prevent clutter
rm -f bte

echo -e "${YELLOW}[4/4] Starting BTE Interactive Guide...${NC}"
echo -e "Press [ENTER] to launch the Guide Slideshow..."
read -r

# ============================================================================
# INTERACTIVE SLIDESHOW GUIDE
# ============================================================================
show_slide_header() {
    clear
    local title=$1
    local current_page=$2
    local total_pages=$3
    echo -e "${MAGENTA}${BOLD}====================================================${NC}"
    echo -e "${MAGENTA}${BOLD}   BTE GUIDE: ${title} (${current_page}/${total_pages})${NC}"
    echo -e "${MAGENTA}${BOLD}====================================================${NC}"
    echo
}

show_slide_footer() {
    echo
    echo -e "${MAGENTA}${BOLD}====================================================${NC}"
    echo -e "  [Enter] Next Slide   |   [b] Back   |   [q] Exit Guide"
    echo -e "${MAGENTA}${BOLD}====================================================${NC}"
    echo -n "Select option: "
}

current_slide=1
total_slides=5

while true; do
    case $current_slide in
        1)
            show_slide_header "Welcome to BTE" 1 5
            echo -e " BTE is a minimal, blazing-fast, modal terminal text editor."
            echo -e " It brings the clean aesthetic of ${YELLOW}nano${NC} together with the"
            echo -e " raw control and modal architecture of ${CYAN}vim${NC}."
            echo
            echo -e " ${BOLD}Key Highlights:${NC}"
            echo -e "   * No bulky margins or placeholder tildes (~)"
            echo -e "   * Dynamic nano-style line numbers"
            echo -e "   * Perfect input decoding for Chromebooks (Crostini)"
            echo -e "   * Robust Vim-style command execution engine"
            show_slide_footer
            ;;
        2)
            show_slide_header "The Two Modes" 2 5
            echo -e " BTE operates in two main modes:"
            echo
            echo -e " ${GREEN}${BOLD}1. INSERT MODE (Default Startup)${NC}"
            echo -e "   * Type directly into the buffer."
            echo -e "   * Hit ${RED}[ESC]${NC} to switch to Normal Mode."
            echo
            echo -e " ${BLUE}${BOLD}2. NORMAL MODE${NC}"
            echo -e "   * Hit ${YELLOW}[i]${NC} to go back into Insert Mode."
            echo -e "   * Hit ${YELLOW}[/]${NC} to open the command prompt at the bottom."
            show_slide_footer
            ;;
        3)
            show_slide_header "Navigation & Shortcuts" 3 5
            echo -e " BTE features native fixes tailored for ChromeOS/Crostini."
            echo
            echo -e " ${BOLD}Cursor Movement (Insert & Normal Modes):${NC}"
            echo -e "   * ${CYAN}Arrow Keys${NC} : Navigate letter-by-letter, line-by-line."
            echo -e "   * ${CYAN}Ctrl + Left / Right${NC} : Skip word-by-word (Safe on Chromebooks)."
            echo -e "   * ${CYAN}Alt + Left / Right${NC}  : Skip word-by-word (On Linux/macOS terminals)."
            echo
            echo -e " ${BOLD}Smart Deletion (Insert Mode):${NC}"
            echo -e "   * ${CYAN}Ctrl + Backspace${NC} : Instantly delete the word to your left."
            show_slide_footer
            ;;
        4)
            show_slide_header "The Command Engine" 4 5
            echo -e " Press ${YELLOW}[/]${NC} in Normal Mode to open the Command Engine."
            echo
            echo -e " ${BOLD}Core Commands:${NC}"
            echo -e "   * ${CYAN}/edit${NC}       - Return to Insert Mode."
            echo -e "   * ${CYAN}/s${NC}          - Save current file (prompts for name if new)."
            echo -e "   * ${CYAN}/sa [name]${NC}  - Save file under a specific new name."
            echo -e "   * ${CYAN}/qs${NC}         - Quick save current file and quit BTE."
            echo -e "   * ${CYAN}/qsa [name]${NC}- Quick save file as [name] and quit BTE."
            echo -e "   * ${CYAN}/q${NC}          - Quit BTE instantly without saving."
            echo -e "   * ${CYAN}/dw${NC}         - Delete word right at/under the cursor."
            show_slide_footer
            ;;
        5)
            show_slide_header "All Set! How to Run" 5 5
            echo -e " ${GREEN}${BOLD}Congratulations! BTE is completely installed!${NC}"
            echo
            echo -e " You can run BTE globally from any terminal directory:"
            echo
            echo -e "   ${BOLD}bte${NC}               - Open a new empty text buffer."
            echo -e "   ${BOLD}bte filename.txt${NC}  - Open or create a specific file."
            echo
            echo -e " To update or recompile the editor in the future, simply re-run"
            echo -e " this installer script."
            echo
            echo -e " ${YELLOW}Thank you for choosing BTE! Happy hacking.${NC}"
            show_slide_footer
            ;;
    esac

    read -r action
    if [[ "$action" == "q" || "$action" == "Q" ]]; then
        break
    elif [[ "$action" == "b" || "$action" == "B" ]]; then
        if [ $current_slide -gt 1 ]; then
            ((current_slide--))
        fi
    else
        if [ $current_slide -lt $total_slides ]; then
            ((current_slide++))
        else
            break # Exit loop on the last slide when Enter is hit
        fi
    fi
done

clear
echo -e "${GREEN}${BOLD}Installation complete. Start BTE now by typing: bte${NC}"