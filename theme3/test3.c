#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mtk_c.h"

// Control Characters
#define BACKSPACE "\x7f"           // Backspace character
#define ENTER "\x0d"               // Enter/Return character
#define ESC "\x1b"                 // Escape character

// Cursor Control
#define LOCATEHOME ESC "[H"        // Move cursor to the home position
#define LOCATECURSOR ESC "[%d;%df" // Move cursor to a specific position
#define CURSORMOVE ESC "[%d;%dH"   // Move cursor to row %d, column %d
#define CURSORMOVENUP ESC "[%dA"   // Move cursor up by %d lines
#define CURSORMOVENDOWN ESC "[%dB" // Move cursor down by %d lines
#define CURSORMOVENRIGHT ESC "[%dC"// Move cursor right by %d columns
#define CURSORMOVENLEFT ESC "[%dD" // Move cursor left by %d columns
#define CURSORINVISIBLE ESC "[?25l"// Make cursor invisible
#define CURSORVISIBLE ESC "[?25h"  // Make cursor visible
#define SAVECURSORLOC ESC "7"      // Save current cursor position
#define RETCURSORLOC ESC "8"       // Restore saved cursor position

// Screen Control
#define SETSCREENSIZE ESC "[8;%d;%dt" // Set screen size (rows and columns)
#define CLEARDISPLAY ESC "[H" ESC "[2J" // Clear entire screen and move cursor to home
#define DELETELINEAFTERCURSOR ESC "[0K" // Delete line content after cursor
#define DELETELINEBEFORECURSOR ESC "[1K" // Delete line content before cursor
#define DELETELINEECURSOR ESC "[2K"     // Delete entire line content at cursor
#define DELETESCREENAFTERCURSOR ESC "[0J" // Clear screen after the cursor
#define DELETESCREENBEFORECURSOR ESC "[1J" // Clear screen before the cursor
#define DELETESCREEN ESC "[2J"           // Clear the entire screen
#define CHANGESCROLLROWS ESC "[%d;%dr"   // Set scrollable region of the screen
#define RESETSCROLLROWS ESC "[r"         // Reset scrollable region

// Line and Character Deletion
#define DELETENCHAR ESC "[%dP"          // Delete %d characters at cursor

// Color and Graphics Rendition
#define SGR ESC "[%dm"                  // Select graphic rendition (e.g., text attributes)
#define RESETSGR ESC "[0m"              // Reset graphic rendition to default
#define SGRCOL256 ESC "[38;5;%dm"       // Set 256-color foreground
#define SGRBGCOL256 ESC "[48;5;%dm"     // Set 256-color background
#define SGRCOL24B ESC "[38;2;%d;%d;%dm" // Set 24-bit RGB foreground
#define SGRBGCOL24B ESC "[48;2;%d;%d;%dm" // Set 24-bit RGB background

// Character Set Switching
#define DECLINEDRAW ESC "(0"            // Switch to DEC line drawing character set
#define ASCIIDRAW ESC "(B"              // Switch to ASCII character set

// Mode Control
#define DECSETMODE ESC "[?%dh"          // Set terminal mode
#define DECRESETMODE ESC "[?%dl"        // Reset terminal mode
#define SETLRMARGIN ESC "[?69h"         // Enable left/right margin mode

// Terminal Attributes
#define DECTCEM 25                      // Terminal cursor enable mode (use with DECSETMODE/DECRESETMODE)

// Miscellaneous
#define MAX 1024                        // Maximum buffer size
#define SECONDDIVIDER 100               // Arbitrary constant, likely for time conversions
#define BUFLEN 100                      // Buffer length
#define TOTALTIPS 4                     // Total number of tips

/*
// Shared Resource Variables (Used in Multiple Tasks)

// Used in command_bar(), main_app()
bool inappcom[2] = {true, true};       // Track if clients are in application communication mode
char keypress[2] = {-1, -1};           // Last keypress for each client
char last_keybuf[2][10];               // Buffer for storing last key sequences
int last_keybuf_len[2];                // Length of the last key buffer
char msg_buf[2][BUFLEN];               // Buffer for storing messages
unsigned int msg_buf_len[2];           // Length of the message buffer
char cmd_buf[2][BUFLEN];               // Buffer for storing commands
unsigned int cmd_buf_len[2];           // Length of the command buffer
int tip_num[2];                        // Indicates the current tip being displayed for each client.

// Used in runtime_clock(), command_bar(), main_app()
unsigned int screen_height = 35;       // Height of the screen
unsigned int screen_width = 203;       // Width of the screen

// Used in runtime_clock(), main_app()
unsigned int chat_row = 1;             // Row for chat content display
*/

// Global Variables and Constants

// Indices for managing two clients (Client1 and Client2)
int i, j; 

// Screen and Mode States
bool in_alt_screen[2] = {false, false}; // Track if clients are in the alternate screen
bool inappcom[2] = {true, true};       // Track if clients are in application communication mode

// Input Tracking
char keypress[2] = {-1, -1};           // Last keypress for each client
char last_keybuf[2][10];               // Buffer for storing last key sequences
int last_keybuf_len[2];                // Length of the last key buffer

// Client Nicknames
char client_nickname_color[2] = {11, 14}; // Color assigned to user nicknames
char client_nickname[2][10] = {"USER1", "USER2"}; // Default user nicknames
bool client_nickname_set[2] = {false, false}; // Track if nicknames are set

// Terminal Input Handling
int term_input_col[2]; // Current input column for each client

// Message Buffers
char msg_buf[2][BUFLEN];          // Buffer for storing messages
unsigned int msg_buf_len[2];      // Length of the message buffer
char cmd_buf[2][BUFLEN];          // Buffer for storing commands
unsigned int cmd_buf_len[2];      // Length of the command buffer
char printf_buf[BUFLEN];          // Buffer for formatted print outputs

// Tip Management
int tip_num[2];                   // Index of the current tip for each client

// Screen Configuration
unsigned int screen_height = 35;  // Height of the screen
unsigned int screen_width = 203;  // Width of the screen
unsigned int chat_row = 1;        // Row for chat content display

// Terminal Control Sequences
const char term_backspace[] = "\b \b";  // Backspace sequence for terminal
const char term_line10[] = "\x1b[10;H"; // Move to line 10

// Prompts
const char msg_input_prompt[] = "Your message"; // Prompt for message input
const char command_prompt[] = "Command: ";      // Prompt for command input

// Tips
const char tip_list[TOTALTIPS][BUFLEN] = {
    "Press ESC to enter command mode",               // Tip 1
    "Maximum length of each message is 100 characters", // Tip 2
    "Functionality of the arrow keys is suppressed", // Tip 3
    "Only help command is available for now"         // Tip 4
};

// Help Text
const char help_text[] = 
    "\n\x1b[1;4mManual page for the chat application\x1b[0m\n\n"
    "This page should be populated when the application is finished.\n"
    "Please press any key to go back.\n";

// Fast printf function for output to specific file descriptors
void fast_printf(const bool ch0, const bool ch1, const char *fmt, ...)
{
    // Declare a variable to store the length of the formatted string
    int printf_buf_len;

    // Initialize a variable argument list to process variadic arguments
    va_list args;
    va_start(args, fmt); // Start processing arguments after 'fmt'

    // Format the input string and arguments into 'printf_buf'
    vsprintf(printf_buf, fmt, args);

    // Calculate the length of the formatted string
    printf_buf_len = strlen(printf_buf);

    // Write the formatted string to file descriptor 3 if 'ch0' is true
    if (ch0)
        write(3, &printf_buf, printf_buf_len);

    // Write the formatted string to file descriptor 4 if 'ch1' is true
    if (ch1)
        write(4, &printf_buf, printf_buf_len);

    // Clean up the variable argument list
    va_end(args);
}

// Function to continuously update and display the runtime clock
void runtime_clock()
{
    // Initialize time variables
    unsigned int hr = 0, min = 0, sec = 0, last_runtime = 0;

    // Infinite loop to keep the runtime clock running
    while (1)
    {
        // Lock semaphores to ensure safe access to shared resources
        P(0);
        P(1);

        // Check if a second has passed since the last update
        if (runtime - last_runtime >= SECONDDIVIDER)
        {
            // Update seconds based on the elapsed runtime
            sec += (runtime - last_runtime) / SECONDDIVIDER;
            last_runtime = runtime; // Update the last runtime checkpoint

            // Handle time rollover: seconds to minutes
            if (sec == 60)
            {
                sec = sec - 60;
                min++;

                // Handle time rollover: minutes to hours
                if (min == 60)
                {
                    min = 0;
                    hr++;
                }
            }

            // Update the screen with the new runtime
            fast_printf(true, true, SETSCREENSIZE, screen_height, screen_width); // Adjust screen size if needed
            fast_printf(true, true, CURSORINVISIBLE); // Hide cursor during update
            fast_printf(true, true, SAVECURSORLOC);   // Save current cursor position

            // Display the runtime in HH:MM:SS format at the top-left corner
            fast_printf(true, true, "\x1b[H\x1b[47;30mRuntime %02d:%02d:%02d \x1b[K\x1b[0m", hr, min, sec);

            fast_printf(true, true, RETCURSORLOC);   // Restore cursor position
            fast_printf(true, true, CURSORVISIBLE); // Make cursor visible again
        }

        // Release semaphores to allow other tasks to proceed
        V(1);
        V(0);
    }
}

// Command bar function to handle user inputs and display tips
void command_bar()
{
    int last_tip_changed[2]; // Array to track when the tip was last changed for each channel
    int some_num2; // Random value to add variability to tip update intervals

    // A command bar at the bottom of the screen; accepts commands after the user presses the Escape key
    while (1)
    {
        P(0); // Lock semaphore 0 for safe access to shared resources
        P(1); // Lock semaphore 1 for additional synchronization

        some_num2 = rand() %100 ; // Generate a random delay modifier for tip updates

        // Process input for each channel (0 and 1)
        for (i = 0; i <= 1; i++)
        {
            if (!inappcom[i]) // If the user is not in command input mode
            {
                keypress[i] = inkey(i); // Get the keypress input for the channel

                if (keypress[i] != -1) // If a key was pressed
                {
                    if (in_alt_screen[i]) // If the alternate screen is active
                    {
                        fast_printf(i == 0, i == 1, DECRESETMODE, 47); // Exit alternate screen mode
                        in_alt_screen[i] = false;
                        fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K", command_prompt); // Display the command prompt
                    }
                    else
                    {
                        // Add the keypress to the buffer
                        last_keybuf[i][last_keybuf_len[i]] = keypress[i];
                        last_keybuf_len[i]++;
                        last_keybuf[i][last_keybuf_len[i]] = '\0';
                    }
                }
                else if (last_keybuf_len[i] > 0) // If there is data in the key buffer
                {
                    if (strcmp(last_keybuf[i], ESC) == 0) // If the Escape key is pressed
                    {
                        fast_printf(i == 0, i == 1, RESETSGR); // Reset text formatting
                        fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]); // Display current tip
                        fast_printf(i == 0, i == 1, CURSORMOVE, screen_height - 2, 2); // Move to the input prompt position
                        fast_printf(i == 0, i == 1, "\x1b[1m%s\x1b[0m│%s", msg_input_prompt, msg_buf[i]); // Display input prompt
                        inappcom[i] = true; // Enter command input mode
                        cmd_buf_len[i] = 0; // Reset the command buffer
                        cmd_buf[i][0] = '\0';
                    }
                    else if (strcmp(last_keybuf[i], BACKSPACE) == 0) // If the Backspace key is pressed
                    {
                        if (cmd_buf_len[i] > 0) // Ensure the buffer is not empty
                        {
                            fast_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", term_backspace); // Handle backspace visually
                            cmd_buf_len[i]--; // Remove the last character from the buffer
                            cmd_buf[i][cmd_buf_len[i]] = '\0';
                        }
                    }
                    else if (strcmp(last_keybuf[i], ENTER) == 0) // If the Enter key is pressed
                    {
                        if (cmd_buf_len[i] > 0) // Ensure there is a command in the buffer
                        {
                            // Check if the command is 'h' or 'help'
                            if (strncmp(cmd_buf[i], "h", 1) == 0 && cmd_buf_len[i] == 1 || 
                                strncmp(cmd_buf[i], "help", 4) == 0 && cmd_buf_len[i] == 4)
                            {
                                fast_printf(i == 0, i == 1, DECSETMODE, 47); // Switch to alternate screen mode
                                fast_printf(i == 0, i == 1, RESETSGR); // Reset text formatting
                                fast_printf(i == 0, i == 1, CLEARDISPLAY); // Clear the screen
                                fast_printf(i == 0, i == 1, help_text); // Display help text
                                in_alt_screen[i] = true; // Indicate alternate screen is active
                                cmd_buf_len[i] = 0; // Clear the command buffer
                                cmd_buf[i][0] = '\0';
                            }
                            else // Handle unrecognized commands
                            {
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", command_prompt); // Redisplay the command prompt
                                cmd_buf_len[i] = 0; // Clear the command buffer
                                cmd_buf[i][0] = '\0';
                            }
                        }
                    }
                    else if (last_keybuf_len[i] == 1 && last_keybuf[i][0] >= 32 && last_keybuf[i][0] <= 126) // Printable characters
                    {
                        if (cmd_buf_len[i] < 100) // Ensure the buffer does not exceed 100 characters
                        {
                            cmd_buf[i][cmd_buf_len[i]] = last_keybuf[i][0]; // Add character to the buffer
                            cmd_buf_len[i]++;
                            cmd_buf[i][cmd_buf_len[i]] = '\0';
                            fast_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", last_keybuf[i]); // Display the character
                        }
                        else if (!(last_keybuf_len[i] == 3 && last_keybuf[i][0] == 27 && last_keybuf[i][1] == 91 && last_keybuf[i][2] >= 65 && last_keybuf[i][2] <= 68)) // Ignore arrow keys
                        {
                            write(i + 3, &last_keybuf[i], last_keybuf_len[i]); // Write the keypress to the appropriate output
                        }
                    }
                    last_keybuf[i][0] = '\0'; // Clear the key buffer
                    last_keybuf_len[i] = 0;
                }
                keypress[i] = -1; // Reset keypress
            }
            else // If the user is in command input mode
            {
                // Handle tip updates
               	if (runtime - last_tip_changed[i] >= 800 + some_num2)
                {
                    if (tip_num[i] == TOTALTIPS - 1)
                        tip_num[i] = 0; // Reset to the first tip
                    else
                        tip_num[i]++; // Move to the next tip

                    last_tip_changed[i] = runtime; // Update the last tip change timestamp
                    fast_printf(i == 0, i == 1, CURSORINVISIBLE); // Hide the cursor
                    fast_printf(i == 0, i == 1, SAVECURSORLOC); // Save the cursor position
                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]); // Display the tip
                    fast_printf(i == 0, i == 1, RETCURSORLOC); // Restore the cursor position
                    fast_printf(i == 0, i == 1, CURSORVISIBLE); // Make the cursor visible again
                }
            }
        }
        V(1); // Release semaphore 1
        V(0); // Release semaphore 0
    }
}

// Main application loop to handle user input and message processing
void main_app()
{
    bool warn_length[2] = {false, false}; // Tracks if a warning for maximum length was displayed
    bool warn_empty[2] = {false, false};  // Tracks if a warning for empty messages was displayed
    int some_num1; // Random variable for potential delays or additional functionality

    // Infinite loop to process user input and messages
    while (1)
    {
        P(0); // Lock semaphore 0 for safe resource access
        P(1); // Lock semaphore 1 for additional synchronization

        some_num1 = rand() %100 ; 

        // Process input for each channel (0 and 1)
        for (i = 0; i <= 1; i++)
        {
            if (inappcom[i]) // If the user is in command input mode
            {
                keypress[i] = inkey(i); // Get the keypress for the channel

                if (keypress[i] != -1) // If a key is pressed
                {
                    // Add the keypress to the key buffer
                    last_keybuf[i][last_keybuf_len[i]] = keypress[i];
                    last_keybuf_len[i]++;
                    last_keybuf[i][last_keybuf_len[i]] = '\0';
                }
                else
                {
                    if (last_keybuf_len[i] > 0) // If there is data in the key buffer
                    {
                        if (strcmp(last_keybuf[i], ESC) == 0) // If Escape key is pressed
                        {
                            fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", command_prompt); // Reset the command prompt
                            inappcom[i] = false; // Exit command input mode
                        }
                        else if (strcmp(last_keybuf[i], BACKSPACE) == 0) // If Backspace is pressed
                        {
                            if (msg_buf_len[i] > 0) // Ensure the message buffer is not empty
                            {
                                write(i + 3, &term_backspace, 3); // Handle backspace visually
                                msg_buf_len[i]--; // Remove the last character from the buffer
                                msg_buf[i][msg_buf_len[i]] = '\0';
                            }
                        }
                        else if (strcmp(last_keybuf[i], ENTER) == 0) // If Enter is pressed
                        {
                            if (msg_buf_len[i] > 0) // If the message buffer contains data
                            {
                                fast_printf(true, true, DECRESETMODE, 47); // Reset the screen mode
                                if (warn_empty[i]) // If an empty message warning was displayed
                                {
                                    fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                    fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]); // Reset the tip
                                    fast_printf(i == 0, i == 1, RETCURSORLOC);
                                    fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                    warn_empty[i] = false; // Clear the warning flag
                                }
                                // Display the message in the chat area
                                fast_printf(true, true, CURSORINVISIBLE);
                                fast_printf(true, true, CURSORMOVE, 1, 1);
                                for (j = 0; j < chat_row; j++)
                                    printf_buf[j] = '\n';
                                printf_buf[j + 1] = '\0';
                                write(3, &printf_buf, chat_row); // Write the new message to channel 3
                                write(4, &printf_buf, chat_row); // Write the new message to channel 4
                                if (chat_row < screen_height - 5) // Increment chat row if within limits
                                    chat_row++;
                                fast_printf(true, true, "\x1b[0m\n│\x1b[1;38;5;%dm%s: \x1b[0m", client_nickname_color[i], client_nickname[i]); // Display the sender's nickname
                                write(3, &msg_buf[i], msg_buf_len[i]); // Write the message content
                                write(4, &msg_buf[i], msg_buf_len[i]);
                                fast_printf(true, true, "\x1b[%d;999H│", chat_row + 1); // Update the chat window
                                msg_buf_len[i] = 0; // Clear the message buffer
                                msg_buf[i][0] = '\0';

                                // Reset the input area
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
                                fast_printf(true, true, CURSORVISIBLE); // Make the cursor visible
                            }
                            else // If the message buffer is empty
                            {
                                // Display a warning about empty messages
                                fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mSending Empty message is not allowed\x1b[K\x1b[0m");
                                fast_printf(i == 0, i == 1, RETCURSORLOC);
                                fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                warn_empty[i] = true; // Set the warning flag
                            }
                            // Restore the alternate screen mode
                            fast_printf(in_alt_screen[0], in_alt_screen[1], DECSETMODE, 47);
                            fast_printf(in_alt_screen[0], in_alt_screen[1], CURSORMOVE, 1, 1);
                            fast_printf(in_alt_screen[0], in_alt_screen[1], help_text);
                        }
                        else if (last_keybuf_len[i] == 1 && last_keybuf[i][0] >= 32 && last_keybuf[i][0] <= 126) // Printable characters
                        {
                            if (msg_buf_len[i] < 100) // Ensure the buffer does not exceed the limit
                            {
                                msg_buf[i][msg_buf_len[i]] = last_keybuf[i][0]; // Add character to the buffer
                                msg_buf_len[i]++;
                                msg_buf[i][msg_buf_len[i]] = '\0';
                                write(i + 3, &last_keybuf[i], last_keybuf_len[i]); // Write the keypress to the output
                                if (warn_length[i]) // If a length warning was displayed
                                {
                                    fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                    fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                    fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[tip_num[i]]); // Reset the tip
                                    fast_printf(i == 0, i == 1, RETCURSORLOC);
                                    fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                    warn_length[i] = false; // Clear the warning flag
                                }
                            }
                            else // If the buffer is full
                            {
                                // Display a warning about maximum length
                                fast_printf(i == 0, i == 1, CURSORINVISIBLE);
                                fast_printf(i == 0, i == 1, SAVECURSORLOC);
                                fast_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mMaximum character limit per message reached (%d characters)\x1b[K\x1b[0m", BUFLEN);
                                fast_printf(i == 0, i == 1, RETCURSORLOC);
                                fast_printf(i == 0, i == 1, CURSORVISIBLE);
                                warn_length[i] = true; // Set the warning flag
                            }
                        }
                        else if (!(last_keybuf_len[i] == 3 && last_keybuf[i][0] == 27 && last_keybuf[i][1] == 91 && last_keybuf[i][2] >= 65 && last_keybuf[i][2] <= 68))
                        {
                            // Ignore arrow keys
                            write(i + 3, &last_keybuf[i], last_keybuf_len[i]);
                        }
                        last_keybuf[i][0] = '\0'; // Clear the key buffer
                        last_keybuf_len[i] = 0;
                    }
                }
                keypress[i] = -1; // Reset the keypress
            }
        }
        V(1); // Release semaphore 1
        V(0); // Release semaphore 0
    }
}

// Initialize the user interface and layout of the terminal
void init_ui()
{
    // Set initial positions for input columns based on the message input prompt length
    for (i = 0; i <= 1; i++)
    {
        term_input_col[i] = strlen(msg_input_prompt) + 3;
    }

    // Configure terminal screen settings
    fast_printf(true, true, SETSCREENSIZE, screen_height, screen_width); // Set screen size
    fast_printf(true, true, DECRESETMODE, 47); // Reset scroll mode
    fast_printf(true, true, CLEARDISPLAY); // Clear the terminal display
    fast_printf(true, true, RESETSGR); // Reset text formatting
    fast_printf(true, true, RESETSCROLLROWS); // Reset scroll rows
    fast_printf(true, true, DECSETMODE, 19); // Enable a specific terminal mode

    // Display runtime and initial tip
    fast_printf(true, true, "\x1b[47;30mRuntime 00:00:00\x1b[K\x1b[0m\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tip_list[0]);

    // Configure scrolling region
    fast_printf(true, true, CHANGESCROLLROWS, 2, screen_height - 4);
    fast_printf(true, true, DECRESETMODE, 19); // Reset terminal mode
    fast_printf(true, true, CURSORINVISIBLE); // Make the cursor invisible

    // Draw the top border of the UI
    fast_printf(true, true, CURSORMOVE, 2, 1);
    fast_printf(true, true, "┌"); // Top left corner
    for (i = 1; i <= screen_width - 2; i++)
        fast_printf(true, true, "─"); // Top horizontal border
    fast_printf(true, true, "┐"); // Top right corner

    // Draw the vertical borders
    for (i = 3; i <= screen_height - 4; i++)
    {
        fast_printf(true, true, CURSORMOVE, i, 1);
        fast_printf(true, true, "│"); // Left vertical border
        fast_printf(true, true, CURSORMOVE, i, screen_width);
        fast_printf(true, true, "│"); // Right vertical border
    }

    // Draw the divider above the message input box
    fast_printf(true, true, CURSORMOVE, screen_height - 3, 1);
    fast_printf(true, true, "├"); // Left corner of the divider
    for (i = 1; i <= screen_width - 2; i++)
    {
        if (i == term_input_col[0] - 2)
            fast_printf(true, true, "┬"); // Intersection for input column
        else
            fast_printf(true, true, "─"); // Horizontal divider
    }
    fast_printf(true, true, "┤"); // Right corner of the divider

    // Draw the input box borders
    fast_printf(true, true, CURSORMOVE, screen_height - 2, 1);
    fast_printf(true, true, "│"); // Left border of the input box
    fast_printf(true, true, CURSORMOVE, screen_height - 2, screen_width);
    fast_printf(true, true, "│"); // Right border of the input box

    // Draw the bottom border of the UI
    fast_printf(true, true, CURSORMOVE, screen_height - 1, 1);
    fast_printf(true, true, "└"); // Bottom left corner
    for (i = 1; i <= screen_width - 2; i++)
    {
        if (i == term_input_col[0] - 2)
            fast_printf(true, true, "┴"); // Intersection for input column
        else
            fast_printf(true, true, "─"); // Bottom horizontal border
    }
    fast_printf(true, true, "┘"); // Bottom right corner

    // Position and display the message input prompt
    fast_printf(true, true, CURSORMOVE, screen_height - 2, 2);
    fast_printf(true, true, "\x1b[1m%s\x1b[0m", msg_input_prompt);
    fast_printf(true, true, "│"); // Vertical middle border of the input box

    // Make the cursor visible again
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
