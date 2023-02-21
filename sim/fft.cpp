// FFT demo

#include <iostream>
#include <fstream>
#include "math.h"

using std::swap;
using std::max;
using std::fstream;
using std::ios;
using std::cout;
using std::endl;

typedef long long int ll;

const int input_len = 8;
const int kernel_len = 3;
const int len = 128;
const long double PI = acos(-1.0);
const int quantized_bits = 21;

struct Complex {
    ll r;
    ll i;
    Complex(double r_ = 0, double i_ = 0): r(r_), i(i_) {}
    Complex operator+(const Complex& b) {
        Complex ans(0, 0);
        ans.r = r + b.r;
        ans.i = i + b.i;
        return ans;
    }
    Complex operator-(const Complex& b) {
        Complex ans(0, 0);
        ans.r = r - b.r;
        ans.i = i - b.i;
        return ans;
    }
    Complex operator*(const Complex& b) {
        Complex ans(0, 0);
        ans.r = (r * b.r - i * b.i) / pow(2, quantized_bits);
        ans.i = (r * b.i + i * b.r) / pow(2, quantized_bits);
        return ans;
    }
};

// Rader algorithm
void rader(Complex f[], int len) {
    int j = len >> 1;
    for(int i = 1; i < len-1; i++) {
        if(i < j)
            swap(f[i], f[j]);
        int k = len >> 1;
        while(j >= k) {
            j -= k;
            k >>= 1;
        }
        if(j < k)
            j += k;
    }
}

int cos(int h, int on) {
    return cos(-on * 2 * PI / h) * pow(2, quantized_bits);
}

int sin(int h, int on) {
    return sin(-on * 2 * PI / h) * pow(2, quantized_bits);
}

void fft(Complex f[], int len, int on) {
    rader(f, len);
    
    int h = 2;
    while(h <= len) {// h-length DFT
        // trig
        Complex wn(cos(h, on), sin(h, on)); // Euler's formula
        int j = 0;
        while(j < len) {
            // rotate    
            Complex w(1 * pow(2, quantized_bits), 0 * pow(2, quantized_bits)); // 旋转因子
            int k = j;
            while(k < j + h/2) {
                // merge
                Complex u = f[k];
                Complex t = w * f[k + h/2];
                f[k] = u + t;
                f[k + h/2] = u - t;
                w = w * wn;
                k++;
            }
            // j_add
            j += h;
        }
        // h_add;
        h <<= 1;
    }

    if(on == -1) {
        int i = 0;
        while(i < len) {
            f[i].r /= len;
            i++;
        }
    }
}

void convolution(Complex a[], Complex b[], int len) {
    for(int i = 0; i < len; i++) { // quantize input for quantized trignomical function
        a[i].r = a[i].r << quantized_bits;
        a[i].i = a[i].i << quantized_bits;
        b[i].r = b[i].r << quantized_bits;
        b[i].i = b[i].i << quantized_bits;
    }

    fft(a, len, 1); // a(time domain) fft a(frequemcy domain)
    fft(b, len, 1); // b(time domain) fft b(frequemcy domain)
    // convolution
    for(int i = 0; i < len; i++) {
        a[i] = a[i] * b[i];
    }

    fft(a, len, -1);
    for(int i = 0; i < len; i++) { // dequantize 
        a[i].r = a[i].r >> quantized_bits;
        a[i].i = a[i].i >> quantized_bits;
        b[i].r = b[i].r >> quantized_bits;
        b[i].i = b[i].i >> quantized_bits;
    }
}

void read(int input[input_len][input_len], int kernel[kernel_len][kernel_len]) {
    fstream file;
    file.open("input.dat", ios::in);
    for(int i = 0; i < input_len; i++) {
        for(int j = 0; j < input_len; j ++) {
            file >> input[i][j];
        }
    }
    file.close();

    file.open("weight.dat", ios::in);
    for(int i = 0; i < kernel_len; i++) {
        for(int j = 0; j < kernel_len; j++) {
            file >> kernel[i][j];
        }
    }
    file.close();
}

void write(int result[input_len - kernel_len + 1][input_len - kernel_len + 1]) {
    fstream file;
    file.open("golden.dat", ios::out);
    for(int i = 0; i < input_len - kernel_len + 1; i++) {
        for(int j = 0; j < input_len - kernel_len + 1; j++) {
            file << result[i][j] << endl;
        }
    }
    file.close();
}

int main() {
    int input[input_len][input_len];
    int kernel[kernel_len][kernel_len];
    int golden[input_len - kernel_len + 1][input_len - kernel_len + 1];
    int result[input_len - kernel_len + 1][input_len - kernel_len + 1];

    read(input, kernel);

    // caculate golden
    for(int i = 0; i < input_len - kernel_len + 1; i++) {
        for(int j = 0; j < input_len - kernel_len + 1; j++) {
            golden[i][j] = 0;
            for(int k = 0; k < kernel_len; k++) {
                for(int l = 0; l < kernel_len; l++) {
                    golden[i][j] += kernel[k][l] * input[i+k][j+l];
                }
            }
        }
    }

    // 2D 2 1D
    Complex a[len];
    Complex b[len];
    for(int i = 0; i < len; i++) {
        if(i < input_len * input_len)
            a[i] = input[i / input_len][i % input_len];
        else
            a[i] = Complex(0, 0);
    }

    for(int i = 0; i < len; i++) {
        b[i] = Complex(0, 0);
    }
    for(int i = 0; i < kernel_len; i++) {
        for(int j = 0; j < kernel_len; j++) {
            b[i * input_len + j] = kernel[kernel_len - 1 - i][kernel_len - 1 - j];
        }
    }

    convolution(a, b, len);

    // vertifaction
    bool pass = true;
    int base = input_len * kernel_len - input_len + kernel_len - 1;
    double max_error_rate = 0;
    double avg_error_rate = 0;
    int error_val = 2;
    for(int i = 0; i < input_len - kernel_len + 1; i++) {
        for(int j = 0; j < input_len - kernel_len + 1; j++) {
            double error = abs(abs(round(a[base + input_len * i + j].r)) - abs(golden[i][j]));
            double error_rate = error / abs(golden[i][j]);
            avg_error_rate += error_rate;
            max_error_rate = max(max_error_rate, error_rate);
            result[i][j] = (int)round(a[base + input_len * i + j].r);

            if(abs(golden[i][j] - round(a[base + input_len * i + j].r)) > error_val) {
                cout << "[" << i << "," << j << "]" << endl;
                cout << "   golden: " << golden[i][j] << endl;
                cout << "   fft   : " << round(a[base + input_len * i + j].r) << endl;
                cout << "   ===> failed" << endl;
            }
        }
    }
    if(pass)
        cout << "pass!!!" << endl;
    else
        cout << "fail!!!" << endl;
    write(result);

    cout << "error rate     = " << avg_error_rate / (input_len - kernel_len + 1) / (input_len - kernel_len + 1) * 100 << " %" << endl;
    cout << "max error rate = " << max_error_rate * 100 << " %" << endl;
    
    return 0;
}
