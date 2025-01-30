#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include "mtk_c.h"

// Control Characters
#define BS "\x7f"           // Backspace character
#define ENT "\x0d"          // Enter/Return character
#define ESC "\x1b"          // Escape character

// Cursor Control
#define LH ESC "[H"         // Move cursor to the home position
#define LC ESC "[%d;%df"    // Move cursor to a specific position
#define CM ESC "[%d;%dH"    // Move cursor to row %d, column %d
#define CMU ESC "[%dA"     // Move cursor up by %d lines
#define CMD ESC "[%dB"    // Move cursor down by %d lines
#define CMR ESC "[%dC"// Move cursor right by %d columns
#define CML ESC "[%dD" // Move cursor left by %d columns
#define CI ESC "[?25l"// Make cursor invisible
#define CV ESC "[?25h"  // Make cursor visible
#define SC ESC "7"      // Save current cursor position
#define RC ESC "8"       // Restore saved cursor position

// Screen Control
#define SS ESC "[8;%d;%dt" // Set screen size (rows and columns)
#define CD ESC "[H" ESC "[2J" // Clear entire screen and move cursor to home
#define DLAC ESC "[0K" // Delete line content after cursor
#define DLBC ESC "[1K" // Delete line content before cursor
#define DLC ESC "[2K"     // Delete entire line content at cursor
#define DSAC ESC "[0J" // Clear screen after the cursor
#define DSBC ESC "[1J" // Clear screen before the cursor
#define DS ESC "[2J"           // Clear the entire screen
#define CSR ESC "[%d;%dr"   // Set scrollable region of the screen
#define RSR ESC "[r"         // Reset scrollable region

// Line and Character Deletion
#define DC ESC "[%dP"          // Delete %d characters at cursor

// Color and Graphics Rendition
#define SG ESC "[%dm"                  // Select graphic rendition (e.g., text attributes)
#define RS ESC "[0m"              // Reset graphic rendition to default
#define SF256 ESC "[38;5;%dm"       // Set 256-color foreground
#define SB256 ESC "[48;5;%dm"     // Set 256-color background
#define SF24 ESC "[38;2;%d;%d;%dm" // Set 24-bit RGB foreground
#define SB24 ESC "[48;2;%d;%d;%dm" // Set 24-bit RGB background

// Character Set Switching
#define DD ESC "(0"            // Switch to DEC line drawing character set
#define AD ESC "(B"              // Switch to ASCII character set

// Mode Control
#define DSM ESC "[?%dh"          // Set terminal mode
#define DRM ESC "[?%dl"        // Reset terminal mode
#define SM ESC "[?69h"         // Enable left/right margin mode

// Terminal Attributes
#define DEM 25                      // Terminal cursor enable mode (use with DSM/DRM)

// Miscellaneous
#define M 1024                        // Maximum buffer size
#define SD 100               // Arbitrary constant, likely for time conversions
#define BL 100                      // Buffer length
#define TT 4                     // Total number of tips

/*
// Shared Resource Variables (Used in Multiple Tasks)

// Used in bar(), mainapp()
bool inapp[2] = {true, true};       
char kp[2] = {-1, -1};          
char lkb[2][10];               
int lkbl[2];                
char mb[2][BL];               
unsigned int mbl[2];           
char cb[2][BL];               
unsigned int cbl[2];           
int tn[2];                        
// Used in clock(), bar(), mainapp()
unsigned int sh = 35;       
unsigned int sw = 203;       

// Used in clock(), mainapp()
unsigned int cr = 1;            

*/

// Global Variables and Constants

// Indices for managing two users (user1 and user2)
int i, j; 

// Screen and Mode States
bool inalt[2] = {false, false}; // Track if users are in the alternate screen
bool inapp[2] = {true, true};       // Track if users are in application communication mode

// Input Tracking
char kp[2] = {-1, -1};           // Last keypress for each user
char lkb[2][10];               // Buffer for storing last key sequences
int lkbl[2];                // Length of the last key buffer

// User Nicknames
char unc[2] = {11, 14}; // Color assigned to user nicknames
char un[2][10] = {"USER1", "USER2"}; // Default user nicknames
bool uns[2] = {false, false}; // Track if nicknames are set

// Terminal Input Handling
int tic[2]; // Current input column for each user

// Message Buffers
char mb[2][BL];          // Buffer for storing messages
unsigned int mbl[2];      // Length of the message buffer
char cb[2][BL];          // Buffer for storing commands
unsigned int cbl[2];      // Length of the command buffer
char pb[BL];          // Buffer for formatted print outputs

// Tip Management
int tn[2];                   // Index of the current tip for each user

// Screen Configuration
unsigned int sh = 35;  // Height of the screen
unsigned int sw = 203;  // Width of the screen
unsigned int cr = 1;        // Row for chat content display

// Terminal Control Sequences
const char tbs[] = "\b \b";  // Backspace sequence for terminal
const char tl10[] = "\x1b[10;H"; // Move to line 10

// Prompts
const char mip[] = "Your message"; // Prompt for message input
const char cp[] = "Command: ";      // Prompt for command input

// Tips
const char tl[TT][BL] = {
    "Press ESC to enter command mode",               // Tip 1
    "Maximum length of each message is 100 characters", // Tip 2
    "Functionality of the arrow keys is suppressed", // Tip 3
    "Only help command is available for now"         // Tip 4
};

// Help Text
const char ht[] = 
    "\n\x1b[1;4mManual page for the chat application\x1b[0m\n\n"
    "This page should be populated when the application is finished.\n"
    "Please press any key to go back.\n";

// special printf function for output to specific file descriptors
void special_printf(const bool ch0, const bool ch1, const char *fmt, ...)
{
    // Declare a variable to store the length of the formatted string
    int pbl;

    // Initialize a variable argument list to process variadic arguments
    va_list args;
    va_start(args, fmt); // Start processing arguments after 'fmt'

    // Format the input string and arguments into 'pb'
    vsprintf(pb, fmt, args);

    // Calculate the length of the formatted string
    pbl = strlen(pb);

    // Write the formatted string to file descriptor 3 if 'ch0' is true
    if (ch0)
        write(3, &pb, pbl);

    // Write the formatted string to file descriptor 4 if 'ch1' is true
    if (ch1)
        write(4, &pb, pbl);

    // Clean up the variable argument list
    va_end(args);
}

// Function to continuously update and display the runtime clock
void clock()
{
    // Initialize time variables
    unsigned int h = 0, m = 0, s = 0, lastruntime = 0;

    // Infinite loop to keep the runtime clock running
    while (1)
    {
        // Lock semaphores to ensure safe access to shared resources
        P(0);
        P(1);

        // Check if a sond has passed since the last update
        if (runtime - lastruntime >= SD)
        {
            // Update sonds based on the elapsed runtime
            s += (runtime - lastruntime) / SD;
            lastruntime = runtime; // Update the last runtime checkpoint

            // Handle time rollover: sonds to minutes
            if (s == 60)
            {
                s = s - 60;
                m++;

                // Handle time rollover: minutes to hours
                if (m == 60)
                {
                    m = 0;
                    h++;
                }
            }

            // Update the screen with the new runtime
            special_printf(true, true, SS, sh, sw); // Adjust screen size if needed
            special_printf(true, true, CI); // Hide cursor during update
            special_printf(true, true, SC);   // Save current cursor position

            // Display the runtime in HH:MM:SS format at the top-left corner
            special_printf(true, true, "\x1b[H\x1b[47;30mRuntime %02d:%02d:%02d \x1b[K\x1b[0m", h, m, s);

            special_printf(true, true, RC);   // Restore cursor position
            special_printf(true, true, CV); // Make cursor visible again
        }

        // Release semaphores to allow other tasks to proceed
        V(1);
        V(0);
    }
}

// Command bar function to handle user inputs and display tips
void bar()
{
    int ltc[2]; // Array to track when the tip was last changed for each channel
    int num1; // Random value to add variability to tip update intervals

    // A command bar at the bottom of the screen; accepts commands after the user presses the Escape key
    while (1)
    {
        P(0); // Lock semaphore 0 for safe access to shared resources
        P(1); // Lock semaphore 1 for additional synchronization

        num1 = rand() %100 ; // Generate a random delay modifier for tip updates

        // Process input for each channel (0 and 1)
        for (i = 0; i <= 1; i++)
        {
            if (!inapp[i]) // If the user is not in command input mode
            {
                kp[i] = inkey(i); // Get the kp input for the channel

                if (kp[i] != -1) // If a key was pressed
                {
                    if (inalt[i]) // If the alternate screen is active
                    {
                        special_printf(i == 0, i == 1, DRM, 47); // Exit alternate screen mode
                        inalt[i] = false;
                        special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K", cp); // Display the command prompt
                    }
                    else
                    {
                        // Add the kp to the buffer
                        lkb[i][lkbl[i]] = kp[i];
                        lkbl[i]++;
                        lkb[i][lkbl[i]] = '\0';
                    }
                }
                else if (lkbl[i] > 0) // If there is data in the key buffer
                {
                    if (strcmp(lkb[i], ESC) == 0) // If the Escape key is pressed
                    {
                        special_printf(i == 0, i == 1, RS); // Reset text formatting
                        special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tl[tn[i]]); // Display current tip
                        special_printf(i == 0, i == 1, CM, sh - 2, 2); // Move to the input prompt position
                        special_printf(i == 0, i == 1, "\x1b[1m%s\x1b[0m│%s", mip, mb[i]); // Display input prompt
                        inapp[i] = true; // Enter command input mode
                        cbl[i] = 0; // Reset the command buffer
                        cb[i][0] = '\0';
                    }
                    else if (strcmp(lkb[i], BS) == 0) // If the Backspace key is pressed
                    {
                        if (cbl[i] > 0) // Ensure the buffer is not empty
                        {
                            special_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", tbs); // Handle backspace visually
                            cbl[i]--; // Remove the last character from the buffer
                            cb[i][cbl[i]] = '\0';
                        }
                    }
                    else if (strcmp(lkb[i], ENT) == 0) // If the Enter key is pressed
                    {
                        if (cbl[i] > 0) // Ensure there is a command in the buffer
                        {
                            // Check if the command is 'help'
                            if (strncmp(cb[i], "h", 1) == 0 && cbl[i] == 1 || 
                                strncmp(cb[i], "help", 4) == 0 && cbl[i] == 4)
                            {
                                special_printf(i == 0, i == 1, DSM, 47); // Switch to alternate screen mode
                                special_printf(i == 0, i == 1, RS); // Reset text formatting
                                special_printf(i == 0, i == 1, CD); // Clear the screen
                                special_printf(i == 0, i == 1, ht); // Display help text
                                inalt[i] = true; // Indicate alternate screen is active
                                cbl[i] = 0; // Clear the command buffer
                                cb[i][0] = '\0';
                            }
                            else // Handle unrecognized commands
                            {
                                special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", cp); // Redisplay the command prompt
                                cbl[i] = 0; // Clear the command buffer
                                cb[i][0] = '\0';
                            }
                        }
                    }
                    else if (lkbl[i] == 1 && lkb[i][0] >= 32 && lkb[i][0] <= 126) // Printable characters
                    {
                        if (cbl[i] < 100) // Ensure the buffer does not exceed 100 characters
                        {
                            cb[i][cbl[i]] = lkb[i][0]; // Add character to the buffer
                            cbl[i]++;
                            cb[i][cbl[i]] = '\0';
                            special_printf(i == 0, i == 1, "\x1b[47;30m%s\x1b[0m", lkb[i]); // Display the character
                        }
                        else if (!(lkbl[i] == 3 && lkb[i][0] == 27 && lkb[i][1] == 91 && lkb[i][2] >= 65 && lkb[i][2] <= 68)) // Ignore arrow keys
                        {
                            write(i + 3, &lkb[i], lkbl[i]); // Write the kp to the appropriate output
                        }
                    }
                    lkb[i][0] = '\0'; // Clear the key buffer
                    lkbl[i] = 0;
                }
                kp[i] = -1; // Reset kp
            }
            else // If the user is in command input mode
            {
                // Handle tip updates
               	if (runtime - ltc[i] >= 800 + num1)
                {
                    if (tn[i] == TT - 1)
                        tn[i] = 0; // Reset to the first tip
                    else
                        tn[i]++; // Move to the next tip

                    ltc[i] = runtime; // Update the last tip change timestamp
                    special_printf(i == 0, i == 1, CI); // Hide the cursor
                    special_printf(i == 0, i == 1, SC); // Save the cursor position
                    special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tl[tn[i]]); // Display the tip
                    special_printf(i == 0, i == 1, RC); // Restore the cursor position
                    special_printf(i == 0, i == 1, CV); // Make the cursor visible again
                }
            }
        }
        V(1); // Release semaphore 1
        V(0); // Release semaphore 0
    }
}

// Main application loop to handle user input and message processing
void mainapp()
{
    bool wl[2] = {false, false}; // Tracks if a warning for maximum length was displayed
    bool we[2] = {false, false};  // Tracks if a warning for empty messages was displayed
    int num2; // Random variable for potential delays or additional functionality

    // Infinite loop to process user input and messages
    while (1)
    {
        P(0); // Lock semaphore 0 for safe resource access
        P(1); // Lock semaphore 1 for additional synchronization

        num2 = rand() %100 ; 

        // Process input for each channel (0 and 1)
        for (i = 0; i <= 1; i++)
        {
            if (inapp[i]) // If the user is in command input mode
            {
                kp[i] = inkey(i); // Get the kp for the channel

                if (kp[i] != -1) // If a key is pressed
                {
                    // Add the kp to the key buffer
                    lkb[i][lkbl[i]] = kp[i];
                    lkbl[i]++;
                    lkb[i][lkbl[i]] = '\0';
                }
                else
                {
                    if (lkbl[i] > 0) // If there is data in the key buffer
                    {
                        if (strcmp(lkb[i], ESC) == 0) // If Escape key is pressed
                        {
                            special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", cp); // Reset the command prompt
                            inapp[i] = false; // Exit command input mode
                        }
                        else if (strcmp(lkb[i], BS) == 0) // If Backspace is pressed
                        {
                            if (mbl[i] > 0) // Ensure the message buffer is not empty
                            {
                                write(i + 3, &tbs, 3); // Handle backspace visually
                                mbl[i]--; // Remove the last character from the buffer
                                mb[i][mbl[i]] = '\0';
                            }
                        }
                        else if (strcmp(lkb[i], ENT) == 0) // If Enter is pressed
                        {
                            if (mbl[i] > 0) // If the message buffer contains data
                            {
                                special_printf(true, true, DRM, 47); // Reset the screen mode
                                if (we[i]) // If an empty message warning was displayed
                                {
                                    special_printf(i == 0, i == 1, CI);
                                    special_printf(i == 0, i == 1, SC);
                                    special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tl[tn[i]]); // Reset the tip
                                    special_printf(i == 0, i == 1, RC);
                                    special_printf(i == 0, i == 1, CV);
                                    we[i] = false; // Clear the warning flag
                                }
                                // Display the message in the chat area
                                special_printf(true, true, CI);
                                special_printf(true, true, CM, 1, 1);
                                for (j = 0; j < cr; j++)
                                    pb[j] = '\n';
                                pb[j + 1] = '\0';
                                write(3, &pb, cr); // Write the new message to channel 3
                                write(4, &pb, cr); // Write the new message to channel 4
                                if (cr < sh - 5) // Increment chat row if within limits
                                    cr++;
                                special_printf(true, true, "\x1b[0m\n│\x1b[1;38;5;%dm%s: \x1b[0m", unc[i], un[i]); // Display the sender's nickname
                                write(3, &mb[i], mbl[i]); // Write the message content
                                write(4, &mb[i], mbl[i]);
                                special_printf(true, true, "\x1b[%d;999H│", cr + 1); // Update the chat window
                                mbl[i] = 0; // Clear the message buffer
                                mb[i][0] = '\0';

                                // Reset the input area
                                special_printf(true, false, CM, sh - 2, tic[0] + mbl[0]);
                                special_printf(false, true, CM, sh - 2, tic[1] + mbl[1]);
                                special_printf(i == 0, i == 1, DLAC);
                                special_printf(true, true, "\x1b[%d;999H│", sh - 2);
                                if (inapp[0])
                                {
                                    special_printf(true, false, CM, sh - 2, tic[0] + mbl[0]);
                                }
                                else
                                {
                                    special_printf(true, false, CM, sh, strlen(cp) + cbl[0] + 1);
                                }
                                if (inapp[1])
                                {
                                    special_printf(false, true, CM, sh - 2, tic[1] + mbl[1]);
                                }
                                else
                                {
                                    special_printf(false, true, CM, sh, strlen(cp) + cbl[1] + 1);
                                }
                                special_printf(true, true, CV); // Make the cursor visible
                            }
                            else // If the message buffer is empty
                            {
                                // Display a warning about empty messages
                                special_printf(i == 0, i == 1, CI);
                                special_printf(i == 0, i == 1, SC);
                                special_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mSending Empty message is not allowed\x1b[K\x1b[0m");
                                special_printf(i == 0, i == 1, RC);
                                special_printf(i == 0, i == 1, CV);
                                we[i] = true; // Set the warning flag
                            }
                            // Restore the alternate screen mode
                            special_printf(inalt[0], inalt[1], DSM, 47);
                            special_printf(inalt[0], inalt[1], CM, 1, 1);
                            special_printf(inalt[0], inalt[1], ht);
                        }
                        else if (lkbl[i] == 1 && lkb[i][0] >= 32 && lkb[i][0] <= 126) // Printable characters
                        {
                            if (mbl[i] < 100) // Ensure the buffer does not exceed the limit
                            {
                                mb[i][mbl[i]] = lkb[i][0]; // Add character to the buffer
                                mbl[i]++;
                                mb[i][mbl[i]] = '\0';
                                write(i + 3, &lkb[i], lkbl[i]); // Write the kp to the output
                                if (wl[i]) // If a length warning was displayed
                                {
                                    special_printf(i == 0, i == 1, CI);
                                    special_printf(i == 0, i == 1, SC);
                                    special_printf(i == 0, i == 1, "\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tl[tn[i]]); // Reset the tip
                                    special_printf(i == 0, i == 1, RC);
                                    special_printf(i == 0, i == 1, CV);
                                    wl[i] = false; // Clear the warning flag
                                }
                            }
                            else // If the buffer is full
                            {
                                // Display a warning about maximum length
                                special_printf(i == 0, i == 1, CI);
                                special_printf(i == 0, i == 1, SC);
                                special_printf(i == 0, i == 1, "\x1b[999;H\x1b[44;97m\x1b[5mMaximum character limit per message reached (%d characters)\x1b[K\x1b[0m", BL);
                                special_printf(i == 0, i == 1, RC);
                                special_printf(i == 0, i == 1, CV);
                                wl[i] = true; // Set the warning flag
                            }
                        }
                        else if (!(lkbl[i] == 3 && lkb[i][0] == 27 && lkb[i][1] == 91 && lkb[i][2] >= 65 && lkb[i][2] <= 68))
                        {
                            // Ignore arrow keys
                            write(i + 3, &lkb[i], lkbl[i]);
                        }
                        lkb[i][0] = '\0'; // Clear the key buffer
                        lkbl[i] = 0;
                    }
                }
                kp[i] = -1; // Reset the kp
            }
        }
        V(1); // Release semaphore 1
        V(0); // Release semaphore 0
    }
}

// Initialize the user interface and layout of the terminal
void UI()
{
    // Set initial positions for input columns based on the message input prompt length
    for (i = 0; i <= 1; i++)
    {
        tic[i] = strlen(mip) + 3;
    }

    // Configure terminal screen settings
    special_printf(true, true, SS, sh, sw); // Set screen size
    special_printf(true, true, DRM, 47); // Reset scroll mode
    special_printf(true, true, CD); // Clear the terminal display
    special_printf(true, true, RS); // Reset text formatting
    special_printf(true, true, RSR); // Reset scroll rows
    special_printf(true, true, DSM, 19); // Enable a specific terminal mode

    // Display runtime and initial tip
    special_printf(true, true, "\x1b[47;30mRuntime 00:00:00\x1b[K\x1b[0m\x1b[999;H\x1b[47;30m%s\x1b[K\x1b[0m", tl[0]);

    // Configure scrolling region
    special_printf(true, true, CSR, 2, sh - 4);
    special_printf(true, true, DRM, 19); // Reset terminal mode
    special_printf(true, true, CI); // Make the cursor invisible

    // Draw the top border of the UI
    special_printf(true, true, CM, 2, 1);
    special_printf(true, true, "┌"); // Top left corner
    for (i = 1; i <= sw - 2; i++)
        special_printf(true, true, "─"); // Top horizontal border
    special_printf(true, true, "┐"); // Top right corner

    // Draw the vertical borders
    for (i = 3; i <= sh - 4; i++)
    {
        special_printf(true, true, CM, i, 1);
        special_printf(true, true, "│"); // Left vertical border
        special_printf(true, true, CM, i, sw);
        special_printf(true, true, "│"); // Right vertical border
    }

    // Draw the divider above the message input box
    special_printf(true, true, CM, sh - 3, 1);
    special_printf(true, true, "├"); // Left corner of the divider
    for (i = 1; i <= sw - 2; i++)
    {
        if (i == tic[0] - 2)
            special_printf(true, true, "┬"); // Interstion for input column
        else
            special_printf(true, true, "─"); // Horizontal divider
    }
    special_printf(true, true, "┤"); // Right corner of the divider

    // Draw the input box borders
    special_printf(true, true, CM, sh - 2, 1);
    special_printf(true, true, "│"); // Left border of the input box
    special_printf(true, true, CM, sh - 2, sw);
    special_printf(true, true, "│"); // Right border of the input box

    // Draw the bottom border of the UI
    special_printf(true, true, CM, sh - 1, 1);
    special_printf(true, true, "└"); // Bottom left corner
    for (i = 1; i <= sw - 2; i++)
    {
        if (i == tic[0] - 2)
            special_printf(true, true, "┴"); // Interstion for input column
        else
            special_printf(true, true, "─"); // Bottom horizontal border
    }
    special_printf(true, true, "┘"); // Bottom right corner

    // Position and display the message input prompt
    special_printf(true, true, CM, sh - 2, 2);
    special_printf(true, true, "\x1b[1m%s\x1b[0m", mip);
    special_printf(true, true, "│"); // Vertical middle border of the input box

    // Make the cursor visible again
    special_printf(true, true, CV);
}

void main()
{
    init_kernel();
    init_io();
    UI();
    set_task(clock);
    set_task(bar);
    set_task(mainapp);
    begin_sch();
}
