#pragma once
#include <chrono>

class FPSMeter {
public:
    explicit FPSMeter(std::chrono::duration<double> interval = std::chrono::seconds(1));

    double get() const;            // Vrac� posledn� FPS
    bool is_updated() const;       // Je hodnota aktualizovan� od posledn�ho update?

    void update();                 // Zavolat na konci ka�d�ho frame
    void reset();                  // Reset FPS po��tadla
    void set_interval(std::chrono::duration<double> interval); // Nastavit interval

private:
    double m_fps{ 0.0 };
    std::chrono::time_point<std::chrono::steady_clock> m_last_time;
    std::chrono::duration<double> m_interval;
    size_t m_frame_count{ 0 };
    bool m_updated{ false };
};
