#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

typedef struct {
    int input_dim;
    int hidden_dim;
    int style_dim;
    int output_dim;
    
    // RNN weights and biases
    float* W_h;    // shape: (hidden_dim, hidden_dim + input_dim + style_dim)
    float* b_h;    // shape: (hidden_dim)
    float* W_out;  // shape: (output_dim, hidden_dim)
    float* b_out;  // shape: (output_dim)
    
    // State variables
    float* h;           // hidden state: (hidden_dim)
    float* style_vector;// (style_dim)
    float* last_output; // (output_dim)
    
    // Pre-allocated temporary arrays
    float* combined_input; // (hidden_dim + input_dim + style_dim)
    float* temp;           // (hidden_dim)
} TextureRNN;

// Helper functions
float randn() {
    // Simple random normal approximation
    return ((float)rand() / RAND_MAX - 0.5f) * 2.0f;
}

float sigmoid(float x) {
    return 1.0f / (1.0f + expf(-x));
}

void matrix_vector_mult(const float* matrix, const float* vector, float* result, 
                       int rows, int cols) {
    for (int i = 0; i < rows; i++) {
        result[i] = 0;
        for (int j = 0; j < cols; j++) {
            result[i] += matrix[i * cols + j] * vector[j];
        }
    }
}

// Constructor-like initialization
TextureRNN* texture_rnn_init(int input_dim, int hidden_dim, int style_dim, int output_dim) {
    TextureRNN* rnn = (TextureRNN*)malloc(sizeof(TextureRNN));
    if (!rnn) return NULL;
    
    rnn->input_dim = input_dim;
    rnn->hidden_dim = hidden_dim;
    rnn->style_dim = style_dim;
    rnn->output_dim = output_dim;
    
    // Allocate memory
    int w_h_size = hidden_dim * (hidden_dim + input_dim + style_dim);
    rnn->W_h = (float*)malloc(w_h_size * sizeof(float));
    rnn->b_h = (float*)calloc(hidden_dim, sizeof(float));
    rnn->W_out = (float*)malloc(output_dim * hidden_dim * sizeof(float));
    rnn->b_out = (float*)calloc(output_dim, sizeof(float));
    rnn->h = (float*)calloc(hidden_dim, sizeof(float));
    rnn->style_vector = (float*)calloc(style_dim, sizeof(float));
    rnn->last_output = (float*)calloc(output_dim, sizeof(float));
    rnn->combined_input = (float*)calloc(hidden_dim + input_dim + style_dim, sizeof(float));
    rnn->temp = (float*)calloc(hidden_dim, sizeof(float));
    
    // Check for allocation failures
    if (!rnn->W_h || !rnn->b_h || !rnn->W_out || !rnn->b_out || 
        !rnn->h || !rnn->style_vector || !rnn->last_output || 
        !rnn->combined_input || !rnn->temp) {
        texture_rnn_free(rnn);
        return NULL;
    }
    
    // Initialize weights with random values * 0.1
    for (int i = 0; i < w_h_size; i++) {
        rnn->W_h[i] = randn() * 0.1f;
    }
    for (int i = 0; i < output_dim * hidden_dim; i++) {
        rnn->W_out[i] = randn() * 0.1f;
    }
    
    return rnn;
}

// Destructor-like cleanup
void texture_rnn_free(TextureRNN* rnn) {
    if (!rnn) return;
    free(rnn->W_h);
    free(rnn->b_h);
    free(rnn->W_out);
    free(rnn->b_out);
    free(rnn->h);
    free(rnn->style_vector);
    free(rnn->last_output);
    free(rnn->combined_input);
    free(rnn->temp);
    free(rnn);
}

void reset_state(TextureRNN* rnn, const float* style_vector) {
    if (!rnn) return;
    // Copy style vector
    for (int i = 0; i < rnn->style_dim; i++) {
        rnn->style_vector[i] = style_vector[i];
    }
    
    // Reset hidden state and last output
    for (int i = 0; i < rnn->hidden_dim; i++) {
        rnn->h[i] = 0.0f;
    }
    for (int i = 0; i < rnn->output_dim; i++) {
        rnn->last_output[i] = 0.0f;
    }
}

void rnn_step(TextureRNN* rnn, const float* inp, float* h_out, float* output_vec) {
    if (!rnn) return;
    
    // Combine inputs into pre-allocated array
    int idx = 0;
    for (int i = 0; i < rnn->hidden_dim; i++) rnn->combined_input[idx++] = rnn->h[i];
    for (int i = 0; i < rnn->input_dim; i++) rnn->combined_input[idx++] = inp[i];
    for (int i = 0; i < rnn->style_dim; i++) rnn->combined_input[idx++] = rnn->style_vector[i];
    
    // Update hidden state
    matrix_vector_mult(rnn->W_h, rnn->combined_input, rnn->temp, 
                      rnn->hidden_dim, rnn->hidden_dim + rnn->input_dim + rnn->style_dim);
    
    for (int i = 0; i < rnn->hidden_dim; i++) {
        h_out[i] = tanhf(rnn->temp[i] + rnn->b_h[i]);
        rnn->h[i] = h_out[i];
    }
    
    // Compute output
    matrix_vector_mult(rnn->W_out, rnn->h, output_vec, 
                      rnn->output_dim, rnn->hidden_dim);
    
    for (int i = 0; i < rnn->output_dim; i++) {
        output_vec[i] = sigmoid(output_vec[i] + rnn->b_out[i]);
    }
}

float* generate_next_pixel(TextureRNN* rnn) {
    if (!rnn) return NULL;
    
    float* h_temp = (float*)malloc(rnn->hidden_dim * sizeof(float));
    float* output_vec = (float*)malloc(rnn->output_dim * sizeof(float));
    if (!h_temp || !output_vec) {
        free(h_temp);
        free(output_vec);
        return NULL;
    }
    
    rnn_step(rnn, rnn->last_output, h_temp, output_vec);
    
    // Update last_output
    for (int i = 0; i < rnn->output_dim; i++) {
        rnn->last_output[i] = output_vec[i];
    }
    
    free(h_temp);
    return output_vec;  // Caller must free this
}

void stop_and_swap(TextureRNN* rnn, const float* new_style_vector, int keep_state) {
    if (!rnn) return;
    
    if (!keep_state) {
        for (int i = 0; i < rnn->hidden_dim; i++) {
            rnn->h[i] = 0.0f;
        }
    }
    
    // Update style vector
    for (int i = 0; i < rnn->style_dim; i++) {
        rnn->style_vector[i] = new_style_vector[i];
    }
}

// New function to generate and save a 2D texture image
void generate_texture_image(TextureRNN* rnn, int width, int height, const char* filename) {
    if (!rnn || width <= 0 || height <= 0 || !filename) return;
    
    // Open file for writing in binary mode
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("Error: Could not open file %s for writing\n", filename);
        return;
    }
    
    // Write PPM header (P6 format for binary RGB)
    fprintf(fp, "P6\n%d %d\n255\n", width, height);
    
    // Generate texture row by row
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            float* pixel = generate_next_pixel(rnn);
            if (!pixel) {
                printf("Error: Pixel generation failed at (%d, %d)\n", x, y);
                fclose(fp);
                return;
            }
            
            // Convert float (0-1) to unsigned char (0-255) for RGB
            unsigned char rgb[3];
            for (int i = 0; i < rnn->output_dim && i < 3; i++) {
                rgb[i] = (unsigned char)(pixel[i] * 255.0f);
            }
            free(pixel);
            
            // Write pixel to file
            fwrite(rgb, sizeof(unsigned char), 3, fp);
        }
    }
    
    fclose(fp);
    printf("Texture image saved as %s\n", filename);
}

// Example usage
int main() {
    srand(time(NULL)); // Seed RNG for varied initializations
    
    TextureRNN* rnn = texture_rnn_init(1, 64, 16, 3); // input_dim=1, hidden=64, style=16, RGB output
    if (!rnn) {
        printf("Error: Failed to initialize TextureRNN\n");
        return 1;
    }
    
    float style_vec[16] = {0}; // Example style vector
    reset_state(rnn, style_vec);
    
    // Generate a 64x64 texture image
    generate_texture_image(rnn, 64, 64, "texture.ppm");
    
    texture_rnn_free(rnn);
    return 0;
}
