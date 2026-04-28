/*
    ALL UTILITY FUNCTIONS WILL BE DEFINED HERE.
*/

#include <math.h>

double lerp(double x, double target, double weight) {
    return x + (target - x) * weight;
}

double deg2rad(double x) {
    return x * (M_PI / 180);
}