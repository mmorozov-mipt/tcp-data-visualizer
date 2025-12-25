#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <math.h>
#include <gtk/gtk.h>
#include <cairo.h>

#pragma comment(lib, "ws2_32.lib")

#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 65432
#define BUFFER_SIZE 1024

typedef struct {
    int *data;
    int size;
    double *moving_avg;
    int moving_avg_size;
} GraphData;

double calculate_mean(int *data, int size) {
    double sum = 0.0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
    }
    return sum / size;
}

int compare(const void *a, const void *b) {
    return (*(int *)a - *(int *)b);
}

double calculate_median(int *data, int size) {
    qsort(data, size, sizeof(int), compare);
    if (size % 2 == 0) {
        return (data[size / 2 - 1] + data[size / 2]) / 2.0;
    } else {
        return data[size / 2];
    }
}

double calculate_stddev(int *data, int size, double mean) {
    double variance = 0.0;
    for (int i = 0; i < size; i++) {
        variance += (data[i] - mean) * (data[i] - mean);
    }
    return sqrt(variance / size);
}

void calculate_moving_average(int *data, int size, int window_size, double *moving_avg) {
    for (int i = 0; i <= size - window_size; i++) {
        double sum = 0.0;
        for (int j = 0; j < window_size; j++) {
            sum += data[i + j];
        }
        moving_avg[i] = sum / window_size;
    }
}

static void draw_graph(GtkWidget *widget, cairo_t *cr, gpointer user_data) {
    GraphData *graph_data = (GraphData *)user_data;

    int width = gtk_widget_get_allocated_width(widget);
    int height = gtk_widget_get_allocated_height(widget);


    cairo_set_source_rgb(cr, 1, 1, 1); 
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

 
    int margin = 50;  
    int graph_width = width - 2 * margin;
    int graph_height = height - 2 * margin;


    double scale_x = (double)graph_width / graph_data->size;
    double scale_y = (double)graph_height / 100.0;

    cairo_translate(cr, margin, height - margin);  
    cairo_scale(cr, scale_x, -scale_y);  

  
    cairo_set_source_rgb(cr, 0, 0, 1); 
    cairo_set_line_width(cr, 1);
    cairo_move_to(cr, 0, graph_data->data[0]);
    for (int i = 1; i < graph_data->size; i++) {
        cairo_line_to(cr, i, graph_data->data[i]);
    }
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 1, 0, 0);  
    cairo_set_line_width(cr, 1.5);
    cairo_move_to(cr, 0, graph_data->moving_avg[0]);
    for (int i = 1; i < graph_data->moving_avg_size; i++) {
        cairo_line_to(cr, i, graph_data->moving_avg[i]);
    }
    cairo_stroke(cr);

    cairo_set_source_rgb(cr, 0, 0, 0);  
    cairo_set_line_width(cr, 0.5);
    cairo_move_to(cr, 0, 0);  
    cairo_line_to(cr, graph_data->size, 0);  
    cairo_move_to(cr, 0, 0);  
    cairo_line_to(cr, 0, 100);  
    cairo_stroke(cr);

	cairo_save(cr);
	cairo_scale(cr, -1, 0.6);
	cairo_rotate(cr, M_PI);
    cairo_move_to(cr, 500, 10);
    cairo_set_font_size(cr, 12);
    cairo_show_text(cr, "Time");
	cairo_restore(cr);

	cairo_save(cr);
	cairo_scale(cr, -1, 0.6);
	cairo_rotate(cr, M_PI);
    cairo_move_to(cr, -30, -150);
    cairo_show_text(cr, "Value");
    cairo_restore(cr);
	cairo_rotate(cr, M_PI);

	cairo_save(cr);
	cairo_scale(cr, -1, 0.6);
    cairo_move_to(cr, 400, -160);
    cairo_set_font_size(cr, 16);
    cairo_show_text(cr, "Data with Moving Average");
	cairo_rotate(cr, M_PI);
	cairo_restore(cr);

	cairo_save(cr);
	cairo_scale(cr, -1, 0.6);
    cairo_move_to(cr, 850, -15);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 0, 0, 1);
    cairo_show_text(cr, "Data");
	cairo_line_to(cr, 900, -15);
	cairo_stroke(cr);
	cairo_rotate(cr, M_PI);
	cairo_restore(cr);

	cairo_save(cr);
	cairo_scale(cr, -1, 0.6);
    cairo_move_to(cr, 850, -5);
    cairo_set_font_size(cr, 12);
    cairo_set_source_rgb(cr, 1, 0, 0);
    cairo_show_text(cr, "Moving Average");
	cairo_line_to(cr, 970, -5);
	cairo_stroke(cr);
	cairo_rotate(cr, M_PI);
	cairo_restore(cr);
}





int main(int argc, char *argv[]) {
    WSADATA wsaData;
    SOCKET sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    int data[1000];
    int count = 0;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        fprintf(stderr, "WSAStartup failed.\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        fprintf(stderr, "Socket creation failed.\n");
        WSACleanup();
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Connection failed.\n");
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    printf("Connected to the server.\n");

    while (count < 1000) {
        int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_received <= 0) {
            fprintf(stderr, "Receive failed.\n");
            break;
        }
        buffer[bytes_received] = '\0';

        char *token = strtok(buffer, "\n");
        while (token != NULL && count < 1000) {
            data[count++] = atoi(token);
            token = strtok(NULL, "\n");
        }
    }

    closesocket(sock);
    WSACleanup();
    printf("Data collection completed. Total numbers received: %d\n", count);

    double mean = calculate_mean(data, count);
    double median = calculate_median(data, count);
    double stddev = calculate_stddev(data, count, mean);

    int window_size = 10;
    int moving_avg_size = count - window_size + 1;
    double *moving_avg = malloc(moving_avg_size * sizeof(double));
    if (!moving_avg) {
        perror("Memory allocation failed");
        return 1;
    }
    calculate_moving_average(data, count, window_size, moving_avg);

    printf("Mean: %.2f\n", mean);
    printf("Median: %.2f\n", median);
    printf("Standard Deviation: %.2f\n", stddev);

    gtk_init(&argc, &argv);

    GraphData graph_data = {data, count, moving_avg, moving_avg_size};

    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Graph Visualization");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 600);

    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_container_add(GTK_CONTAINER(window), drawing_area);
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_graph), &graph_data);

    gtk_widget_show_all(window);
    gtk_main();

    free(moving_avg);
    return 0;
}
