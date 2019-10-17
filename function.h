#ifndef FUNCTION_H
#define FUNCTION_H

#include <chrono>
#include <forward_list>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <unordered_map>
#include <vector>

template<typename T>
void measurePerformance(T& t, const std::string& cont) {
    // measure performance template
    std::cout << std::fixed << std::setprecision(10);

    auto start = std::chrono::system_clock::now();
    T t1(t);
    auto end = std::chrono::system_clock::now() - start;
    std::cout << cont << std::endl;
    auto copyTime = std::chrono::duration<double>(end).count();
    std::cout <<  " Copy: " << copyTime << " sec" << std::endl;

    start = std::chrono::system_clock::now();
    T t2(std::move(t));
    end = std::chrono::system_clock::now() - start;
    auto moveTime = std::chrono::duration<double>(end).count();
    std::cout <<  " Move: " << moveTime << " sec" << std::endl;
}

// The function can also be written in such a way that it will accept any time duration unit.
template<class rep, class period>
void blink_led(std::chrono::duration<rep, period> time_to_blink) {
    // assuming that millisecond is the smallest relevant unit
    auto milliseconds_to_blink = std::chrono::duration_cast<std::chrono::milliseconds>(time_to_blink).count();
    std::cout << milliseconds_to_blink << " milliseconds" << std::endl;
}

#endif // FUNCTION_H
