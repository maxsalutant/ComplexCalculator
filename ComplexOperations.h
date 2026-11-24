#pragma once
#include <cmath>

class Complex {
private:
    double real, imagin;

public:
    // Конструкторы
    Complex(double r, double i) : real(r), imagin(i) {}
    Complex() : real(0.0), imagin(0.0) {}

    // Геттеры для вывода в ImGui
    double getReal() const { return real; }
    double getImag() const { return imagin; }

    // Сложение
    Complex operator+(const Complex& a) const {
        return Complex(real + a.real, imagin + a.imagin);
    }

    // Вычитание
    Complex operator-(const Complex& a) const {
        return Complex(real - a.real, imagin - a.imagin);
    }

    // Умножение
    Complex operator*(const Complex& a) const {
        return Complex(
            (real * a.real) - (imagin * a.imagin),
            (real * a.imagin) + (imagin * a.real)
        );
    }

    // Деление
    Complex operator/(const Complex& a) const {
        double denom = (a.real * a.real) + (a.imagin * a.imagin);
        return Complex(
            ((real * a.real) + (imagin * a.imagin)) / denom,
            ((imagin * a.real) - (real * a.imagin)) / denom
        );
    }

    // Модуль комплексного числа
    double abs() const {
        return std::hypot(real, imagin);
    }
};