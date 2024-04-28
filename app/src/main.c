#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

int main() {
    const char *device = "/dev/lcd";
    int lcd_fd;

    // Check if the device exists
    if (access(device, F_OK) == -1) {
        printf("LCD device not found.\n");
        return 1;
    }

    // Open the device
    lcd_fd = open(device, O_WRONLY);
    if (lcd_fd == -1) {
        perror("Error opening LCD device");
        return 1;
    }

    // Write message to the device
    if (write(lcd_fd, "\f\e[L+Application Started", 19) != 19) {
        perror("Error writing to LCD device");
        close(lcd_fd);
        return 1;
    }

    // Close the device
    close(lcd_fd);
    return 0;
}
