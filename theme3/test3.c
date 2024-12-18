#define BACKSPACE "\x7f"
#define ENTER "\x0d"
#define ESC "\x1b"
#define LOCATEHOME ESC "[H"
#define SAVECURSORLOC ESC "7"
#define RETCURSORLOC ESC "8"
#define SETSCREENSIZE ESC "[8;%d;%dt"
#define CLEARDISPLAY ESC "[H" ESC "[2J"
#define DELETELINEAFTERCURSOR ESC "[0K"
#define DELETELINEBEFORECURSOR ESC "[1K"
#define DELETELINEECURSOR ESC "[2K"
#define DELETESCREENAFTERCURSOR ESC "[0J"
#define DELETESCREENBEFORECURSOR ESC "[1J"
#define DELETESCREEN ESC "[2J"
#define LOCATECURSOR ESC "[%d;%df"
#define CURSORMOVE ESC "[%d;%dH"
#define CHANGESCROLLROWS ESC "[%d;%dr"
#define RESETSCROLLROWS ESC "[r"
#define CURSORMOVENUP ESC "[%dA"
#define CURSORMOVENDOWN ESC "[%dB"
#define CURSORMOVENRIGHT ESC "[%dC"
#define CURSORMOVENLEFT ESC "[%dD"
#define DELETENCHAR ESC "[%dP"
#define CURSORINVISIBLE ESC "[?25l"
#define CURSORVISIBLE ESC "[?25h"
// #define RESETLRMARGIN   ESC"[?69l"
#define SETLRMARGIN ESC "[?69h"
// #define CHANGESCROLLCOLS ESC"[%d;%ds"
// #define RESETSCROLLCOLS  ESC"[s"
#define DECTCEM 25
// #define DECLRMM 69
#define DECSETMODE ESC "[?%dh"
#define DECRESETMODE ESC "[?%dl"
#define SGR ESC "[%dm" // SelectGraphicRendition
#define RESETSGR ESC "[0m"
#define SGRCOL256 ESC "[38;5;%dm"
#define SGRBGCOL256 ESC "[48;5;%dm"
#define SGRCOL24B ESC "[38;2;%d;%d;%dm"
#define SGRBGCOL24B ESC "[48;2;%d;%d;%dm"
#define DECLINEDRAW ESC "(0"
#define ASCIIDRAW ESC "(B"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mtk_c.h"

#define MAX 1024
#define SECONDDIVIDER 100
#define BUFLEN 100
#define TOTALTIPS 4

int i, j;

bool in_alt_screen[2] = {false, false};
bool inappcom[2] = {true, true};
char keypress[2] = {-1, -1};
char client_nickname_color[2] = {11, 14};
char client_nickname[2][10] = {"Client1", "Client2"};
bool client_nickname_set[2] = {false, false};
int term_input_col[2];
char last_keybuf[2][10];
int last_keybuf_len[2];
char msg_buf[2][BUFLEN];
unsigned int msg_buf_len[2];
char cmd_buf[2][BUFLEN];
unsigned int cmd_buf_len[2];
char printf_buf[BUFLEN];
int tip_num[2];
unsigned int screen_height = 43;
unsigned int screen_width = 132;
unsigned int chat_row = 1;

const char term_backspace[] = "\b \b";
const char term_line10[] = "\x1b[10;H";
const char msg_input_prompt[] = "Your message";
const char command_prompt[] = "Command: ";
const char tip_list[TOTALTIPS][BUFLEN] = {
    "Press ESC to enter command mode",
    "Maximum length of each message is 100 characters",
    "Functionality of the arrow keys is suppressed",
    "Only help command is available for now"};
const char help_text[] = "\n\x1b[1;4mManual page for the chat application\x1b[0m\n\nThis page should be populated when the application is finished.\nPlease press any key to go back.\n";

void fast_printf(const bool ch0, const bool ch1, const char *fmt, ...)
{
    int printf_buf_len;
    va_list args;
    va_start(args, fmt);
    vsprintf(printf_buf, fmt, args);
    printf_buf_len = strlen(printf_buf);
    if (ch0)
        write(3, &printf_buf, printf_buf_len);
    if (ch1)
        write(4, &printf_buf, printf_buf_len);
    va_end(args);
}

void runtime_clock()
{
    unsigned int hr = 0, min = 0, sec = 0, last_runtime = 0;
    while (1)
    {
        P(0);
        P(1);
        if (runtime - last_runtime >= SECONDDIVIDER)
        {
            sec += (runtime - last_runtime) / SECONDDIVIDER;
            last_runtime = runtime;
            if (sec == 60)
            {
                sec = sec - 60;
                min++;
                if (min == 60)
                {
                    min = 0;
                    hr++;
                }
            }
            fast_printf(true, true, SETSCREENSIZE, screen_height, screen_width);
            fast_printf(true, true, CURSORINVISIBLE);
            fast_printf(true, true, SAVECURSORLOC);
            fast_printf(true, true, "\x1b[H\x1b[47;30mRuntime %02d:%02d:%02d \x1b[K\x1b[0m", hr, min, sec);
            fast_printf(true, true, RETCURSORLOC);
            fast_printf(true, true, CURSORVISIBLE);
        }
        V(1);
        V(0);
    }
}

void command_bar()
{
    int last_tip_changed[2];
    int some_num2;
    // a command bar at the bottom of the screen, it will accept command after the user press escape key
    while (1)
    {
        P(0);
        P(1);
        some_num2 = rand() % 100;
        for (i = 0; i <= 1; i++)
        {
            if (!inappcom[i])
            {
                keypress[i] = inkey(i);
                if (keypress[i] != -1)
                {
                    if (in_alt_screen[i])
                    {
                        if (keypress[i] != -1)
                        {
                            fast_printf(i == 0, i == 1, DECRESETMODE, 47);
                            in_alt_screen[i] = false;
                            fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K", command_prompt);
                        }
                    }
                    else
                    {
                        last_keybuf[i][last_keybuf_len[i]] = keypress[i];
                        last_keybuf_len[i]++;
                        last_keybuf[i][last_keybuf_len[i]] = '\0';
                    }
                }
                else if (last_keybuf_len[i] > 0)
                {
                    if (strcmp(last_keybuf[i], ESC) == 0)
                    {
                        fast_printf(i == 0, i == 1, RESETSGR);
                        fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]);
                        fast_printf(i == 0, i == 1, CURSORMOVE, screen_height - 2, 2);
                        fast_printf(i == 0, i == 1, "\x1b[1m%s\x1b[0m│%s", msg_input_prompt, msg_buf[i]);
                        inappcom[i] = true;
                        cmd_buf_len[i] = 0;
                        cmd_buf[i][0] = '\0';
                    }
                    else if (strcmp(last_keybuf[i], BACKSPACE) == 0)
                    {
                        if (cmd_buf_len[i] > 0)
                        {
                            fast_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", term_backspace);
                            cmd_buf_len[i]--;
                            cmd_buf[i][cmd_buf_len[i]] = '\0';
                        }
                    }
                    else if (strcmp(last_keybuf[i], ENTER) == 0)
                    {
                        if (cmd_buf_len[i] > 0)
                        {
                            if (strncmp(cmd_buf[i], "h", 1) == 0 && cmd_buf_len[i] == 1 || strncmp(cmd_buf[i], "help", 4) == 0 && cmd_buf_len[i] == 4)
                            {
                                fast_printf(i == 0, i == 1, DECSETMODE, 47);
                                fast_printf(i == 0, i == 1, RESETSGR);
                                fast_printf(i == 0, i == 1, CLEARDISPLAY);
                                fast_printf(i == 0, i == 1, help_text);
                                in_alt_screen[i] = true;
                                cmd_buf_len[i] = 0;
                                cmd_buf[i][0] = '\0';
                            }
                            // add more commands here
                            else
                            {
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", command_prompt);
                                cmd_buf_len[i] = 0;
                                cmd_buf[i][0] = '\0';
                            }
                        }
                    }
                    else if (last_keybuf_len[i] == 1 && last_keybuf[i][0] >= 32 && last_keybuf[i][0] <= 126)
                    {
                        // only allow printable characters
                        if (cmd_buf_len[i] < 100)
                        {
                            cmd_buf[i][cmd_buf_len[i]] = last_keybuf[i][0];
                            cmd_buf_len[i]++;
                            cmd_buf[i][cmd_buf_len[i]] = '\0';
                            fast_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", last_keybuf[i]);
                        }
                        else if (!(last_keybuf_len[i] == 3 && last_keybuf[i][0] == 27 && last_keybuf[i][1] == 91 && last_keybuf[i][2] >= 65 && last_keybuf[i][2] <= 68))
                        {
                            // ignore the arrow keys
                            write(i + 3, &last_keybuf[i], last_keybuf_len[i]);
                        }
                    }
                    last_keybuf[i][0] = '\0';
                    last_keybuf_len[i] = 0;
                }
                keypress[i] = -1;
            }
            else
            {
                if (runtime - last_tip_changed[i] >= 800 + some_num2)
                {
                    if (tip_num[i] == TOTALTIPS - 1)
                        tip_num[i] = 0;
                    else
                        tip_num[i]++;
                    last_tip_changed[i] = runtime;
                    fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                    fast_printf(i == 0, i == 1, SAVECURSORLOC);
                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]);
                    fast_printf(i == 0, i == 1, RETCURSORLOC);
                    fast_printf(i == 0, i == 1, CURSORVISIBLE);
                }
            }
        }
        V(1);
        V(0);
    }
}

void main_app()
{
    bool warn_length[2] = {false, false};
    bool warn_empty[2] = {false, false};
    int some_num1;
    while (1)
    {
        P(0);
        P(1);
        some_num1 = rand() % 100;
        for (i = 0; i <= 1; i++)
        {
            if (inappcom[i])
            {
                keypress[i] = inkey(i);
                if (keypress[i] != -1)
                {
                    last_keybuf[i][last_keybuf_len[i]] = keypress[i];
                    last_keybuf_len[i]++;
                    last_keybuf[i][last_keybuf_len[i]] = '\0';
                }
                else
                {
                    if (last_keybuf_len[i] > 0)
                    {
                        if (strcmp(last_keybuf[i], ESC) == 0)
                        {
                            fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", command_prompt);
                            inappcom[i] = false;
                        }
                        else if (strcmp(last_keybuf[i], BACKSPACE) == 0)
                        {
                            if (msg_buf_len[i] > 0)
                            {
                                write(i + 3, &term_backspace, 3);
                                msg_buf_len[i]--;
                                msg_buf[i][msg_buf_len[i]] = '\0';
                            }
                        }
                        else if (strcmp(last_keybuf[i], ENTER) == 0)
                        {
                            if (msg_buf_len[i] > 0)
                            {
                                fast_printf(true, true, DECRESETMODE, 47);
                                if (warn_empty[i])
                                {
                                    fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                    fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]);
                                    fast_printf(i == 0, i == 1, RETCURSORLOC);
                                    fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                    warn_empty[i] = false;
                                }
                                fast_printf(true, true, CURSORINVISIBLE);
                                fast_printf(true, true, CURSORMOVE, 1, 1);
                                for (j = 0; j < chat_row; j++)
                                    printf_buf[j] = '\n';
                                printf_buf[j + 1] = '\0';
                                write(3, &printf_buf, chat_row);
                                write(4, &printf_buf, chat_row);
                                if (chat_row < screen_height - 5)
                                    chat_row++;
                                fast_printf(true, true, "\x1b[0m\n│\x1b[1;38;5;%dm%s: \x1b[0m", client_nickname_color[i], client_nickname[i]);
                                write(3, &msg_buf[i], msg_buf_len[i]);
                                write(4, &msg_buf[i], msg_buf_len[i]);
                                fast_printf(true, true, "\x1b[%d;999H│", chat_row + 1);
                                msg_buf_len[i] = 0;
                                msg_buf[i][0] = '\0';

                                fast_printf(true, false, CURSORMOVE, screen_height - 2, term_input_col[0] + msg_buf_len[0]);
                                fast_printf(false, true, CURSORMOVE, screen_height - 2, term_input_col[1] + msg_buf_len[1]);
                                fast_printf(i == 0, i == 1, DELETELINEAFTERCURSOR);
                                fast_printf(true, true, "\x1b[%d;999H│", screen_height - 2);
                                if (inappcom[0])
                                {
                                    fast_printf(true, false, CURSORMOVE, screen_height - 2, term_input_col[0] + msg_buf_len[0]);
                                }
                                else
                                {
                                    fast_printf(true, false, CURSORMOVE, screen_height, strlen(command_prompt) + cmd_buf_len[0] + 1);
                                }
                                if (inappcom[1])
                                {
                                    fast_printf(false, true, CURSORMOVE, screen_height - 2, term_input_col[1] + msg_buf_len[1]);
                                }
                                else
                                {
                                    fast_printf(false, true, CURSORMOVE, screen_height, strlen(command_prompt) + cmd_buf_len[1] + 1);
                                }
                                fast_printf(true, true, CURSORVISIBLE);
                            }
                            else
                            {
                                fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mSending Empty message is not allowed\x1b[K\x1b[0m");
                                fast_printf(i == 0, i == 1, RETCURSORLOC);
                                fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                warn_empty[i] = true;
                            }
                            fast_printf(in_alt_screen[0], in_alt_screen[1], DECSETMODE, 47);
                            fast_printf(in_alt_screen[0], in_alt_screen[1], CURSORMOVE, 1, 1);
                            fast_printf(in_alt_screen[0], in_alt_screen[1], help_text);
                        }
                        else if (last_keybuf_len[i] == 1 && last_keybuf[i][0] >= 32 && last_keybuf[i][0] <= 126)
                        {
                            // only allow printable characters
                            if (msg_buf_len[i] < 100)
                            {
                                msg_buf[i][msg_buf_len[i]] = last_keybuf[i][0];
                                msg_buf_len[i]++;
                                msg_buf[i][msg_buf_len[i]] = '\0';
                                write(i + 3, &last_keybuf[i], last_keybuf_len[i]);
                                if (warn_length[i])
                                {
                                    fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                    fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]);
                                    fast_printf(i == 0, i == 1, RETCURSORLOC);
                                    fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                    warn_length[i] = false;
                                }
                            }
                            else
                            {
                                fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mMaximum character limit per message reached (%d characters)\x1b[K\x1b[0m", BUFLEN);
                                fast_printf(i == 0, i == 1, RETCURSORLOC);
                                fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                warn_length[i] = true;
                            }
                        }
                        else if (!(last_keybuf_len[i] == 3 && last_keybuf[i][0] == 27 && last_keybuf[i][1] == 91 && last_keybuf[i][2] >= 65 && last_keybuf[i][2] <= 68))
                        {
                            // ignore the arrow keys
                            write(i + 3, &last_keybuf[i], last_keybuf_len[i]);
                        }
                        last_keybuf[i][0] = '\0';
                        last_keybuf_len[i] = 0;
                    }
                }
                keypress[i] = -1;
            }
        }
        V(1);
        V(0);
    }
}

void init_ui()
{
    for (i = 0; i <= 1; i++)
    {
        term_input_col[i] = strlen(msg_input_prompt) + 3;
    }
    fast_printf(true, true, SETSCREENSIZE, screen_height, screen_width);
    fast_printf(true, true, DECRESETMODE, 47);
    fast_printf(true, true, CLEARDISPLAY);
    fast_printf(true, true, RESETSGR);
    fast_printf(true, true, RESETSCROLLROWS);
    fast_printf(true, true, DECSETMODE, 19);
    fast_printf(true, true, "\x1b[47;30mRuntime 00:00:00\x1b[K\x1b[0m\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[0]);
    fast_printf(true, true, CHANGESCROLLROWS, 2, screen_height - 4);
    fast_printf(true, true, DECRESETMODE, 19);
    fast_printf(true, true, CURSORINVISIBLE);
    fast_printf(true, true, CURSORMOVE, 2, 1);
    fast_printf(true, true, "┌"); // top left corner
    for (i = 1; i <= screen_width - 2; i++)
        fast_printf(true, true, "─"); // top border
    fast_printf(true, true, "┐");     // top right corner
    for (i = 3; i <= screen_height - 4; i++)
    {
        fast_printf(true, true, CURSORMOVE, i, 1);
        fast_printf(true, true, "│"); // vertical left border
        fast_printf(true, true, CURSORMOVE, i, screen_width);
        fast_printf(true, true, "│"); // vertical right border
    }
    fast_printf(true, true, CURSORMOVE, screen_height - 3, 1);
    fast_printf(true, true, "├"); // middle left corner
    for (i = 1; i <= screen_width - 2; i++)
    {
        if (i == term_input_col[0] - 2)
            fast_printf(true, true, "┬"); // middle down border
        else
            fast_printf(true, true, "─"); // middle border
    }
    fast_printf(true, true, "┤"); // middle right corner
    fast_printf(true, true, CURSORMOVE, screen_height - 2, 1);
    fast_printf(true, true, "│"); // vertical left border
    fast_printf(true, true, CURSORMOVE, screen_height - 2, screen_width);
    fast_printf(true, true, "│"); // vertical right border
    fast_printf(true, true, CURSORMOVE, screen_height - 1, 1);
    fast_printf(true, true, "└"); // bottom left corner
    for (i = 1; i <= screen_width - 2; i++)
    {
        if (i == term_input_col[0] - 2)
            fast_printf(true, true, "┴"); // bottom up border
        else
            fast_printf(true, true, "─"); // bottom border
    }
    fast_printf(true, true, "┘"); // bottom right corner
    fast_printf(true, true, CURSORMOVE, screen_height - 2, 2);
    fast_printf(true, true, "\x1b[1m%s\x1b[0m", msg_input_prompt);
    fast_printf(true, true, "│"); // vertical middle border
    fast_printf(true, true, CURSORVISIBLE);
}

void main()
{
    init_kernel();
    init_io();
    init_ui();
    set_task(runtime_clock);
    set_task(command_bar);
    set_task(main_app);
    begin_sch();
}